// unit-test-library-unload-memory.cpp

#include "core/slang-io.h"
#include "core/slang-math.h"
#include "core/slang-platform.h"
#include "core/slang-process.h"
#include "core/slang-shared-library.h"
#include "slang-com-ptr.h"
#include "slang-glslang/slang-glslang.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <condition_variable>
#include <mutex>
#include <thread>

// The unload hooks under test exist only in shared MSVC builds (see
// source/slang/slang-mimalloc.cpp), and the memory thresholds below assume 64-bit arenas. Every
// target that links `core` sees `SLANG_STATIC`, so `SLANG_DYNAMIC` is what says that slang-compiler
// itself is a DLL.
#if defined(_MSC_VER) && SLANG_PTR_IS_64 && defined(SLANG_DYNAMIC)
#define SLANG_UNLOAD_TEST_ENABLED 1
#endif

#if SLANG_UNLOAD_TEST_ENABLED
// clang-format off
// psapi.h uses windows.h types without including it, so the order matters.
#include <windows.h>
#include <psapi.h>
// clang-format on
#endif

using namespace Slang;

// slang-compiler.dll and slang-glslang.dll each release their mimalloc memory when they are
// unloaded (see the `DllMain` in source/slang/slang-mimalloc.cpp). Without that, every unload left
// at least one 1 GiB arena reservation behind on 64-bit, plus the pages of threads that were still
// running (issue #13828).
//
// We check this by repeatedly loading a private copy of each DLL, compiling something on a worker
// thread and on this thread, and unloading the copy while the worker is still alive. The copy is
// needed because the test harness already has the original DLL loaded, so `FreeLibrary` on the
// original would only drop a reference instead of unmapping the module.

#if SLANG_UNLOAD_TEST_ENABLED

static const int kCycleCount = 10;
static const uint64_t kMiB = 1024 * 1024;

// One mimalloc arena reservation is 1 GiB; seeing most of it proves the arena path was exercised.
static const uint64_t kMinArenaGrowth = 768 * kMiB;

// A fixed DLL leaves only a few KiB of per-thread mimalloc metadata behind on each cycle, whereas
// an unfixed one leaves at least 1 GiB. The bound leaves room for loader and CRT bookkeeping noise.
static const uint64_t kMaxResidualGrowth = 64 * kMiB;

struct MemorySample
{
    uint64_t usedVirtual = 0;
    uint64_t privateBytes = 0;
};

static MemorySample sampleMemory()
{
    MemorySample sample;

    MEMORYSTATUSEX status = {};
    status.dwLength = sizeof(status);
    SLANG_CHECK_ABORT(GlobalMemoryStatusEx(&status));
    sample.usedVirtual = status.ullTotalVirtual - status.ullAvailVirtual;

    PROCESS_MEMORY_COUNTERS_EX counters = {};
    counters.cb = sizeof(counters);
    SLANG_CHECK_ABORT(K32GetProcessMemoryInfo(
        GetCurrentProcess(),
        (PROCESS_MEMORY_COUNTERS*)&counters,
        sizeof(counters)));
    sample.privateBytes = counters.PrivateUsage;

    return sample;
}

static uint64_t growth(uint64_t from, uint64_t to)
{
    return to > from ? to - from : 0;
}

/// A thread that runs one piece of work and then stays alive, idle, until it is destroyed. It lets
/// the test unload a DLL while a thread that allocated through the DLL still exists, which is the
/// case where mimalloc otherwise keeps that thread's pages committed.
class IdleWorker
{
public:
    template<typename F>
    explicit IdleWorker(F work)
    {
        m_thread = std::thread(
            [this, work]()
            {
                work();
                std::unique_lock<std::mutex> lock(m_mutex);
                m_workDone = true;
                m_condition.notify_all();
                m_condition.wait(lock, [this]() { return m_exitRequested; });
            });
    }

    void waitUntilWorkDone()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this]() { return m_workDone; });
    }

    ~IdleWorker()
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_exitRequested = true;
        }
        m_condition.notify_all();
        m_thread.join();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_condition;
    bool m_workDone = false;
    bool m_exitRequested = false;
    std::thread m_thread;
};

/// Copy the DLL at `sourcePath` to `<prefix>-<process id>.dll` in the same directory and return the
/// copy's path in `outPath`. The process id keeps concurrent test servers from sharing a copy.
static SlangResult makePrivateCopy(const String& sourcePath, const char* prefix, String& outPath)
{
    StringBuilder copyPath;
    copyPath << Path::getParentDirectory(sourcePath) << "\\" << prefix << "-" << Process::getId()
             << ".dll";
    if (!CopyFileW(sourcePath.toWString(), copyPath.toString().toWString(), FALSE))
        return SLANG_FAIL;
    outPath = copyPath.toString();
    return SLANG_OK;
}

