// unit-test-generic-entrypoint.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <condition_variable>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

using namespace Slang;

namespace
{

static int _getParallelGenericEntryPointIterationCount()
{
    static constexpr int kDefaultIterationCount = 20;

    const char* envValue = getenv("SLANG_PARALLEL_GENERIC_ENTRYPOINT_ITERATIONS");
    if (!envValue || !envValue[0])
        return kDefaultIterationCount;

    int iterationCount = atoi(envValue);
    return iterationCount > 0 ? iterationCount : kDefaultIterationCount;
}

struct ParallelThreadGate
{
    void arriveAndWait(int threadCount)
    {
        std::unique_lock<std::mutex> lock(mutex);
        readyCount++;
        if (readyCount == threadCount)
            condition.notify_all();
        condition.wait(lock, [&]() { return shouldStart; });
    }

    void releaseWhenReady(int threadCount)
    {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [&]() { return readyCount == threadCount; });
        shouldStart = true;
        condition.notify_all();
    }

    std::mutex mutex;
    std::condition_variable condition;
    int readyCount = 0;
    bool shouldStart = false;
};

static String _blobToString(slang::IBlob* blob)
{
    if (!blob)
        return String();
    return String(UnownedStringSlice((const char*)blob->getBufferPointer(), blob->getBufferSize()));
}

struct ParallelThreadResult
{
    SlangResult specializeResult = SLANG_OK;
    SlangResult composeResult = SLANG_OK;
    SlangResult linkResult = SLANG_OK;
    SlangResult codeResult = SLANG_OK;
    String diagnostics;
    String code;
};

enum class ParallelGenericEntryPointBackend
{
    HLSL,
    CUDA,
    SPIRV,
    Metal,
};

struct ParallelGenericEntryPointTarget
{
    const char* name;
    ParallelGenericEntryPointBackend backend;
    SlangCompileTarget format;
    const char* profileName;
};

static void _initializeTargetDesc(
    slang::IGlobalSession* globalSession,
    const ParallelGenericEntryPointTarget& target,
    slang::TargetDesc& outTargetDesc)
{
    outTargetDesc = {};
    outTargetDesc.format = target.format;
    if (target.profileName)
        outTargetDesc.profile = globalSession->findProfile(target.profileName);
}

static void _verifyParallelGenericEntryPointCode(
    const ParallelGenericEntryPointTarget& target,
    const String& code,
    int specializationValue)
{
    const int emittedValue = specializationValue * 100 + 7;

    UnownedStringSlice codeSlice(code.getUnownedSlice());

    StringBuilder expectedLiteral;
    expectedLiteral << emittedValue;
    SLANG_CHECK_ABORT(codeSlice.indexOf(expectedLiteral.getUnownedSlice()) != -1);

    switch (target.backend)
    {
    case ParallelGenericEntryPointBackend::HLSL:
        {
            StringBuilder expectedNumThreads;
            expectedNumThreads << "[numthreads(" << specializationValue << ", 1, 1)]";
            SLANG_CHECK_ABORT(codeSlice.indexOf(expectedNumThreads.getUnownedSlice()) != -1);
            SLANG_CHECK_ABORT(codeSlice.indexOf(toSlice("SV_DispatchThreadID")) != -1);
            break;
        }
    case ParallelGenericEntryPointBackend::CUDA:
        SLANG_CHECK_ABORT(codeSlice.indexOf(toSlice("__global__ void computeMain")) != -1);
        SLANG_CHECK_ABORT(codeSlice.indexOf(toSlice("(blockIdx * blockDim + threadIdx).x")) != -1);
        break;
    case ParallelGenericEntryPointBackend::SPIRV:
        {
            StringBuilder expectedExecutionMode;
            expectedExecutionMode << "OpExecutionMode %computeMain LocalSize " << specializationValue
                                  << " 1 1";
            SLANG_CHECK_ABORT(codeSlice.indexOf(toSlice("OpEntryPoint GLCompute %computeMain")) != -1);
            SLANG_CHECK_ABORT(codeSlice.indexOf(expectedExecutionMode.getUnownedSlice()) != -1);
            SLANG_CHECK_ABORT(codeSlice.indexOf(toSlice("OpConstant %int")) != -1);
            break;
        }
    case ParallelGenericEntryPointBackend::Metal:
        SLANG_CHECK_ABORT(codeSlice.indexOf(toSlice("[[kernel]] void computeMain")) != -1);
        SLANG_CHECK_ABORT(codeSlice.indexOf(toSlice("[[thread_position_in_grid]]")) != -1);
        break;
    }
}

