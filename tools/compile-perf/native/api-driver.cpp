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
//     api-driver <libslang> session-create    --iters N
//     api-driver <libslang> many-kernels      --dir DIR [--reflect]
//     api-driver <libslang> module-graph      --dir DIR --root MODULENAME [--reflect]
//     api-driver <libslang> module-graph-bin  --dir DIR --root MODULENAME
//     api-driver <libslang> specialize        --dir DIR --root MODULENAME
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

// The subset of libslang entry points the driver resolves dynamically. COM
// methods dispatch through vtables and need no symbols, but the reflection
// accessors in slang.h are inline wrappers over imported spReflection_* C
// functions — a dlopen-based tool must resolve those itself. decltype keeps
// each pointer's signature exactly what the header declares.
struct LibSlang
{
    CreateGlobalSessionFn createGlobalSession = nullptr;
    decltype(&spReflection_GetParameterCount) reflGetParameterCount = nullptr;
    decltype(&spReflection_GetParameterByIndex) reflGetParameterByIndex = nullptr;
    decltype(&spReflection_FindTypeByName) reflFindTypeByName = nullptr;
    decltype(&spReflectionVariableLayout_GetTypeLayout) reflVarLayoutGetTypeLayout = nullptr;
    decltype(&spReflectionTypeLayout_GetFieldCount) reflTypeLayoutGetFieldCount = nullptr;
    decltype(&spReflectionTypeLayout_GetFieldByIndex) reflTypeLayoutGetFieldByIndex = nullptr;
    decltype(&spReflectionTypeLayout_GetElementTypeLayout) reflTypeLayoutGetElementTypeLayout =
        nullptr;
    decltype(&spReflectionTypeLayout_GetSize) reflTypeLayoutGetSize = nullptr;
};

static void* loadSymbol(void* lib, const char* name)
{
#ifdef _WIN32
    return (void*)GetProcAddress((HMODULE)lib, name);
#else
    return dlsym(lib, name);
#endif
}

// Loads libslang and resolves the entry points above. Only
// slang_createGlobalSession is mandatory; reflection functions are checked at
// the point of use so older/unusual libraries still run the non-reflection
// workloads.
static bool loadLibSlang(const char* libPath, LibSlang& out)
{
#ifdef _WIN32
    void* lib = (void*)LoadLibraryA(libPath);
    if (!lib)
    {
        printf("error: failed to load library %s\n", libPath);
        return false;
    }
#else
    void* lib = dlopen(libPath, RTLD_NOW | RTLD_LOCAL);
    if (!lib)
    {
        printf("error: failed to load library %s (%s)\n", libPath, dlerror());
        return false;
    }
#endif
    out.createGlobalSession =
        (CreateGlobalSessionFn)loadSymbol(lib, "slang_createGlobalSession");
    if (!out.createGlobalSession)
    {
        printf("error: slang_createGlobalSession not found in %s\n", libPath);
        return false;
    }
    out.reflGetParameterCount =
        (decltype(&spReflection_GetParameterCount))loadSymbol(lib, "spReflection_GetParameterCount");
    out.reflGetParameterByIndex = (decltype(&spReflection_GetParameterByIndex))loadSymbol(
        lib, "spReflection_GetParameterByIndex");
    out.reflFindTypeByName =
        (decltype(&spReflection_FindTypeByName))loadSymbol(lib, "spReflection_FindTypeByName");
    out.reflVarLayoutGetTypeLayout =
        (decltype(&spReflectionVariableLayout_GetTypeLayout))loadSymbol(
            lib, "spReflectionVariableLayout_GetTypeLayout");
    out.reflTypeLayoutGetFieldCount = (decltype(&spReflectionTypeLayout_GetFieldCount))loadSymbol(
        lib, "spReflectionTypeLayout_GetFieldCount");
    out.reflTypeLayoutGetFieldByIndex =
        (decltype(&spReflectionTypeLayout_GetFieldByIndex))loadSymbol(
            lib, "spReflectionTypeLayout_GetFieldByIndex");
    out.reflTypeLayoutGetElementTypeLayout =
        (decltype(&spReflectionTypeLayout_GetElementTypeLayout))loadSymbol(
            lib, "spReflectionTypeLayout_GetElementTypeLayout");
    out.reflTypeLayoutGetSize =
        (decltype(&spReflectionTypeLayout_GetSize))loadSymbol(lib, "spReflectionTypeLayout_GetSize");
    return true;
}