/// Return whether any module is still mapped at the address `library` was loaded at.
static bool isMapped(SharedLibrary::Handle library)
{
    HMODULE module = nullptr;
    return GetModuleHandleExW(
               GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                   GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
               (LPCWSTR)library,
               &module) != FALSE;
}

static bool isSpirv(const void* data, size_t size)
{
    return size >= sizeof(uint32_t) && *(const uint32_t*)data == 0x07230203;
}

/// Compile a small compute shader to SPIR-V through the `slang_createGlobalSession` exported by
/// the copied slang-compiler DLL, and return whether it produced SPIR-V.
static bool compileWithSlang(SharedLibrary::Handle library)
{
    typedef SlangResult (*CreateGlobalSessionFunc)(SlangInt, slang::IGlobalSession**);
    auto createGlobalSession = (CreateGlobalSessionFunc)SharedLibrary::findSymbolAddressByName(
        library,
        "slang_createGlobalSession");
    if (!createGlobalSession)
        return false;

    ComPtr<slang::IGlobalSession> globalSession;
    if (SLANG_FAILED(createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
        return false;

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    // Without optimization or validation the copy does not load slang-glslang, which is checked on
    // its own. Validation is turned off explicitly because CI exports SLANG_RUN_SPIRV_VALIDATION=1.
    slang::CompilerOptionEntry options[2] = {};
    options[0].name = slang::CompilerOptionName::Optimization;
    options[0].value.intValue0 = SLANG_OPTIMIZATION_LEVEL_NONE;
    options[1].name = slang::CompilerOptionName::SkipSPIRVValidation;
    options[1].value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = options;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(options);

    ComPtr<slang::ISession> session;
    if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
        return false;

    const char* source = R"(
        RWStructuredBuffer<float> output;
        [shader("compute")]
        [numthreads(4, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            output[tid.x] = float(tid.x) * 2.0f;
        }
        )";
    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module =
        session->loadModuleFromSourceString("m", "m.slang", source, diagnostics.writeRef());
    if (!module)
        return false;

    ComPtr<slang::IEntryPoint> entryPoint;
    if (SLANG_FAILED(module->findEntryPointByName("computeMain", entryPoint.writeRef())))
        return false;

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> composite;
    if (SLANG_FAILED(session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            composite.writeRef(),
            diagnostics.writeRef())))
        return false;

    ComPtr<slang::IComponentType> linked;
    if (SLANG_FAILED(composite->link(linked.writeRef(), diagnostics.writeRef())))
        return false;

    ComPtr<slang::IBlob> code;
    if (SLANG_FAILED(linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())))
        return false;
    return isSpirv(code->getBufferPointer(), code->getBufferSize());
}

static void appendOutput(void const* data, size_t size, void* userData)
{
    ((List<uint8_t>*)userData)->addRange((const uint8_t*)data, Index(size));
}

/// Compile a small GLSL compute shader to optimized SPIR-V through the `glslang_compile_1_3`
/// exported by the copied slang-glslang DLL. Optimization makes the compile run SPIRV-Tools-opt as
/// well as glslang, so it reaches the code that brings mimalloc into slang-glslang.
static bool compileWithGlslang(SharedLibrary::Handle library)
{
    auto compile = (glslang_CompileFunc_1_3)SharedLibrary::findSymbolAddressByName(
        library,
        "glslang_compile_1_3");
    if (!compile)
        return false;

    const char* source = R"(#version 450
        layout(local_size_x = 4) in;
        layout(std430, binding = 0) buffer Output { float values[]; };
        void main() { values[gl_GlobalInvocationID.x] = float(gl_GlobalInvocationID.x) * 2.0; }
        )";

    List<uint8_t> spirv;
    List<uint8_t> diagnostics;

    glslang_CompileRequest_1_3 request;
    memset(&request, 0, sizeof(request));
    request.sizeInBytes = sizeof(request);
    request.action = GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV;
    request.sourcePath = "unload.comp";
    request.slangStage = SLANG_STAGE_COMPUTE;
    request.inputBegin = source;
    request.inputEnd = source + strlen(source);
    request.outputFunc = appendOutput;
    request.outputUserData = &spirv;
    request.diagnosticFunc = appendOutput;
    request.diagnosticUserData = &diagnostics;
    request.optimizationLevel = SLANG_OPTIMIZATION_LEVEL_DEFAULT;
    request.debugInfoType = SLANG_DEBUG_INFO_LEVEL_NONE;
    request.entryPointName = "main";

    if (compile(&request) != 0)
        return false;
    return isSpirv(spirv.getBuffer(), size_t(spirv.getCount()));
}

typedef bool (*CompileThroughLibraryFunc)(SharedLibrary::Handle library);

