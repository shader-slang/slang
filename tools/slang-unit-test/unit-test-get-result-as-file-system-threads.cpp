// unit-test-get-result-as-file-system-threads.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <thread>
#include <vector>

using namespace Slang;

// `getResultAsFileSystem` adds a diagnostics association (and obfuscated source-map associations)
// to the entry point's cached result artifact. Calling it from several threads on the same linked
// program, so that they share one artifact, used to race on the artifact's association list (a
// check-then-add with no lock), which could corrupt the list.

SLANG_UNIT_TEST(getResultAsFileSystemParallel)
{
    const char* userSourceBody = R"(
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID, uniform RWStructuredBuffer<int> o)
        {
            o[0] = 1;
        }
        )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    constexpr int kRoundCount = 20;
    constexpr int kThreadCount = 8;

    int failureCount = 0;
    for (int round = 0; round < kRoundCount; ++round)
    {
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HLSL;
        targetDesc.profile = globalSession->findProfile("sm_6_0");
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        auto module = session->loadModuleFromSourceString(
            "m",
            "m.slang",
            userSourceBody,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);

        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_CHECK_ABORT(
            module->findEntryPointByName("computeMain", entryPoint.writeRef()) == SLANG_OK);

        slang::IComponentType* components[] = {module, entryPoint.get()};
        ComPtr<slang::IComponentType> composite;
        SLANG_CHECK_ABORT(
            session->createCompositeComponentType(
                components,
                2,
                composite.writeRef(),
                diagnostics.writeRef()) == SLANG_OK);
        ComPtr<slang::IComponentType> linked;
        SLANG_CHECK_ABORT(composite->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

        // Compile once up front so that all threads work on the same cached artifact.
        ComPtr<slang::IBlob> code;
        SLANG_CHECK_ABORT(
            linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) == SLANG_OK);

        std::atomic<bool> go{false};
        std::atomic<int> failures{0};
        std::vector<std::thread> threads;
        for (int i = 0; i < kThreadCount; ++i)
        {
            threads.emplace_back(
                [&]()
                {
                    while (!go.load())
                        std::this_thread::yield();
                    ComPtr<ISlangMutableFileSystem> fileSystem;
                    if (SLANG_FAILED(linked->getResultAsFileSystem(0, 0, fileSystem.writeRef())) ||
                        !fileSystem)
                        failures++;
                });
        }
        go = true;
        for (auto& thread : threads)
            thread.join();
        failureCount += failures.load();
    }
    SLANG_CHECK(failureCount == 0);
}
