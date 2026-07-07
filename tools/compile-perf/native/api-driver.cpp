// Compile-perf API driver: measures the Slang compilation API path (session
// creation, module loading, linking, code generation) that slangc-based
// workloads cannot see, because a CLI invocation pays those costs exactly once
// per process and reports none of them as separable timers.
//
// The tool loads a given libslang dynamically (dlopen / LoadLibrary) rather
// than linking it, so ONE host-built binary can measure EVERY release in a
// sweep ladder — the public COM ABI is append-only by contract, so a driver
// compiled against the current include/slang.h calls correctly into any older
// library in the tracked release window.
//
// Output format matches slangc -report-perf-benchmark so bench.py's
// parse_timers() consumes it unchanged:
//
//     [*] apiLoadModule   128     412.03ms
//
// Usage:
//     api-driver <libslang> session-create --iters N
//     api-driver <libslang> many-kernels  --dir DIR
//     api-driver <libslang> module-graph  --dir DIR --root MODULENAME
//
// Exit code 0 on success; on any Slang failure, diagnostics are printed to
// stdout (bench.py's real_error() recognizes "error" lines) and the exit code
// is 1.

#include "slang.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <dlfcn.h>
#endif

using Clock = std::chrono::steady_clock;

// Accumulates total milliseconds and an op count per phase, and prints the
// slangc-compatible "[*] name count total" report at the end of a run.
struct Timers
{
    struct Entry
    {
        std::string name;
        double totalMs = 0.0;
        long count = 0;
    };
    std::vector<Entry> entries;

    Entry& get(const char* name)
    {
        for (auto& e : entries)
            if (e.name == name)
                return e;
        entries.push_back({name, 0.0, 0});
        return entries.back();
    }

    void add(const char* name, double ms)
    {
        auto& e = get(name);
        e.totalMs += ms;
        e.count += 1;
    }

    void report() const
    {
        for (const auto& e : entries)
            printf("[*] %s\t%ld\t%.4fms\n", e.name.c_str(), e.count, e.totalMs);
    }
};

// Times one phase: construct to start, call stop() (or destruct) to record.
struct Scope
{
    Timers& timers;
    const char* name;
    Clock::time_point t0 = Clock::now();
    bool stopped = false;

    Scope(Timers& t, const char* n)
        : timers(t), name(n)
    {
    }
    void stop()
    {
        if (stopped)
            return;
        stopped = true;
        double ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
        timers.add(name, ms);
    }
    ~Scope() { stop(); }
};

typedef SlangResult (*CreateGlobalSessionFn)(SlangInt apiVersion, slang::IGlobalSession**);

static CreateGlobalSessionFn loadCreateGlobalSession(const char* libPath)
{
#ifdef _WIN32
    HMODULE lib = LoadLibraryA(libPath);
    if (!lib)
    {
        printf("error: failed to load library %s\n", libPath);
        return nullptr;
    }
    auto fn = (CreateGlobalSessionFn)GetProcAddress(lib, "slang_createGlobalSession");
#else
    void* lib = dlopen(libPath, RTLD_NOW | RTLD_LOCAL);
    if (!lib)
    {
        printf("error: failed to load library %s (%s)\n", libPath, dlerror());
        return nullptr;
    }
    auto fn = (CreateGlobalSessionFn)dlsym(lib, "slang_createGlobalSession");
#endif
    if (!fn)
        printf("error: slang_createGlobalSession not found in %s\n", libPath);
    return fn;
}

static void printDiagnostics(slang::IBlob* diagnostics)
{
    if (diagnostics && diagnostics->getBufferSize())
        printf(
            "%.*s\n",
            (int)diagnostics->getBufferSize(),
            (const char*)diagnostics->getBufferPointer());
}

// Reads a whole file; returns false (and reports) when unreadable.
static bool readFile(const std::string& path, std::string& out)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
    {
        printf("error: cannot open %s\n", path.c_str());
        return false;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    out.resize((size_t)size);
    size_t got = fread(&out[0], 1, (size_t)size, f);
    fclose(f);
    out.resize(got);
    return true;
}

// Lists <dir>/<prefix>*.slang, sorted, as full paths.
static std::vector<std::string> listSlangFiles(const std::string& dir, const char* prefix)
{
    std::vector<std::string> files;
#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    std::string pattern = dir + "\\" + prefix + "*.slang";
    HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            files.push_back(dir + "\\" + fd.cFileName);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
#else
    if (DIR* d = opendir(dir.c_str()))
    {
        while (dirent* ent = readdir(d))
        {
            std::string name = ent->d_name;
            if (name.rfind(prefix, 0) == 0 && name.size() > 6 &&
                name.compare(name.size() - 6, 6, ".slang") == 0)
                files.push_back(dir + "/" + name);
        }
        closedir(d);
    }
#endif
    std::sort(files.begin(), files.end());
    return files;
}