static bool hasReflectionApi(const LibSlang& lib)
{
    return lib.reflGetParameterCount && lib.reflGetParameterByIndex &&
           lib.reflVarLayoutGetTypeLayout && lib.reflTypeLayoutGetFieldCount &&
           lib.reflTypeLayoutGetFieldByIndex && lib.reflTypeLayoutGetElementTypeLayout &&
           lib.reflTypeLayoutGetSize;
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

// Recursively sums field counts and uniform sizes of a type layout, the way an
// application walks reflection to build binding tables. The returned value is
// accumulated into a checksum so the traversal cannot be considered dead.
static long walkTypeLayout(const LibSlang& lib, SlangReflectionTypeLayout* tl, int depth)
{
    if (!tl || depth > 12)
        return 0;
    long acc = (long)lib.reflTypeLayoutGetSize(tl, SLANG_PARAMETER_CATEGORY_UNIFORM);
    unsigned fields = lib.reflTypeLayoutGetFieldCount(tl);
    acc += fields;
    for (unsigned i = 0; i < fields; i++)
    {
        auto field = lib.reflTypeLayoutGetFieldByIndex(tl, i); // a variable layout
        if (field)
            acc += walkTypeLayout(lib, lib.reflVarLayoutGetTypeLayout(field), depth + 1);
    }
    if (auto elem = lib.reflTypeLayoutGetElementTypeLayout(tl))
        if (elem != tl)
            acc += walkTypeLayout(lib, elem, depth + 1);
    return acc;
}

// Queries the linked program's layout and walks every global parameter's type
// layout — the per-program reflection cost every API client pays to build its
// binding tables. Adds the time to `timers` under apiReflection.
static bool reflectProgram(
    const LibSlang& lib,
    slang::IComponentType* linked,
    Timers& timers,
    long& checksum)
{
    if (!hasReflectionApi(lib))
    {
        printf("error: reflection entry points missing from this libslang\n");
        return false;
    }
    Scope s(timers, "apiReflection");
    slang::IBlob* diagnostics = nullptr;
    slang::ProgramLayout* layout = linked->getLayout(0, &diagnostics);
    printDiagnostics(diagnostics);
    if (diagnostics)
        diagnostics->release();
    if (!layout)
    {
        printf("error: getLayout returned no layout\n");
        return false;
    }
    auto refl = (SlangReflection*)layout;
    // One walk per program is only ~10 µs — below any alerting threshold.
    // Applications also query layouts repeatedly (per-dispatch parameter
    // lookups), so time 50 full walks per program: realistic usage AND a
    // timer large enough for the trend check's absolute-ms floor.
    for (int rep = 0; rep < 50; rep++)
    {
        unsigned params = lib.reflGetParameterCount(refl);
        for (unsigned i = 0; i < params; i++)
        {
            auto p = (SlangReflectionVariableLayout*)lib.reflGetParameterByIndex(refl, i);
            if (p)
                checksum += walkTypeLayout(lib, lib.reflVarLayoutGetTypeLayout(p), 0);
        }
    }
    return true;
}

// Compiles one loaded module's computeMain to SPIR-V through the standard
// entry-point path (find -> composite -> link -> getEntryPointCode), adding
// each phase to `timers`. When `lib` is non-null the linked program's layout
// is walked too (apiReflection). Returns false (diagnostics already printed)
// on failure.
static bool compileEntryPoint(
    slang::ISession* session,
    slang::IModule* module,
    Timers& timers,
    const LibSlang* reflectWith = nullptr)
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

    if (reflectWith)
    {
        static long s_checksum = 0;
        if (!reflectProgram(*reflectWith, linked, timers, s_checksum))
            return false;
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
static int runManyKernels(const LibSlang& lib, const std::string& dir, bool reflect)
{
    Timers timers;
    Scope total(timers, "apiTotal");

    slang::IGlobalSession* globalSession = nullptr;
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(lib.createGlobalSession(SLANG_API_VERSION, &globalSession)))
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
        if (!compileEntryPoint(session, module, timers, reflect ? &lib : nullptr))
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
    const LibSlang& lib,
    const std::string& dir,
    const std::string& root,
    bool reflect)
{
    Timers timers;
    Scope total(timers, "apiTotal");

    slang::IGlobalSession* globalSession = nullptr;
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(lib.createGlobalSession(SLANG_API_VERSION, &globalSession)))
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
    if (!compileEntryPoint(session, module, timers, reflect ? &lib : nullptr))
        return 1;

    session->release();
    globalSession->release();
    total.stop();
    timers.report();
    return 0;
}

