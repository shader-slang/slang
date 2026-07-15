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
//     api-driver <libslang> specialize        --dir DIR --root MODULENAME [--impl-prefix P]
//     api-driver <libslang> rt-composite      --dir DIR --root MODULENAME
//
// Exit code 0 on success; on any Slang failure, diagnostics are printed to
// stdout (bench.py's real_error() recognizes "error" lines) and the exit code
// is 1.
//
// Timer contract: leaf `Scope(timers, ...)` sites below wrap exactly ONE
// public API call (wall clock, driver side); the composites are apiTotal
// (a mode's whole timed section), apiReflection (getLayout + the reflection
// walk), and apiCreateSession (findProfile + createSession). The user-facing
// glossary — which call
// each timer maps to, the apiTotal setup-exclusion rule, and how to read
// apiGetCode against the library's own compileInner — lives in
// ../README.md ("API timer glossary"). These names appear on the perf site
// and in trend alerts: when adding or changing a timer, update that table in
// the same commit.

#include "slang.h"

// COM lifetimes are managed with Slang::ComPtr so early error-path returns
// cannot leak acquired objects. IModule pointers from ISession::loadModule*
// are session-owned BORROWED pointers (not add-ref'd for the caller) and stay
// raw by design.
#include "slang-com-ptr.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WIN32
#include <psapi.h>
#include <windows.h>
#else
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <mach/mach.h>
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

// Current resident set size of this process in KB (working set on Windows),
// or -1 if unavailable. Point-in-time, not peak: the caller differences two
// readings around a phase, so what matters is that both use the same measure.
static long currentRssKb()
{
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        return (long)(pmc.WorkingSetSize / 1024);
    return -1;
#elif defined(__APPLE__)
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) ==
        KERN_SUCCESS)
        return (long)(info.resident_size / 1024);
    return -1;
#else
    FILE* f = fopen("/proc/self/statm", "r");
    if (!f)
        return -1;
    long pages = 0, resident = 0;
    int n = fscanf(f, "%ld %ld", &pages, &resident);
    fclose(f);
    if (n != 2)
        return -1;
    return resident * (sysconf(_SC_PAGESIZE) / 1024);
#endif
}

// Memory deltas recorded around selected phases, reported next to the timers
// as "[MEM] name\tNNNkb" lines that bench.py parses into the record's
// `memory` dict. The RSS growth across createGlobalSession is the metric of
// shader-slang/slang#9817. Reuses Timers as the accumulator (totalMs holds
// KB); reportMemDeltas() runs wherever timers.report() does.
static Timers g_memDeltas;

static void recordSessionCreateRss(long rssBefore)
{
    // Only the FIRST createGlobalSession of the process is recorded: that is
    // the cold-session footprint of shader-slang/slang#9817. Workloads that
    // create many sessions (api_session_create) warm the allocator and core
    // module on the first call, so summing or averaging later deltas would
    // dilute the number the issue is about.
    if (g_memDeltas.get("apiCreateGlobalSessionRssDeltaKb").count > 0)
        return;
    long rssAfter = currentRssKb();
    if (rssBefore >= 0 && rssAfter >= 0)
        g_memDeltas.add("apiCreateGlobalSessionRssDeltaKb", (double)(rssAfter - rssBefore));
}

static void reportMemDeltas()
{
    for (const auto& e : g_memDeltas.entries)
        printf("[MEM] %s\t%.0fkb\n", e.name.c_str(), e.totalMs);
}

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
    out.createGlobalSession = (CreateGlobalSessionFn)loadSymbol(lib, "slang_createGlobalSession");
    if (!out.createGlobalSession)
    {
        printf("error: slang_createGlobalSession not found in %s\n", libPath);
        return false;
    }
    out.reflGetParameterCount = (decltype(&spReflection_GetParameterCount))loadSymbol(
        lib,
        "spReflection_GetParameterCount");
    out.reflGetParameterByIndex = (decltype(&spReflection_GetParameterByIndex))loadSymbol(
        lib,
        "spReflection_GetParameterByIndex");
    out.reflFindTypeByName =
        (decltype(&spReflection_FindTypeByName))loadSymbol(lib, "spReflection_FindTypeByName");
    out.reflVarLayoutGetTypeLayout = (decltype(&spReflectionVariableLayout_GetTypeLayout))
        loadSymbol(lib, "spReflectionVariableLayout_GetTypeLayout");
    out.reflTypeLayoutGetFieldCount = (decltype(&spReflectionTypeLayout_GetFieldCount))loadSymbol(
        lib,
        "spReflectionTypeLayout_GetFieldCount");
    out.reflTypeLayoutGetFieldByIndex = (decltype(&spReflectionTypeLayout_GetFieldByIndex))
        loadSymbol(lib, "spReflectionTypeLayout_GetFieldByIndex");
    out.reflTypeLayoutGetElementTypeLayout =
        (decltype(&spReflectionTypeLayout_GetElementTypeLayout))loadSymbol(
            lib,
            "spReflectionTypeLayout_GetElementTypeLayout");
    out.reflTypeLayoutGetSize = (decltype(&spReflectionTypeLayout_GetSize))loadSymbol(
        lib,
        "spReflectionTypeLayout_GetSize");
    return true;
}

