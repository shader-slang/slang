// unit-test-parallel-llvm-codegen.cpp

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <stdio.h>
#include <thread>

using namespace Slang;

// Several threads make the first LLVM code generation request of a fresh global session at the
// same time, each on its own fully linked component. That request loads slang-llvm into the global
// session and, in the first such request of the process, initializes LLVM's process-global target
// registry and command-line options. Each iteration uses a new global session so that every
// iteration starts with slang-llvm not yet loaded into that session.
SLANG_UNIT_TEST(parallelFirstLLVMCodegen)
{
    static constexpr int kIterationCount = 4;
    static constexpr int kThreadCount = 8;

    for (int iteration = 0; iteration < kIterationCount; ++iteration)
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SHADER_LLVM_IR;
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        slang::IModule* module =
            session->loadModule("tests/llvm/parallel-first-codegen.slang", diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);
        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(module->findEntryPointByName("computeMain", entryPoint.writeRef())));

        ComPtr<slang::IComponentType> programs[kThreadCount];
        for (auto& program : programs)
        {
            slang::IComponentType* components[] = {module, entryPoint.get()};
            ComPtr<slang::IComponentType> composite;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
                components,
                SLANG_COUNT_OF(components),
                composite.writeRef(),
                diagnostics.writeRef())));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(composite->link(program.writeRef(), diagnostics.writeRef())));
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

        // We check for slang-llvm only after the workers finish, so that their requests are the
        // ones that load it.
        if (SLANG_FAILED(results[0]) &&
            SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_LLVM)))
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