// Creates a session targeting SPIR-V with `dir` (when non-empty) as the import
// search path — the shape every workload here shares.
static SlangResult createSession(
    slang::IGlobalSession* globalSession,
    const std::string& dir,
    slang::ISession** outSession)
{
    slang::TargetDesc target = {};
    target.format = SLANG_SPIRV;
    target.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc desc = {};
    desc.targets = &target;
    desc.targetCount = 1;
    const char* searchPaths[] = {dir.c_str()};
    if (!dir.empty())
    {
        desc.searchPaths = searchPaths;
        desc.searchPathCount = 1;
    }
    return globalSession->createSession(desc, outSession);
}

// Compiles one loaded module's computeMain to SPIR-V through the standard
// entry-point path (find -> composite -> link -> getEntryPointCode), adding
// each phase to `timers`. Returns false (diagnostics already printed) on
// failure.
static bool compileEntryPoint(slang::ISession* session, slang::IModule* module, Timers& timers)
{
    slang::IEntryPoint* entryPoint = nullptr;
    {
        Scope s(timers, "apiFindEntryPoint");
        if (SLANG_FAILED(module->findEntryPointByName("computeMain", &entryPoint)))
        {
            printf("error: findEntryPointByName(computeMain) failed for %s\n", module->getName());
            return false;
        }
    }

    slang::IComponentType* components[] = {module, entryPoint};
    slang::IComponentType* composite = nullptr;
    slang::IBlob* diagnostics = nullptr;
    {
        Scope s(timers, "apiComposite");
        SlangResult res =
            session->createCompositeComponentType(components, 2, &composite, &diagnostics);
        printDiagnostics(diagnostics);
        if (diagnostics)
            diagnostics->release();
        if (SLANG_FAILED(res))
            return false;
    }

    slang::IComponentType* linked = nullptr;
    diagnostics = nullptr;
    {
        Scope s(timers, "apiLink");
        SlangResult res = composite->link(&linked, &diagnostics);
        printDiagnostics(diagnostics);
        if (diagnostics)
            diagnostics->release();
        if (SLANG_FAILED(res))
            return false;
    }

    slang::IBlob* code = nullptr;
    diagnostics = nullptr;
    {
        Scope s(timers, "apiGetCode");
        SlangResult res = linked->getEntryPointCode(0, 0, &code, &diagnostics);
        printDiagnostics(diagnostics);
        if (diagnostics)
            diagnostics->release();
        if (SLANG_FAILED(res) || !code || !code->getBufferSize())
        {
            printf("error: getEntryPointCode produced no code for %s\n", module->getName());
            return false;
        }
    }

    code->release();
    linked->release();
    composite->release();
    entryPoint->release();
    return true;
}

// session-create: N times create + destroy a global session and a session,
// timing both phases. Measures core-module deserialization and session setup
// — the fixed cost every API client (and every slangc invocation) pays first.
static int runSessionCreate(CreateGlobalSessionFn createGlobalSession, int iters)
{
    Timers timers;
    Scope total(timers, "apiTotal");
    for (int i = 0; i < iters; i++)
    {
        slang::IGlobalSession* globalSession = nullptr;
        {
            Scope s(timers, "apiCreateGlobalSession");
            if (SLANG_FAILED(createGlobalSession(SLANG_API_VERSION, &globalSession)))
            {
                printf("error: slang_createGlobalSession failed\n");
                return 1;
            }
        }
        slang::ISession* session = nullptr;
        {
            Scope s(timers, "apiCreateSession");
            if (SLANG_FAILED(createSession(globalSession, "", &session)))
            {
                printf("error: createSession failed\n");
                return 1;
            }
        }
        session->release();
        globalSession->release();
    }
    total.stop();
    timers.report();
    return 0;
}