// module-graph-bin: like module-graph, but the timed load resolves the DAG
// from serialized .slang-module binaries instead of source — the import path
// where the 2026-07-03 module-loading regression (#11952) lives, which
// source-based loads do not exercise. Setup (untimed apiTotal-wise it is
// timed under its own names): load the graph from source once and write every
// loaded module out with IModule::writeToFile; then a FRESH session re-loads
// the root, letting import resolution pick the binaries.
static int runModuleGraphBin(
    const LibSlang& lib,
    const std::string& dir,
    const std::string& root)
{
    Timers timers;

    slang::IGlobalSession* globalSession = nullptr;
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(lib.createGlobalSession(SLANG_API_VERSION, &globalSession)))
        {
            printf("error: slang_createGlobalSession failed\n");
            return 1;
        }
    }

    // Setup pass: source load + serialize every module in the graph.
    {
        slang::ISession* setupSession = nullptr;
        if (SLANG_FAILED(createSession(globalSession, dir, &setupSession)))
        {
            printf("error: createSession failed\n");
            return 1;
        }
        slang::IBlob* diagnostics = nullptr;
        slang::IModule* rootModule;
        {
            Scope s(timers, "apiLoadModuleSource");
            rootModule = setupSession->loadModule(root.c_str(), &diagnostics);
            printDiagnostics(diagnostics);
            if (diagnostics)
                diagnostics->release();
            if (!rootModule)
                return 1;
        }
        {
            Scope s(timers, "apiWriteModule");
            SlangInt count = setupSession->getLoadedModuleCount();
            for (SlangInt i = 0; i < count; i++)
            {
                slang::IModule* m = setupSession->getLoadedModule(i);
                std::string path = dir + "/" + m->getName() + ".slang-module";
                if (SLANG_FAILED(m->writeToFile(path.c_str())))
                {
                    printf("error: writeToFile failed for %s\n", path.c_str());
                    return 1;
                }
            }
        }
        setupSession->release();
    }

    // Timed pass: fresh session; imports must now resolve to the binaries.
    Scope total(timers, "apiTotal");
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
    // The workload exists to measure the binary-module path: fail loudly if
    // import resolution silently fell back to the .slang sources.
    {
        SlangInt count = session->getLoadedModuleCount();
        int binary = 0;
        for (SlangInt i = 0; i < count; i++)
        {
            const char* path = session->getLoadedModule(i)->getFilePath();
            std::string p = path ? path : "";
            if (p.size() > 13 && p.compare(p.size() - 13, 13, ".slang-module") == 0)
                binary++;
        }
        if (binary == 0)
        {
            printf("error: no module resolved to a .slang-module binary (%d loaded)\n",
                   (int)count);
            return 1;
        }
    }
    if (!compileEntryPoint(session, module, timers))
        return 1;

    session->release();
    globalSession->release();
    total.stop();
    timers.report();
    return 0;
}