static bool hasReflectionApi(const LibSlang& lib)
{
    return lib.reflGetParameterCount && lib.reflGetParameterByIndex &&
           lib.reflVarLayoutGetTypeLayout && lib.reflTypeLayoutGetFieldCount &&
           lib.reflTypeLayoutGetFieldByIndex && lib.reflTypeLayoutGetElementTypeLayout &&
           lib.reflTypeLayoutGetSize;
}

// IModule::getName() may return null (an unnamed module); printf("%s", null)
// and std::string concatenation of null are both UB, so every display use goes
// through this.
static const char* moduleDisplayName(slang::IModule* module)
{
    const char* name = module->getName();
    return (name && *name) ? name : "<unnamed>";
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
    if (size < 0)
    {
        printf("error: cannot determine size of %s\n", path.c_str());
        fclose(f);
        return false;
    }
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
// application walks reflection to build binding tables. The sum is an opaque
// token (it mixes a byte size with field counts), not a layout metric — it
// exists only to consume the query results. The traversal itself cannot be
// optimized away regardless: every query is a call through the dlopen'd
// library, which the host compiler cannot see into.
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
    Slang::ComPtr<slang::IBlob> diagnostics;
    slang::ProgramLayout* layout = linked->getLayout(0, diagnostics.writeRef());
    printDiagnostics(diagnostics);
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
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    {
        Scope s(timers, "apiFindEntryPoint");
        if (SLANG_FAILED(module->findEntryPointByName("computeMain", entryPoint.writeRef())))
        {
            printf(
                "error: findEntryPointByName(computeMain) failed for %s\n",
                moduleDisplayName(module));
            return false;
        }
    }

    slang::IComponentType* components[] = {module, entryPoint};
    Slang::ComPtr<slang::IComponentType> composite;
    Slang::ComPtr<slang::IBlob> diagnostics;
    {
        Scope s(timers, "apiComposite");
        SlangResult res = session->createCompositeComponentType(
            components,
            2,
            composite.writeRef(),
            diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (SLANG_FAILED(res))
            return false;
    }

    Slang::ComPtr<slang::IComponentType> linked;
    {
        Scope s(timers, "apiLink");
        SlangResult res = composite->link(linked.writeRef(), diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (SLANG_FAILED(res))
            return false;
    }

    Slang::ComPtr<slang::IBlob> code;
    {
        Scope s(timers, "apiGetCode");
        SlangResult res = linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (SLANG_FAILED(res) || !code || !code->getBufferSize())
        {
            printf("error: getEntryPointCode produced no code for %s\n", moduleDisplayName(module));
            return false;
        }
    }

    if (reflectWith)
    {
        long checksum = 0;
        if (!reflectProgram(*reflectWith, linked, timers, checksum))
            return false;
    }

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
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        long rssBeforeSession = currentRssKb();
        {
            Scope s(timers, "apiCreateGlobalSession");
            if (SLANG_FAILED(createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
            {
                printf("error: slang_createGlobalSession failed\n");
                return 1;
            }
        }
        recordSessionCreateRss(rssBeforeSession);
        Slang::ComPtr<slang::ISession> session;
        {
            Scope s(timers, "apiCreateSession");
            if (SLANG_FAILED(createSession(globalSession, "", session.writeRef())))
            {
                printf("error: createSession failed\n");
                return 1;
            }
        }
    }
    total.stop();
    timers.report();
    reportMemDeltas();
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

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    long rssBeforeSession = currentRssKb();
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(lib.createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
        {
            printf("error: slang_createGlobalSession failed\n");
            return 1;
        }
    }
    recordSessionCreateRss(rssBeforeSession);
    Slang::ComPtr<slang::ISession> session;
    {
        Scope s(timers, "apiCreateSession");
        if (SLANG_FAILED(createSession(globalSession, dir, session.writeRef())))
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
        Slang::ComPtr<slang::IBlob> diagnostics;
        {
            Scope s(timers, "apiLoadModule");
            module = session->loadModuleFromSourceString(
                name.c_str(),
                path.c_str(),
                source.c_str(),
                diagnostics.writeRef());
            printDiagnostics(diagnostics);
            if (!module)
                return 1;
        }
        if (!compileEntryPoint(session, module, timers, reflect ? &lib : nullptr))
            return 1;
    }

    total.stop();
    timers.report();
    reportMemDeltas();
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

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    long rssBeforeSession = currentRssKb();
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(lib.createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
        {
            printf("error: slang_createGlobalSession failed\n");
            return 1;
        }
    }
    recordSessionCreateRss(rssBeforeSession);
    Slang::ComPtr<slang::ISession> session;
    {
        Scope s(timers, "apiCreateSession");
        if (SLANG_FAILED(createSession(globalSession, dir, session.writeRef())))
        {
            printf("error: createSession failed\n");
            return 1;
        }
    }

    slang::IModule* module = nullptr;
    Slang::ComPtr<slang::IBlob> diagnostics;
    {
        Scope s(timers, "apiLoadModule");
        module = session->loadModule(root.c_str(), diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (!module)
            return 1;
    }
    if (!compileEntryPoint(session, module, timers, reflect ? &lib : nullptr))
        return 1;

    total.stop();
    timers.report();
    reportMemDeltas();
    return 0;
}

// module-graph-bin: like module-graph, but the timed load resolves the DAG
// from serialized .slang-module binaries instead of source — the import path
// where the 2026-07-03 module-loading regression (#11952) lives, which
// source-based loads do not exercise. Setup runs before the apiTotal scope
// opens, so it does not count toward apiTotal, but its phases are still timed
// and reported individually (apiLoadModuleSource, apiWriteModule): load the
// graph from source once and write every loaded module out with
// IModule::writeToFile; then a FRESH session re-loads the root, letting
// import resolution pick the binaries.
static int runModuleGraphBin(const LibSlang& lib, const std::string& dir, const std::string& root)
{
    Timers timers;

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    long rssBeforeSession = currentRssKb();
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(lib.createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
        {
            printf("error: slang_createGlobalSession failed\n");
            return 1;
        }
    }
    recordSessionCreateRss(rssBeforeSession);

    // Setup pass: source load + serialize every module in the graph.
    {
        Slang::ComPtr<slang::ISession> setupSession;
        if (SLANG_FAILED(createSession(globalSession, dir, setupSession.writeRef())))
        {
            printf("error: createSession failed\n");
            return 1;
        }
        Slang::ComPtr<slang::IBlob> diagnostics;
        slang::IModule* rootModule;
        {
            Scope s(timers, "apiLoadModuleSource");
            rootModule = setupSession->loadModule(root.c_str(), diagnostics.writeRef());
            printDiagnostics(diagnostics);
            if (!rootModule)
                return 1;
        }
        {
            Scope s(timers, "apiWriteModule");
            SlangInt count = setupSession->getLoadedModuleCount();
            for (SlangInt i = 0; i < count; i++)
            {
                slang::IModule* m = setupSession->getLoadedModule(i);
                // The binary's filename comes from the module name, so an
                // unnamed module can't take part in this workload — and the
                // generators always name their modules. Fail loudly rather
                // than concatenate a null name (UB).
                const char* name = m->getName();
                if (!name || !*name)
                {
                    printf(
                        "error: loaded module #%d has no name; cannot write a binary for it\n",
                        (int)i);
                    return 1;
                }
                std::string path = dir + "/" + name + ".slang-module";
                if (SLANG_FAILED(m->writeToFile(path.c_str())))
                {
                    printf("error: writeToFile failed for %s\n", path.c_str());
                    return 1;
                }
            }
        }
    }

    // Timed pass: fresh session; imports must now resolve to the binaries.
    Scope total(timers, "apiTotal");
    Slang::ComPtr<slang::ISession> session;
    {
        Scope s(timers, "apiCreateSession");
        if (SLANG_FAILED(createSession(globalSession, dir, session.writeRef())))
        {
            printf("error: createSession failed\n");
            return 1;
        }
    }
    slang::IModule* module = nullptr;
    Slang::ComPtr<slang::IBlob> diagnostics;
    {
        Scope s(timers, "apiLoadModule");
        module = session->loadModule(root.c_str(), diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (!module)
            return 1;
    }
    // The workload exists to measure the binary-module path: fail loudly if
    // import resolution fell back to a .slang source for ANY module — a
    // partial fallback would silently time a mixed source/binary graph.
    {
        SlangInt count = session->getLoadedModuleCount();
        bool allBinary = count > 0;
        for (SlangInt i = 0; i < count; i++)
        {
            const char* path = session->getLoadedModule(i)->getFilePath();
            std::string p = path ? path : "";
            if (p.size() <= 13 || p.compare(p.size() - 13, 13, ".slang-module") != 0)
            {
                printf(
                    "error: module '%s' resolved to '%s', not a .slang-module binary\n",
                    moduleDisplayName(session->getLoadedModule(i)),
                    p.c_str());
                allBinary = false;
            }
        }
        if (!allBinary)
            return 1;
    }
    if (!compileEntryPoint(session, module, timers))
        return 1;

    total.stop();
    timers.report();
    reportMemDeltas();
    return 0;
}

// specialize: one module declares an interface, N conforming impl structs,
// and a generic computeMain<T : IOp>. Each impl is compiled as its own
// specialized variant (findTypeByName -> SpecializationArg ->
// IEntryPoint::specialize -> composite -> link -> getEntryPointCode) — the
// application pattern where one kernel is stamped out per material/config
// type, stressing specialization + link per variant.
static int runSpecialize(
    const LibSlang& lib,
    const std::string& dir,
    const std::string& root,
    const std::string& implPrefix)
{
    if (!lib.reflFindTypeByName)
    {
        printf("error: spReflection_FindTypeByName missing from this libslang\n");
        return 1;
    }
    Timers timers;
    Scope total(timers, "apiTotal");

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    long rssBeforeSession = currentRssKb();
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(lib.createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
        {
            printf("error: slang_createGlobalSession failed\n");
            return 1;
        }
    }
    recordSessionCreateRss(rssBeforeSession);
    Slang::ComPtr<slang::ISession> session;
    {
        Scope s(timers, "apiCreateSession");
        if (SLANG_FAILED(createSession(globalSession, dir, session.writeRef())))
        {
            printf("error: createSession failed\n");
            return 1;
        }
    }

    slang::IModule* module = nullptr;
    Slang::ComPtr<slang::IBlob> diagnostics;
    {
        Scope s(timers, "apiLoadModule");
        module = session->loadModule(root.c_str(), diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (!module)
            return 1;
    }

    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    if (SLANG_FAILED(module->findEntryPointByName("computeMain", entryPoint.writeRef())))
    {
        printf("error: findEntryPointByName(computeMain) failed\n");
        return 1;
    }
    slang::ProgramLayout* moduleLayout = module->getLayout(0, diagnostics.writeRef());
    printDiagnostics(diagnostics);
    if (!moduleLayout)
    {
        printf("error: module getLayout failed\n");
        return 1;
    }

    for (int i = 0;; i++)
    {
        char implName[64];
        snprintf(implName, sizeof(implName), "%s%d", implPrefix.c_str(), i);
        auto type = (slang::TypeReflection*)lib.reflFindTypeByName(
            (SlangReflection*)moduleLayout,
            implName);
        if (!type)
        {
            if (i == 0)
            {
                printf("error: type %s0 not found in %s\n", implPrefix.c_str(), root.c_str());
                return 1;
            }
            break; // ran past the last generated impl
        }

        Slang::ComPtr<slang::IComponentType> specialized;
        {
            Scope s(timers, "apiSpecialize");
            slang::SpecializationArg arg = slang::SpecializationArg::fromType(type);
            SlangResult res =
                entryPoint->specialize(&arg, 1, specialized.writeRef(), diagnostics.writeRef());
            printDiagnostics(diagnostics);
            if (SLANG_FAILED(res))
                return 1;
        }

        slang::IComponentType* components[] = {module, specialized};
        Slang::ComPtr<slang::IComponentType> composite;
        {
            Scope s(timers, "apiComposite");
            SlangResult res = session->createCompositeComponentType(
                components,
                2,
                composite.writeRef(),
                diagnostics.writeRef());
            printDiagnostics(diagnostics);
            if (SLANG_FAILED(res))
                return 1;
        }
        Slang::ComPtr<slang::IComponentType> linked;
        {
            Scope s(timers, "apiLink");
            SlangResult res = composite->link(linked.writeRef(), diagnostics.writeRef());
            printDiagnostics(diagnostics);
            if (SLANG_FAILED(res))
                return 1;
        }
        Slang::ComPtr<slang::IBlob> code;
        {
            Scope s(timers, "apiGetCode");
            SlangResult res =
                linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
            printDiagnostics(diagnostics);
            if (SLANG_FAILED(res) || !code || !code->getBufferSize())
            {
                printf("error: getEntryPointCode produced no code for %s\n", implName);
                return 1;
            }
        }
    }

    total.stop();
    timers.report();
    reportMemDeltas();
    return 0;
}

// rt-composite: load a root module whose imports pull in the whole generated
// renderer library, then compose its raygen/closesthit/miss entry points into
// ONE program (createCompositeComponentType), link, and generate code for each
// entry — the ray-tracing pipeline shape applications compile, where every
// program pays the full library import cost (few×heavy, complementing
// many-kernels' many×tiny).
static int runRtComposite(const LibSlang& lib, const std::string& dir, const std::string& root)
{
    Timers timers;
    Scope total(timers, "apiTotal");

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    long rssBeforeSession = currentRssKb();
    {
        Scope s(timers, "apiCreateGlobalSession");
        if (SLANG_FAILED(lib.createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
        {
            printf("error: slang_createGlobalSession failed\n");
            return 1;
        }
    }
    recordSessionCreateRss(rssBeforeSession);
    Slang::ComPtr<slang::ISession> session;
    {
        Scope s(timers, "apiCreateSession");
        if (SLANG_FAILED(createSession(globalSession, dir, session.writeRef())))
        {
            printf("error: createSession failed\n");
            return 1;
        }
    }

    slang::IModule* module = nullptr;
    Slang::ComPtr<slang::IBlob> diagnostics;
    {
        Scope s(timers, "apiLoadModule");
        module = session->loadModule(root.c_str(), diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (!module)
            return 1;
    }

    static const char* kEntryNames[] = {"rayGenMain", "closestHitMain", "missMain"};
    const int kEntryCount = 3;
    slang::IComponentType* components[1 + kEntryCount] = {module};
    Slang::ComPtr<slang::IEntryPoint> entryPoints[kEntryCount];
    {
        Scope s(timers, "apiFindEntryPoint");
        for (int i = 0; i < kEntryCount; i++)
        {
            if (SLANG_FAILED(
                    module->findEntryPointByName(kEntryNames[i], entryPoints[i].writeRef())))
            {
                printf("error: findEntryPointByName(%s) failed\n", kEntryNames[i]);
                return 1;
            }
            components[1 + i] = entryPoints[i];
        }
    }

    Slang::ComPtr<slang::IComponentType> composite;
    {
        Scope s(timers, "apiComposite");
        SlangResult res = session->createCompositeComponentType(
            components,
            1 + kEntryCount,
            composite.writeRef(),
            diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (SLANG_FAILED(res))
            return 1;
    }
    Slang::ComPtr<slang::IComponentType> linked;
    {
        Scope s(timers, "apiLink");
        SlangResult res = composite->link(linked.writeRef(), diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (SLANG_FAILED(res))
            return 1;
    }
    for (int i = 0; i < kEntryCount; i++)
    {
        Slang::ComPtr<slang::IBlob> code;
        Scope s(timers, "apiGetCode");
        SlangResult res = linked->getEntryPointCode(i, 0, code.writeRef(), diagnostics.writeRef());
        printDiagnostics(diagnostics);
        if (SLANG_FAILED(res) || !code || !code->getBufferSize())
        {
            printf("error: getEntryPointCode produced no code for %s\n", kEntryNames[i]);
            return 1;
        }
    }

    total.stop();
    timers.report();
    reportMemDeltas();
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
               "       api-driver <libslang> specialize        --dir DIR --root NAME "
               "[--impl-prefix P]\n"
               "       api-driver <libslang> rt-composite      --dir DIR --root NAME\n");
        return 2;
    }
    LibSlang lib;
    if (!loadLibSlang(argv[1], lib))
        return 1;

    std::string mode = argv[2];
    bool reflect = hasFlag(argc, argv, "--reflect");
    if (mode == "session-create")
    {
        // Fail loudly on malformed --iters: atoi would silently turn garbage
        // into 0 and "benchmark" zero sessions.
        int iters = 10;
        if (const char* itersArg = argValue(argc, argv, "--iters"))
        {
            char* end = nullptr;
            long parsed = strtol(itersArg, &end, 10);
            if (!end || *end != '\0' || parsed <= 0 || parsed > 1000000)
            {
                printf("error: --iters must be a positive integer, got '%s'\n", itersArg);
                return 2;
            }
            iters = (int)parsed;
        }
        return runSessionCreate(lib.createGlobalSession, iters);
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
    if (mode == "rt-composite")
        return runRtComposite(lib, dir, root);
    if (mode == "specialize")
    {
        const char* prefix = argValue(argc, argv, "--impl-prefix");
        return runSpecialize(lib, dir, root, prefix ? prefix : "Impl_");
    }
    printf("error: unknown mode %s\n", mode.c_str());
    return 2;
}