// many-kernels: one session; every kernel_*.slang in DIR goes through
// loadModuleFromSourceString -> composite -> link -> getEntryPointCode
// individually, the way application middleware (e.g. slangpy) generates and
// compiles many small kernels. Per-compile fixed overhead dominates here,
// which is exactly the dimension where small-program regressions amplify.
static int runManyKernels(CreateGlobalSessionFn createGlobalSession, const std::string& dir)
{
    Timers timers;
    Scope total(timers, "apiTotal");

    slang::IGlobalSession* globalSession = nullptr;
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(createGlobalSession(SLANG_API_VERSION, &globalSession)))
        {
            printf("error: slang_createGlobalSession failed\n");
            return 1;
        }
    }
    slang::ISession* session = nullptr;
    {
        Scope s(timers, "apiCreateSession");
        if (SLANG_FAILED(createSession(globalSession, dir, &session)))
        {
            printf("error: createSession failed\n");
            return 1;
        }
    }

    auto files = listSlangFiles(dir, "kernel_");
    if (files.empty())
    {
        printf("error: no kernel_*.slang files in %s\n", dir.c_str());
        return 1;
    }
    for (const auto& path : files)
    {
        std::string source;
        if (!readFile(path, source))
            return 1;
        size_t base = path.find_last_of("/\\") + 1;
        std::string name = path.substr(base, path.size() - base - 6); // strip .slang

        slang::IModule* module = nullptr;
        slang::IBlob* diagnostics = nullptr;
        {
            Scope s(timers, "apiLoadModule");
            module = session->loadModuleFromSourceString(
                name.c_str(),
                path.c_str(),
                source.c_str(),
                &diagnostics);
            printDiagnostics(diagnostics);
            if (diagnostics)
                diagnostics->release();
            if (!module)
                return 1;
        }
        if (!compileEntryPoint(session, module, timers))
            return 1;
    }

    session->release();
    globalSession->release();
    total.stop();
    timers.report();
    return 0;
}

// module-graph: load one root module by name (its imports resolve through the
// session search path, pulling the whole DAG through parse/check/IR), then
// link and generate code once. Measures import resolution and linking over a
// realistically deep module graph rather than many-kernels' independent
// singletons.
static int runModuleGraph(
    CreateGlobalSessionFn createGlobalSession,
    const std::string& dir,
    const std::string& root)
{
    Timers timers;
    Scope total(timers, "apiTotal");

    slang::IGlobalSession* globalSession = nullptr;
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(createGlobalSession(SLANG_API_VERSION, &globalSession)))
        {
            printf("error: slang_createGlobalSession failed\n");
            return 1;
        }
    }
    slang::ISession* session = nullptr;
    {
        Scope s(timers, "apiCreateSession");
        if (SLANG_FAILED(createSession(globalSession, dir, &session)))
        {
            printf("error: createSession failed\n");
            return 1;
        }
    }

    slang::IModule* module = nullptr;
    slang::IBlob* diagnostics = nullptr;
    {
        Scope s(timers, "apiLoadModule");
        module = session->loadModule(root.c_str(), &diagnostics);
        printDiagnostics(diagnostics);
        if (diagnostics)
            diagnostics->release();
        if (!module)
            return 1;
    }
    if (!compileEntryPoint(session, module, timers))
        return 1;

    session->release();
    globalSession->release();
    total.stop();
    timers.report();
    return 0;
}

static const char* argValue(int argc, char** argv, const char* flag)
{
    for (int i = 3; i + 1 < argc; i++)
        if (strcmp(argv[i], flag) == 0)
            return argv[i + 1];
    return nullptr;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("usage: api-driver <libslang> session-create --iters N\n"
               "       api-driver <libslang> many-kernels  --dir DIR\n"
               "       api-driver <libslang> module-graph  --dir DIR --root MODULENAME\n");
        return 2;
    }
    auto createGlobalSession = loadCreateGlobalSession(argv[1]);
    if (!createGlobalSession)
        return 1;

    std::string mode = argv[2];
    if (mode == "session-create")
    {
        const char* iters = argValue(argc, argv, "--iters");
        return runSessionCreate(createGlobalSession, iters ? atoi(iters) : 10);
    }
    if (mode == "many-kernels")
    {
        const char* dir = argValue(argc, argv, "--dir");
        if (!dir)
        {
            printf("error: many-kernels requires --dir\n");
            return 2;
        }
        return runManyKernels(createGlobalSession, dir);
    }
    if (mode == "module-graph")
    {
        const char* dir = argValue(argc, argv, "--dir");
        const char* root = argValue(argc, argv, "--root");
        if (!dir || !root)
        {
            printf("error: module-graph requires --dir and --root\n");
            return 2;
        }
        return runModuleGraph(createGlobalSession, dir, root);
    }
    printf("error: unknown mode %s\n", mode.c_str());
    return 2;
}