static void _runParallelGenericEntryPointCompileForTarget(
    slang::IGlobalSession* globalSession,
    const ParallelGenericEntryPointTarget& target,
    int iterationCount)
{
    static constexpr int kThreadCount = 20;

    const char* userSourceBody = R"(
        RWStructuredBuffer<int> outputBuffer;

        [shader("compute")]
        [numthreads(x, 1, 1)]
        void computeMain<int x>(uint3 tid : SV_DispatchThreadID)
        {
            outputBuffer[tid.x] = x * 100 + 7;
        }
        )";

    slang::TargetDesc targetDesc = {};
    _initializeTargetDesc(globalSession, target, targetDesc);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    if (!module && diagnosticBlob)
    {
        fprintf(
            stderr,
            "parallelGenericEntryPointCompile target %s module diagnostics:\n%s\n",
            target.name,
            (const char*)diagnosticBlob->getBufferPointer());
    }
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    if (!entryPoint && diagnosticBlob)
    {
        fprintf(
            stderr,
            "parallelGenericEntryPointCompile target %s entry point diagnostics:\n%s\n",
            target.name,
            (const char*)diagnosticBlob->getBufferPointer());
    }
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    for (int iterationIndex = 0; iterationIndex < iterationCount; ++iterationIndex)
    {
        ParallelThreadGate gate;
        ParallelThreadResult results[kThreadCount];
        std::thread threads[kThreadCount];

        for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex)
        {
            threads[threadIndex] = std::thread(
                [&, threadIndex]()
                {
                    const int specializationValue = threadIndex + 1;

                    ComPtr<slang::IBlob> diagnostics;
                    StringBuilder specializationExprBuilder;
                    specializationExprBuilder << specializationValue;
                    String specializationExpr = specializationExprBuilder;
                    slang::SpecializationArg arg =
                        slang::SpecializationArg::fromExpr(specializationExpr.getBuffer());

                    ComPtr<slang::IComponentType> specializedEntryPoint;
                    results[threadIndex].specializeResult = entryPoint->specialize(
                        &arg,
                        1,
                        specializedEntryPoint.writeRef(),
                        diagnostics.writeRef());
                    if (SLANG_FAILED(results[threadIndex].specializeResult))
                        results[threadIndex].diagnostics = _blobToString(diagnostics);

                    ComPtr<slang::IComponentType> composedProgram;
                    if (SLANG_SUCCEEDED(results[threadIndex].specializeResult))
                    {
                        slang::IComponentType* componentTypes[] = {module, specializedEntryPoint.get()};
                        results[threadIndex].composeResult = session->createCompositeComponentType(
                            componentTypes,
                            2,
                            composedProgram.writeRef(),
                            diagnostics.writeRef());
                        if (SLANG_FAILED(results[threadIndex].composeResult))
                            results[threadIndex].diagnostics = _blobToString(diagnostics);
                    }

                    ComPtr<slang::IComponentType> linkedProgram;
                    if (SLANG_SUCCEEDED(results[threadIndex].composeResult))
                    {
                        results[threadIndex].linkResult =
                            composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef());
                        if (SLANG_FAILED(results[threadIndex].linkResult))
                            results[threadIndex].diagnostics = _blobToString(diagnostics);
                    }

                    gate.arriveAndWait(kThreadCount);

                    if (SLANG_SUCCEEDED(results[threadIndex].linkResult))
                    {
                        ComPtr<slang::IBlob> codeBlob;
                        results[threadIndex].codeResult = linkedProgram->getEntryPointCode(
                            0,
                            0,
                            codeBlob.writeRef(),
                            diagnostics.writeRef());
                        if (SLANG_FAILED(results[threadIndex].codeResult))
                        {
                            results[threadIndex].diagnostics = _blobToString(diagnostics);
                        }
                        else
                        {
                            results[threadIndex].code = _blobToString(codeBlob);
                        }
                    }
                });
        }

        gate.releaseWhenReady(kThreadCount);

        for (auto& thread : threads)
            thread.join();

        for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex)
        {
            if (results[threadIndex].diagnostics.getLength())
            {
                fprintf(
                    stderr,
                    "parallelGenericEntryPointCompile target %s iteration %d thread %d diagnostics:\n%s\n",
                    target.name,
                    iterationIndex,
                    threadIndex,
                    results[threadIndex].diagnostics.getBuffer());
            }

            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(results[threadIndex].specializeResult));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(results[threadIndex].composeResult));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(results[threadIndex].linkResult));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(results[threadIndex].codeResult));
            SLANG_CHECK_ABORT(results[threadIndex].code.getLength() != 0);

            _verifyParallelGenericEntryPointCode(
                target,
                results[threadIndex].code,
                threadIndex + 1);
        }

        for (int ii = 0; ii < kThreadCount; ++ii)
        {
            for (int jj = ii + 1; jj < kThreadCount; ++jj)
            {
                SLANG_CHECK_ABORT(results[ii].code != results[jj].code);
            }
        }
    }
}

} // namespace