// specialize: one module declares an interface, N conforming impl structs,
// and a generic computeMain<T : IOp>. Each impl is compiled as its own
// specialized variant (findTypeByName -> SpecializationArg ->
// IEntryPoint::specialize -> composite -> link -> getEntryPointCode) — the
// application pattern where one kernel is stamped out per material/config
// type, stressing specialization + link per variant.
static int runSpecialize(const LibSlang& lib, const std::string& dir, const std::string& root)
{
    if (!lib.reflFindTypeByName)
    {
        printf("error: spReflection_FindTypeByName missing from this libslang\n");
        return 1;
    }
    Timers timers;
    Scope total(timers, "apiTotal");

    slang::IGlobalSession* globalSession = nullptr;
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(lib.createGlobalSession(SLANG_API_VERSION, &globalSession)))
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

    slang::IEntryPoint* entryPoint = nullptr;
    if (SLANG_FAILED(module->findEntryPointByName("computeMain", &entryPoint)))
    {
        printf("error: findEntryPointByName(computeMain) failed\n");
        return 1;
    }
    slang::ProgramLayout* moduleLayout = module->getLayout(0, &diagnostics);
    printDiagnostics(diagnostics);
    if (diagnostics)
        diagnostics->release();
    if (!moduleLayout)
    {
        printf("error: module getLayout failed\n");
        return 1;
    }

    for (int i = 0;; i++)
    {
        char implName[32];
        snprintf(implName, sizeof(implName), "Impl_%d", i);
        auto type = (slang::TypeReflection*)lib.reflFindTypeByName(
            (SlangReflection*)moduleLayout, implName);
        if (!type)
        {
            if (i == 0)
            {
                printf("error: type Impl_0 not found in %s\n", root.c_str());
                return 1;
            }
            break; // ran past the last generated impl
        }

        slang::IComponentType* specialized = nullptr;
        diagnostics = nullptr;
        {
            Scope s(timers, "apiSpecialize");
            slang::SpecializationArg arg = slang::SpecializationArg::fromType(type);
            SlangResult res = entryPoint->specialize(&arg, 1, &specialized, &diagnostics);
            printDiagnostics(diagnostics);
            if (diagnostics)
                diagnostics->release();
            if (SLANG_FAILED(res))
                return 1;
        }

        slang::IComponentType* components[] = {module, specialized};
        slang::IComponentType* composite = nullptr;
        diagnostics = nullptr;
        {
            Scope s(timers, "apiComposite");
            SlangResult res =
                session->createCompositeComponentType(components, 2, &composite, &diagnostics);
            printDiagnostics(diagnostics);
            if (diagnostics)
                diagnostics->release();
            if (SLANG_FAILED(res))
                return 1;
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
                return 1;
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
                printf("error: getEntryPointCode produced no code for %s\n", implName);
                return 1;
            }
        }
        code->release();
        linked->release();
        composite->release();
        specialized->release();
    }

    entryPoint->release();
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

static bool hasFlag(int argc, char** argv, const char* flag)
{
    for (int i = 3; i < argc; i++)
        if (strcmp(argv[i], flag) == 0)
            return true;
    return false;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("usage: api-driver <libslang> session-create    --iters N\n"
               "       api-driver <libslang> many-kernels      --dir DIR [--reflect]\n"
               "       api-driver <libslang> module-graph      --dir DIR --root NAME [--reflect]\n"
               "       api-driver <libslang> module-graph-bin  --dir DIR --root NAME\n"
               "       api-driver <libslang> specialize        --dir DIR --root NAME\n");
        return 2;
    }
    LibSlang lib;
    if (!loadLibSlang(argv[1], lib))
        return 1;

    std::string mode = argv[2];
    bool reflect = hasFlag(argc, argv, "--reflect");
    if (mode == "session-create")
    {
        const char* iters = argValue(argc, argv, "--iters");
        return runSessionCreate(lib.createGlobalSession, iters ? atoi(iters) : 10);
    }
    const char* dir = argValue(argc, argv, "--dir");
    const char* root = argValue(argc, argv, "--root");
    if (mode == "many-kernels")
    {
        if (!dir)
        {
            printf("error: many-kernels requires --dir\n");
            return 2;
        }
        return runManyKernels(lib, dir, reflect);
    }
    if (!dir || !root)
    {
        printf("error: %s requires --dir and --root\n", mode.c_str());
        return 2;
    }
    if (mode == "module-graph")
        return runModuleGraph(lib, dir, root, reflect);
    if (mode == "module-graph-bin")
        return runModuleGraphBin(lib, dir, root);
    if (mode == "specialize")
        return runSpecialize(lib, dir, root);
    printf("error: unknown mode %s\n", mode.c_str());
    return 2;
}
