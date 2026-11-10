// unit-test-parallel-llvm-codegen.cpp

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <stdio.h>
#include <thread>

using namespace Slang;

// Several threads make the first LLVM code generation request of fresh global sessions at the same
// time. The documented parallel-backend workflow fans out different linked components, entry points
// or targets, so each thread compiles its own linked component. The threads alternate between two
// global sessions, which load slang-llvm separately but share LLVM's process-wide state.
//
// Every iteration races the per-session load of slang-llvm and the creation of LLVM builders.
// LLVM's target registry and option storage are initialized only by the first request in a
// process, and slang-test loads slang-llvm at startup and usually runs other LLVM tests first, so
// inside slang-test this test catches a regression of that initialization only occasionally.
SLANG_UNIT_TEST(parallelFirstLLVMCodegen)
{
    static constexpr int kIterationCount = 4;
    static constexpr int kThreadCount = 8;
    static constexpr int kGlobalSessionCount = 2;

    for (int iteration = 0; iteration < kIterationCount; ++iteration)
    {
        ComPtr<slang::IGlobalSession> globalSessions[kGlobalSessionCount];
        ComPtr<slang::ISession> sessions[kGlobalSessionCount];
        slang::IModule* modules[kGlobalSessionCount] = {};
        ComPtr<slang::IEntryPoint> entryPoints[kGlobalSessionCount];
        ComPtr<slang::IBlob> diagnostics;
        for (int sessionIndex = 0; sessionIndex < kGlobalSessionCount; ++sessionIndex)
        {
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(
                    SLANG_API_VERSION,
                    globalSessions[sessionIndex].writeRef()) == SLANG_OK);

            slang::TargetDesc targetDesc = {};
            targetDesc.format = SLANG_SHADER_LLVM_IR;
            slang::SessionDesc sessionDesc = {};
            sessionDesc.targetCount = 1;
            sessionDesc.targets = &targetDesc;
            SLANG_CHECK_ABORT(
                globalSessions[sessionIndex]->createSession(
                    sessionDesc,
                    sessions[sessionIndex].writeRef()) == SLANG_OK);

            modules[sessionIndex] = sessions[sessionIndex]->loadModule(
                "tests/llvm/parallel-first-codegen.slang",
                diagnostics.writeRef());
            SLANG_CHECK_ABORT(modules[sessionIndex] != nullptr);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(modules[sessionIndex]->findEntryPointByName(
                "computeMain",
                entryPoints[sessionIndex].writeRef())));
        }

        ComPtr<slang::IComponentType> programs[kThreadCount];
        for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex)
        {
            int sessionIndex = threadIndex % kGlobalSessionCount;
            slang::IComponentType* components[] = {
                modules[sessionIndex],
                entryPoints[sessionIndex].get()};
            ComPtr<slang::IComponentType> composite;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(sessions[sessionIndex]->createCompositeComponentType(
                components,
                SLANG_COUNT_OF(components),
                composite.writeRef(),
                diagnostics.writeRef())));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                composite->link(programs[threadIndex].writeRef(), diagnostics.writeRef())));
        }

        // The workers spin until all of them are running so that their first code generation
        // requests overlap as closely as possible.
        std::atomic<int> readyCount{0};
        std::atomic<bool> start{false};
        SlangResult results[kThreadCount];
        ComPtr<slang::IBlob> codes[kThreadCount];
        ComPtr<slang::IBlob> workerDiagnostics[kThreadCount];
        std::thread threads[kThreadCount];
        for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex)
        {
            threads[threadIndex] = std::thread(
                [&, threadIndex]()
                {
                    readyCount++;
                    while (!start)
                        std::this_thread::yield();
                    results[threadIndex] = programs[threadIndex]->getEntryPointCode(
                        0,
                        0,
                        codes[threadIndex].writeRef(),
                        workerDiagnostics[threadIndex].writeRef());
                });
        }
        while (readyCount < kThreadCount)
            std::this_thread::yield();
        start = true;
        for (auto& thread : threads)
            thread.join();

        // We check for slang-llvm only after the workers finish, because the check loads it into
        // the process through the downstream-compiler path, and a preloaded slang-llvm makes the
        // race much rarer. Without slang-llvm every worker fails, so results[0] stands for all.
        if (SLANG_FAILED(results[0]) &&
            SLANG_FAILED(globalSessions[0]->checkPassThroughSupport(SLANG_PASS_THROUGH_LLVM)))
        {
            SLANG_IGNORE_TEST;
        }

        for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex)
        {
            if (SLANG_FAILED(results[threadIndex]) && workerDiagnostics[threadIndex])
            {
                fprintf(
                    stderr,
                    "parallelFirstLLVMCodegen iteration %d thread %d: %.*s\n",
                    iteration,
                    threadIndex,
                    int(workerDiagnostics[threadIndex]->getBufferSize()),
                    (const char*)workerDiagnostics[threadIndex]->getBufferPointer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(results[threadIndex]));
            SLANG_CHECK_ABORT(codes[threadIndex] != nullptr);
            UnownedStringSlice code(
                (const char*)codes[threadIndex]->getBufferPointer(),
                codes[threadIndex]->getBufferSize());
            SLANG_CHECK(code.indexOf(toSlice("@computeMain(")) != -1);
        }
    }
}