// Test the compilation API for compiling a specialized generic entrypoint.

SLANG_UNIT_TEST(genericEntryPointCompile)
{
    const char* userSourceBody = R"(
            interface I { int getValue(); }
            struct X : I { int getValue() { return 100; } }
            float4 vertMain<T:I, int n, each U>(uniform T o) {
                return float4(o.getValue(), countof(U), n, 1);
            }
        )";
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_GLSL;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    // Test 1: Using findAndCheckEntryPoint to supply arguments in string form.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "vertMain<X, 7, int, float>",
            SLANG_STAGE_VERTEX,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);
        slang::IComponentType* componentTypes[2] = {module, entryPoint.get()};
        ComPtr<slang::IComponentType> composedProgram;
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

        ComPtr<slang::IBlob> code;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());

        SLANG_CHECK(
            UnownedStringSlice((char*)code->getBufferPointer())
                .indexOf(toSlice("vec4(float(X_getValue_0()), 2.0, 7.0, 1.0)")) != -1);
    }

    // Test 2: Using `specialize` to supply arguments structurally with reflection types.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "vertMain",
            SLANG_STAGE_VERTEX,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);
        ComPtr<slang::IComponentType> specializedEntryPoint;
        slang::SpecializationArg args[] = {
            slang::SpecializationArg::fromType(module->getLayout()->findTypeByName("X")),
            slang::SpecializationArg::fromExpr("8"),
            slang::SpecializationArg::fromType(module->getLayout()->findTypeByName("int")),
            slang::SpecializationArg::fromType(module->getLayout()->findTypeByName("float"))};

        entryPoint->specialize(args, 4, specializedEntryPoint.writeRef(), nullptr);
        SLANG_CHECK_ABORT(specializedEntryPoint != nullptr);
        slang::IComponentType* componentTypes[2] = {module, specializedEntryPoint.get()};
        ComPtr<slang::IComponentType> composedProgram;
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

        ComPtr<slang::IBlob> code;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());

        SLANG_CHECK(
            UnownedStringSlice((char*)code->getBufferPointer())
                .indexOf(toSlice("vec4(float(X_getValue_0()), 2.0, 8.0, 1.0)")) != -1);
    }

    // Test 3: corner case: specialize variadic param with 0 types.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "vertMain",
            SLANG_STAGE_VERTEX,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);
        ComPtr<slang::IComponentType> specializedEntryPoint;
        slang::SpecializationArg args[] = {
            slang::SpecializationArg::fromType(module->getLayout()->findTypeByName("X")),
            slang::SpecializationArg::fromExpr("8")};

        entryPoint->specialize(args, 2, specializedEntryPoint.writeRef(), nullptr);
        SLANG_CHECK_ABORT(specializedEntryPoint != nullptr);
        slang::IComponentType* componentTypes[2] = {module, specializedEntryPoint.get()};
        ComPtr<slang::IComponentType> composedProgram;
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

        ComPtr<slang::IBlob> code;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());

        SLANG_CHECK(
            UnownedStringSlice((char*)code->getBufferPointer())
                .indexOf(toSlice("vec4(float(X_getValue_0()), 0.0, 8.0, 1.0)")) != -1);
    }
}

SLANG_UNIT_TEST(parallelGenericEntryPointCompile)
{
    const int iterationCount = _getParallelGenericEntryPointIterationCount();

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    const ParallelGenericEntryPointTarget targets[] = {
        {"hlsl", ParallelGenericEntryPointBackend::HLSL, SLANG_HLSL, nullptr},
        {"cuda", ParallelGenericEntryPointBackend::CUDA, SLANG_CUDA_SOURCE, nullptr},
        {"spirv", ParallelGenericEntryPointBackend::SPIRV, SLANG_SPIRV_ASM, "spirv_1_5"},
        {"metal", ParallelGenericEntryPointBackend::Metal, SLANG_METAL, nullptr},
    };

    for (const auto& target : targets)
    {
        _runParallelGenericEntryPointCompileForTarget(
            globalSession,
            target,
            iterationCount);
    }
}