/// Repeatedly load the DLL at `copyPath`, compile through it on an idle worker and on this thread,
/// and unload it while the worker is still alive; then check that the unloads gave the DLL's
/// memory back. If `expectArena` is set the build links mimalloc into the DLL, and every cycle must
/// also show an arena reservation before the unload, so that a pass cannot come from a run that
/// never reached the memory that used to leak.
static void checkUnloadReleasesMemory(
    const String& copyPath,
    CompileThroughLibraryFunc compileFunc,
    bool expectArena)
{
    MemorySample firstAfterUnload;
    MemorySample lastAfterUnload;
    uint64_t smallestArenaGrowth = ~uint64_t(0);

    for (int cycle = 0; cycle < kCycleCount; ++cycle)
    {
        const MemorySample beforeLoad = sampleMemory();

        SharedLibrary::Handle library = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(SharedLibrary::loadWithPlatformPath(copyPath.getBuffer(), library)));

        // `worker` lives until the end of this iteration, so its thread is still alive when the DLL
        // is unloaded below, which is the case where mimalloc used to keep that thread's pages.
        bool workerResult = false;
        IdleWorker worker([&]() { workerResult = compileFunc(library); });
        worker.waitUntilWorkDone();
        SLANG_CHECK(workerResult);
        SLANG_CHECK(compileFunc(library));

        const MemorySample beforeUnload = sampleMemory();
        smallestArenaGrowth = Math::Min(
            smallestArenaGrowth,
            growth(beforeLoad.usedVirtual, beforeUnload.usedVirtual));

        SharedLibrary::unload(library);
        SLANG_CHECK_ABORT(!isMapped(library));

        const MemorySample afterUnload = sampleMemory();
        if (cycle == 0)
            firstAfterUnload = afterUnload;
        lastAfterUnload = afterUnload;
    }

    const uint64_t virtualGrowth =
        growth(firstAfterUnload.usedVirtual, lastAfterUnload.usedVirtual);
    const uint64_t privateGrowth =
        growth(firstAfterUnload.privateBytes, lastAfterUnload.privateBytes);

    const bool arenaExpectationMet = !expectArena || smallestArenaGrowth >= kMinArenaGrowth;
    const bool releasedVirtual = virtualGrowth < kMaxResidualGrowth;
    const bool releasedPrivate = privateGrowth < kMaxResidualGrowth;

    StringBuilder message;
    message << Path::getFileName(copyPath) << ": smallest arena growth "
            << smallestArenaGrowth / kMiB << " MiB; over " << (kCycleCount - 1)
            << " further cycles virtual +" << virtualGrowth / 1024 << " KiB, private +"
            << privateGrowth / 1024 << " KiB\n";
    const bool passed = arenaExpectationMet && releasedVirtual && releasedPrivate;
    getTestReporter()->message(
        passed ? TestMessageType::Info : TestMessageType::TestFailure,
        message.getBuffer());

    SLANG_CHECK(arenaExpectationMet);
    SLANG_CHECK(releasedVirtual);
    SLANG_CHECK(releasedPrivate);
}

#endif

SLANG_UNIT_TEST(libraryUnloadReleasesMemory)
{
#if SLANG_UNLOAD_TEST_ENABLED
    // Without an embedded core module every cycle would compile the core module from source.
    if (slang_getEmbeddedCoreModule() == nullptr)
    {
        SLANG_IGNORE_TEST;
    }

#if defined(SLANG_ENABLE_MIMALLOC)
    const bool slangUsesMimalloc = true;
#else
    const bool slangUsesMimalloc = false;
#endif
#if defined(SLANG_GLSLANG_USES_MIMALLOC)
    const bool glslangUsesMimalloc = true;
#else
    const bool glslangUsesMimalloc = false;
#endif

    {
        const String slangPath =
            SharedLibraryUtils::getSharedLibraryFileName((void*)slang_createGlobalSession);
        String slangCopy;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(makePrivateCopy(slangPath, "slang-compiler-unload-test", slangCopy)));
        SLANG_DEFER(File::remove(slangCopy));
        checkUnloadReleasesMemory(slangCopy, compileWithSlang, slangUsesMimalloc);
    }

    ComPtr<ISlangBlob> glslangPath;
    if (SLANG_SUCCEEDED(unitTestContext->slangGlobalSession->checkPassThroughSupport(
            SLANG_PASS_THROUGH_GLSLANG)) &&
        SLANG_SUCCEEDED(unitTestContext->slangGlobalSession->getDownstreamCompilerPath(
            SLANG_PASS_THROUGH_GLSLANG,
            glslangPath.writeRef())))
    {
        String glslangCopy;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(makePrivateCopy(
            String((const char*)glslangPath->getBufferPointer()),
            "slang-glslang-unload-test",
            glslangCopy)));
        SLANG_DEFER(File::remove(glslangCopy));
        checkUnloadReleasesMemory(glslangCopy, compileWithGlslang, glslangUsesMimalloc);
    }
    else
    {
        // A build that links mimalloc into slang-glslang must also be able to load it.
        SLANG_CHECK(!glslangUsesMimalloc);
    }
#else
    SLANG_IGNORE_TEST;
#endif
}
