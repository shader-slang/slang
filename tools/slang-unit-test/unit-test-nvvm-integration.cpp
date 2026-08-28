// unit-test-nvvm-integration.cpp

#include "unit-test-nvvm-support.h"

SLANG_UNIT_TEST(nvvmSlangRealEmptyCompute)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct Slang NVVM test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    const SlangResult compileResult = _compileSlangWithDirectNVVM(
        globalSession,
        kDirectNVVMEmptyComputeSource,
        code,
        diagnostics);
    if (SLANG_FAILED(compileResult))
    {
        const String diagnosticText = _getBlobText(diagnostics);
        if (diagnosticText.getLength())
            getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(code != nullptr);
    SLANG_CHECK(_getBlobText(code).indexOf(".visible .entry computeMain") >= 0);
}

SLANG_UNIT_TEST(nvvmSlangRealEmptyComputePtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct Slang NVVM ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct Slang NVVM ptxas test because libNVVM was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
        globalSession,
        kDirectNVVMEmptyComputeSource,
        code,
        diagnostics)));
    ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
    SLANG_CHECK_ABORT(ptxArtifact != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
}

SLANG_UNIT_TEST(nvvmSlangRealScalarDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarFunctions());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring Slang scalar PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    struct ScalarCase
    {
        const char* source;
        const uint32_t* parameterWidths;
        Index parameterCount;
        bool expectsLoad;
        bool expectsAdd;
        bool expectsSignedComparison;
    };
    static const uint32_t kWriteWidths[] = {64, 32};
    static const uint32_t kCopyWidths[] = {64, 64};
    static const uint32_t kChooseWidths[] = {64, 32, 32};
    static const ScalarCase kCases[] = {
        {kDirectNVVMWriteScalarSource,
         kWriteWidths,
         SLANG_COUNT_OF(kWriteWidths),
         false,
         false,
         false},
        {kDirectNVVMCopyScalarSource, kCopyWidths, SLANG_COUNT_OF(kCopyWidths), true, false, false},
        {kDirectNVVMChooseScalarSource,
         kChooseWidths,
         SLANG_COUNT_OF(kChooseWidths),
         false,
         true,
         true},
        {kDirectNVVMIntegerConstantSource,
         kWriteWidths,
         SLANG_COUNT_OF(kWriteWidths),
         false,
         true,
         false},
        {kDirectNVVMMergePhiSource,
         kChooseWidths,
         SLANG_COUNT_OF(kChooseWidths),
         false,
         false,
         false},
        {kDirectNVVMFiniteLoopSource,
         kWriteWidths,
         SLANG_COUNT_OF(kWriteWidths),
         false,
         false,
         false},
        {kDirectNVVMScalarFunctionSource,
         kWriteWidths,
         SLANG_COUNT_OF(kWriteWidths),
         false,
         false,
         false},
    };

    for (const auto& scalarCase : kCases)
    {
        ComPtr<slang::IBlob> nvvmCode;
        ComPtr<slang::IBlob> nvvmDiagnostics;
        const SlangResult nvvmResult = _compileSlangWithPTXMethod(
            globalSession,
            scalarCase.source,
            SLANG_EMIT_CUDA_VIA_NVVM,
            nvvmCode,
            nvvmDiagnostics);
        if (SLANG_FAILED(nvvmResult))
        {
            const String text = _getBlobText(nvvmDiagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
        SLANG_CHECK_ABORT(nvvmCode != nullptr);

        ComPtr<slang::IBlob> nvrtcCode;
        ComPtr<slang::IBlob> nvrtcDiagnostics;
        const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
            globalSession,
            scalarCase.source,
            SLANG_EMIT_CUDA_VIA_NVRTC,
            nvrtcCode,
            nvrtcDiagnostics);
        if (SLANG_FAILED(nvrtcResult))
        {
            const String text = _getBlobText(nvrtcDiagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
        SLANG_CHECK_ABORT(nvrtcCode != nullptr);

        PTXEntrySummary nvvmSummary;
        PTXEntrySummary nvrtcSummary;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(nvvmCode).getUnownedSlice(),
            toSlice("computeMain"),
            nvvmSummary)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(nvrtcCode).getUnownedSlice(),
            toSlice("computeMain"),
            nvrtcSummary)));
        SLANG_CHECK(_hasPTXParameterWidths(
            nvvmSummary,
            scalarCase.parameterWidths,
            scalarCase.parameterCount));
        SLANG_CHECK(_hasPTXParameterWidths(
            nvrtcSummary,
            scalarCase.parameterWidths,
            scalarCase.parameterCount));
        SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
        SLANG_CHECK(nvvmSummary.hasGlobalStore32);
        SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
        SLANG_CHECK(nvvmSummary.hasGlobalLoad32 == scalarCase.expectsLoad);
        SLANG_CHECK(nvrtcSummary.hasGlobalLoad32 == scalarCase.expectsLoad);
        if (scalarCase.expectsAdd)
        {
            SLANG_CHECK(nvvmSummary.hasAdd32);
            SLANG_CHECK(nvrtcSummary.hasAdd32);
        }
        if (scalarCase.expectsSignedComparison)
        {
            SLANG_CHECK(nvvmSummary.hasSignedComparison32);
            SLANG_CHECK(nvrtcSummary.hasSignedComparison32);
        }
    }
}

static bool _hasNVVMFloat32ArithmeticPTXEvidence(
    const PTXEntrySummary& summary,
    NVVMFloat32ArithmeticTestOperation operation)
{
    switch (operation)
    {
    case NVVMFloat32ArithmeticTestOperation::Add:
        return summary.hasFloatAdd32;
    case NVVMFloat32ArithmeticTestOperation::Subtract:
        return summary.hasFloatSubtract32;
    case NVVMFloat32ArithmeticTestOperation::Multiply:
        return summary.hasFloatMultiply32;
    case NVVMFloat32ArithmeticTestOperation::Divide:
        return summary.hasFloatDivide32;
    case NVVMFloat32ArithmeticTestOperation::Negate:
        return summary.hasFloatNegate32;
    default:
        SLANG_UNEXPECTED("unknown NVVM float32 binary PTX operation");
    }
}

static void _runNVVMSlangRealFloat32ArithmeticDifferentialPTX(
    UnitTestContext* unitTestContext,
    NVVMFloat32ArithmeticTestOperation testOperation)
{
    const NVVMFloat32ArithmeticTestCase& testCase =
        _getNVVMFloat32ArithmeticTestCase(testOperation);
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(testCase.feature));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-arithmetic PTX differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    PTXEntrySummary summaries[2];
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kMethods); ++i)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            testCase.source,
            kMethods[i],
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(code).getUnownedSlice(),
            toSlice("computeMain"),
            summaries[i])));
        static const uint32_t kParameterWidths[] = {64, 32, 32};
        SLANG_CHECK(
            _hasPTXParameterWidths(summaries[i], kParameterWidths, testCase.operandCount + 1));
        SLANG_CHECK(summaries[i].hasGlobalStore32);
        SLANG_CHECK(!summaries[i].hasGlobalLoad32);
        for (const auto& arithmeticCase : kNVVMFloat32ArithmeticTestCases)
        {
            SLANG_CHECK(
                _hasNVVMFloat32ArithmeticPTXEvidence(summaries[i], arithmeticCase.testOperation) ==
                (&arithmeticCase == &testCase));
        }
    }
    SLANG_CHECK(_haveEqualPTXParameterWidths(summaries[0], summaries[1]));
}

#define NVVM_FLOAT32_ARITHMETIC_DIFFERENTIAL_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                          \
    {                                                              \
        _runNVVMSlangRealFloat32ArithmeticDifferentialPTX(         \
            unitTestContext,                                       \
            NVVMFloat32ArithmeticTestOperation::OPERATION);        \
    }

NVVM_FLOAT32_ARITHMETIC_DIFFERENTIAL_TEST(nvvmSlangRealFloat32AddDifferentialPTX, Add)
NVVM_FLOAT32_ARITHMETIC_DIFFERENTIAL_TEST(nvvmSlangRealFloat32SubtractDifferentialPTX, Subtract)
NVVM_FLOAT32_ARITHMETIC_DIFFERENTIAL_TEST(nvvmSlangRealFloat32MultiplyDifferentialPTX, Multiply)
NVVM_FLOAT32_ARITHMETIC_DIFFERENTIAL_TEST(nvvmSlangRealFloat32DivideDifferentialPTX, Divide)
NVVM_FLOAT32_ARITHMETIC_DIFFERENTIAL_TEST(nvvmSlangRealFloat32NegateDifferentialPTX, Negate)

#undef NVVM_FLOAT32_ARITHMETIC_DIFFERENTIAL_TEST

static void _runNVVMSlangRealFloat32ComparisonDifferentialPTX(
    UnitTestContext* unitTestContext,
    NVVMFloat32ComparisonTestOperation testOperation)
{
    const NVVMFloat32ComparisonTestCase& testCase =
        _getNVVMFloat32ComparisonTestCase(testOperation);
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(testCase.feature));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-comparison PTX differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    PTXEntrySummary summaries[2];
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kMethods); ++i)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            testCase.source,
            kMethods[i],
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(code).getUnownedSlice(),
            toSlice("computeMain"),
            summaries[i])));
        static const uint32_t kParameterWidths[] = {64, 32, 32};
        SLANG_CHECK(_hasPTXParameterWidths(
            summaries[i],
            kParameterWidths,
            SLANG_COUNT_OF(kParameterWidths)));
        SLANG_CHECK(summaries[i].hasGlobalStore32);
        SLANG_CHECK(!summaries[i].hasGlobalLoad32);
        SLANG_CHECK(summaries[i].hasFloatComparison32);
        SLANG_CHECK(!summaries[i].hasEqualityComparison32);
        SLANG_CHECK(!summaries[i].hasSignedComparison32);
        SLANG_CHECK(!summaries[i].hasFloatAdd32);
        SLANG_CHECK(!summaries[i].hasFloatSubtract32);
        SLANG_CHECK(!summaries[i].hasFloatMultiply32);
        SLANG_CHECK(!summaries[i].hasFloatDivide32);
        SLANG_CHECK(!summaries[i].hasFloatNegate32);
    }
    SLANG_CHECK(_haveEqualPTXParameterWidths(summaries[0], summaries[1]));
}

#define NVVM_FLOAT32_COMPARISON_DIFFERENTIAL_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                          \
    {                                                              \
        _runNVVMSlangRealFloat32ComparisonDifferentialPTX(         \
            unitTestContext,                                       \
            NVVMFloat32ComparisonTestOperation::OPERATION);        \
    }

NVVM_FLOAT32_COMPARISON_DIFFERENTIAL_TEST(nvvmSlangRealFloat32EqualDifferentialPTX, OrderedEqual)
NVVM_FLOAT32_COMPARISON_DIFFERENTIAL_TEST(
    nvvmSlangRealFloat32NotEqualDifferentialPTX,
    UnorderedNotEqual)
NVVM_FLOAT32_COMPARISON_DIFFERENTIAL_TEST(
    nvvmSlangRealFloat32GreaterThanDifferentialPTX,
    OrderedGreaterThan)
NVVM_FLOAT32_COMPARISON_DIFFERENTIAL_TEST(
    nvvmSlangRealFloat32LessEqualDifferentialPTX,
    OrderedLessEqual)
NVVM_FLOAT32_COMPARISON_DIFFERENTIAL_TEST(
    nvvmSlangRealFloat32GreaterEqualDifferentialPTX,
    OrderedGreaterEqual)
NVVM_FLOAT32_COMPARISON_DIFFERENTIAL_TEST(
    nvvmSlangRealFloat32LessThanDifferentialPTX,
    OrderedLessThan)

#undef NVVM_FLOAT32_COMPARISON_DIFFERENTIAL_TEST

SLANG_UNIT_TEST(nvvmSlangRealFloat32ConstantDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-constant PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    PTXEntrySummary summaries[2];
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kMethods); ++i)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFloat32ConstantSource,
            kMethods[i],
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(code).getUnownedSlice(),
            toSlice("computeMain"),
            summaries[i])));
        static const uint32_t kParameterWidths[] = {64};
        SLANG_CHECK(_hasPTXParameterWidths(
            summaries[i],
            kParameterWidths,
            SLANG_COUNT_OF(kParameterWidths)));
        SLANG_CHECK(summaries[i].hasGlobalStore32);
        SLANG_CHECK(!summaries[i].hasGlobalLoad32);
        SLANG_CHECK(!summaries[i].hasFloatAdd32);
        SLANG_CHECK(!summaries[i].hasFloatSubtract32);
        SLANG_CHECK(!summaries[i].hasFloatMultiply32);
        SLANG_CHECK(!summaries[i].hasFloatDivide32);
        SLANG_CHECK(!summaries[i].hasFloatNegate32);
        SLANG_CHECK(!summaries[i].hasFloatComparison32);
    }
    SLANG_CHECK(_haveEqualPTXParameterWidths(summaries[0], summaries[1]));
}

SLANG_UNIT_TEST(nvvmSlangRealFloat32PhiDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-phi PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    PTXEntrySummary summaries[2];
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kMethods); ++i)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFloat32PhiSource,
            kMethods[i],
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(code).getUnownedSlice(),
            toSlice("computeMain"),
            summaries[i])));
        static const uint32_t kParameterWidths[] = {64, 32, 32, 32};
        SLANG_CHECK(_hasPTXParameterWidths(
            summaries[i],
            kParameterWidths,
            SLANG_COUNT_OF(kParameterWidths)));
        SLANG_CHECK(summaries[i].hasGlobalStore32);
        SLANG_CHECK(!summaries[i].hasGlobalLoad32);
        SLANG_CHECK(!summaries[i].hasFloatAdd32);
        SLANG_CHECK(!summaries[i].hasFloatSubtract32);
        SLANG_CHECK(!summaries[i].hasFloatMultiply32);
        SLANG_CHECK(!summaries[i].hasFloatDivide32);
        SLANG_CHECK(!summaries[i].hasFloatNegate32);
        SLANG_CHECK(!summaries[i].hasFloatComparison32);
    }
    SLANG_CHECK(_haveEqualPTXParameterWidths(summaries[0], summaries[1]));
}

SLANG_UNIT_TEST(nvvmSlangRealFloat32FunctionDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-function PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    PTXEntrySummary summaries[2];
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kMethods); ++i)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFloat32FunctionSource,
            kMethods[i],
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(code).getUnownedSlice(),
            toSlice("computeMain"),
            summaries[i])));
        static const uint32_t kParameterWidths[] = {64, 32, 32};
        SLANG_CHECK(_hasPTXParameterWidths(
            summaries[i],
            kParameterWidths,
            SLANG_COUNT_OF(kParameterWidths)));
        SLANG_CHECK(summaries[i].hasGlobalStore32);
        SLANG_CHECK(!summaries[i].hasGlobalLoad32);
        SLANG_CHECK(summaries[i].hasFloatAdd32);
        SLANG_CHECK(!summaries[i].hasFloatSubtract32);
        SLANG_CHECK(!summaries[i].hasFloatMultiply32);
        SLANG_CHECK(!summaries[i].hasFloatDivide32);
        SLANG_CHECK(!summaries[i].hasFloatNegate32);
        SLANG_CHECK(!summaries[i].hasFloatComparison32);
    }
    SLANG_CHECK(_haveEqualPTXParameterWidths(summaries[0], summaries[1]));
}

template<typename TCheckPTX>
static void _runNVVMSlangDifferentialPTX(
    const char* source,
    const char* unavailableMessage,
    const uint32_t* parameterWidths,
    Index parameterWidthCount,
    bool expectGlobalLoad,
    TCheckPTX checkPTX)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(TestMessageType::Info, unavailableMessage);
        SLANG_IGNORE_TEST;
    }

    PTXEntrySummary summaries[2];
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kMethods); ++i)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult =
            _compileSlangWithPTXMethod(globalSession, source, kMethods[i], code, diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        const String ptx = _getBlobText(code);
        checkPTX(kMethods[i], ptx);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            _summarizePTXEntry(ptx.getUnownedSlice(), toSlice("computeMain"), summaries[i])));
        SLANG_CHECK(_hasPTXParameterWidths(summaries[i], parameterWidths, parameterWidthCount));
        SLANG_CHECK(summaries[i].hasGlobalStore32);
        SLANG_CHECK(summaries[i].hasGlobalLoad32 == expectGlobalLoad);
    }
    SLANG_CHECK(_haveEqualPTXParameterWidths(summaries[0], summaries[1]));
}

SLANG_UNIT_TEST(nvvmSlangCUDAExecutionDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsVectorConstruction());

    static const uint32_t kParameterWidths[] = {64, 64};
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMCUDAExecutionRuntimeSource,
        "Ignoring CUDA-execution PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        false,
        [](SlangEmitCUDAMethod, const String& ptx)
        {
            static const char* kExpectedPTX[] = {
                "%tid.x",
                "%tid.y",
                "%tid.z",
                "%ctaid.x",
                "%ctaid.y",
                "%ctaid.z",
                "%ntid.x",
                "%ntid.y",
                "%ntid.z",
                "%nctaid.x",
                "%nctaid.y",
                "%nctaid.z",
                "bar.sync",
            };
            for (const char* expected : kExpectedPTX)
                SLANG_CHECK(ptx.indexOf(expected) >= 0);
        });
}

SLANG_UNIT_TEST(nvvmSlangSharedMemoryDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsGlobalStorage());

    static const uint32_t kParameterWidths[] = {64, 64};
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMSharedMemorySource,
        "Ignoring shared-memory PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        false,
        [](SlangEmitCUDAMethod, const String& ptx)
        {
            SLANG_CHECK(ptx.indexOf(".shared .align 4") >= 0);
            SLANG_CHECK(ptx.indexOf("[256]") >= 0);
            SLANG_CHECK(ptx.indexOf("st.shared") >= 0);
            SLANG_CHECK(ptx.indexOf("ld.shared") >= 0);
            SLANG_CHECK(ptx.indexOf("bar.sync") >= 0);
        });
}

SLANG_UNIT_TEST(nvvmSlangMixedNumericDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsVectorConstruction());

    static const uint32_t kParameterWidths[] = {
        64,
        64,
        64,
        64,
        64,
        64,
        64,
        64,
        8,
        8,
        16,
        16,
        64,
        64,
        32,
    };
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMMixedNumericSource,
        "Ignoring mixed-numeric PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        true,
        [](SlangEmitCUDAMethod, const String& ptx)
        {
            SLANG_CHECK(ptx.indexOf(".u8") >= 0);
            SLANG_CHECK(ptx.indexOf(".u16") >= 0);
            SLANG_CHECK(ptx.indexOf(".u64") >= 0);
            SLANG_CHECK(ptx.indexOf("cvt") >= 0);
        });
}

SLANG_UNIT_TEST(nvvmSlangRealWaveLaneIndexDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));

    static const uint32_t kParameterWidths[] = {64};
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMWaveLaneIndexSource,
        "Ignoring wave-lane-index PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        false,
        [](SlangEmitCUDAMethod method, const String& ptx)
        {
            if (method == SLANG_EMIT_CUDA_VIA_NVVM)
            {
                SLANG_CHECK(ptx.indexOf("%laneid") >= 0);
            }
            else
            {
                SLANG_CHECK(ptx.indexOf("%tid.x") >= 0);
                SLANG_CHECK(ptx.indexOf("and.b32") >= 0);
                SLANG_CHECK(ptx.indexOf(", 31") >= 0);
            }
        });
}

SLANG_UNIT_TEST(nvvmSlangRealWaveLaneCountDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT));

    static const uint32_t kParameterWidths[] = {64};
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMWaveLaneCountSource,
        "Ignoring wave-lane-count PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        false,
        [](SlangEmitCUDAMethod method, const String& ptx)
        {
            SLANG_CHECK(ptx.indexOf("%tid.x") >= 0 || ptx.indexOf("%laneid") >= 0);
            if (method == SLANG_EMIT_CUDA_VIA_NVVM)
            {
                SLANG_CHECK(ptx.indexOf("%laneid") >= 0);
                SLANG_CHECK(ptx.indexOf("WARP_SZ") >= 0);
            }
        });
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneAtUIntDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT));

    static const uint32_t kParameterWidths[] = {64, 32, 32};
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMWaveReadLaneAtUIntSource,
        "Ignoring UInt wave-read-lane-at PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        false,
        [](SlangEmitCUDAMethod method, const String& ptx)
        {
            SLANG_CHECK(ptx.indexOf("shfl.sync.idx.b32") >= 0);
            if (method == SLANG_EMIT_CUDA_VIA_NVVM)
            {
                SLANG_CHECK(ptx.indexOf("%laneid") >= 0);
            }
            else
            {
                SLANG_CHECK(ptx.indexOf("%tid.x") >= 0);
            }
        });
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneAtIntDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT));

    static const uint32_t kParameterWidths[] = {64, 64, 32, 32};
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMWaveReadLaneAtIntSource,
        "Ignoring Int wave-read-lane-at PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        true,
        [](SlangEmitCUDAMethod method, const String& ptx)
        {
            SLANG_CHECK(ptx.indexOf("shfl.sync.idx.b32") >= 0);
            if (method == SLANG_EMIT_CUDA_VIA_NVVM)
            {
                SLANG_CHECK(ptx.indexOf("%laneid") >= 0);
            }
            else
            {
                SLANG_CHECK(ptx.indexOf("%tid.x") >= 0);
            }
        });
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneAtFloatDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT));

    static const uint32_t kParameterWidths[] = {64, 64, 32, 32};
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMWaveReadLaneAtFloatSource,
        "Ignoring Float wave-read-lane-at PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        true,
        [](SlangEmitCUDAMethod method, const String& ptx)
        {
            SLANG_CHECK(ptx.indexOf("shfl.sync.idx.b32") >= 0);
            if (method == SLANG_EMIT_CUDA_VIA_NVVM)
            {
                SLANG_CHECK(ptx.indexOf("%laneid") >= 0);
            }
            else
            {
                SLANG_CHECK(ptx.indexOf("%tid.x") >= 0);
            }
        });
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveMaskDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT));

    static const uint32_t kParameterWidths[] = {64};
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMWaveActiveMaskSource,
        "Ignoring active-mask PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        false,
        [](SlangEmitCUDAMethod method, const String& ptx)
        {
            SLANG_CHECK(ptx.indexOf("vote.sync.ballot.b32") >= 0);
            if (method == SLANG_EMIT_CUDA_VIA_NVVM)
            {
                SLANG_CHECK(ptx.indexOf("%laneid") >= 0);
            }
            else
            {
                SLANG_CHECK(ptx.indexOf("%tid.x") >= 0);
            }
        });
}

// Checks the common ballot-plus-shuffle mechanism for one public scalar wave-read fixture.
static void _checkUnmaskedWaveReadLaneAtDifferentialPTX(
    UnitTestContext* context,
    const char* source,
    SlangNVVMBuilderFeature shuffleFeature,
    const char* unavailableMessage,
    const uint32_t* parameterWidths,
    Index parameterWidthCount,
    bool expectGlobalLoad)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(context, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(shuffleFeature));
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT));

    _runNVVMSlangDifferentialPTX(
        source,
        unavailableMessage,
        parameterWidths,
        parameterWidthCount,
        expectGlobalLoad,
        [](SlangEmitCUDAMethod method, const String& ptx)
        {
            String signature;
            String body;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                _extractPTXEntry(ptx.getUnownedSlice(), toSlice("computeMain"), signature, body)));
            SLANG_CHECK(
                _countOccurrences(body.getUnownedSlice(), toSlice("vote.sync.ballot.b32")) == 1);
            SLANG_CHECK(
                _countOccurrences(body.getUnownedSlice(), toSlice("shfl.sync.idx.b32")) == 1);
            if (method == SLANG_EMIT_CUDA_VIA_NVVM)
            {
                SLANG_CHECK(ptx.indexOf("%laneid") >= 0);
            }
            else
            {
                SLANG_CHECK(ptx.indexOf("%tid.x") >= 0);
            }
        });
}

SLANG_UNIT_TEST(nvvmSlangRealUnmaskedWaveReadLaneAtUIntDifferentialPTX)
{
    static const uint32_t kParameterWidths[] = {64, 32};
    _checkUnmaskedWaveReadLaneAtDifferentialPTX(
        unitTestContext,
        kDirectNVVMUnmaskedWaveReadLaneAtUIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT,
        "Ignoring unmasked UInt wave-read-lane-at PTX differential because libNVVM or NVRTC was "
        "not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        false);
}

SLANG_UNIT_TEST(nvvmSlangRealUnmaskedWaveReadLaneAtIntDifferentialPTX)
{
    static const uint32_t kParameterWidths[] = {64, 64, 32};
    _checkUnmaskedWaveReadLaneAtDifferentialPTX(
        unitTestContext,
        kDirectNVVMUnmaskedWaveReadLaneAtIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT,
        "Ignoring unmasked Int wave-read-lane-at PTX differential because libNVVM or NVRTC was "
        "not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        true);
}

SLANG_UNIT_TEST(nvvmSlangRealUnmaskedWaveReadLaneAtFloatDifferentialPTX)
{
    static const uint32_t kParameterWidths[] = {64, 64, 32};
    _checkUnmaskedWaveReadLaneAtDifferentialPTX(
        unitTestContext,
        kDirectNVVMUnmaskedWaveReadLaneAtFloatSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT,
        "Ignoring unmasked Float wave-read-lane-at PTX differential because libNVVM or NVRTC was "
        "not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        true);
}

SLANG_UNIT_TEST(nvvmSlangRealFloat32CopyDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-copy PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    PTXEntrySummary summaries[2];
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kMethods); ++i)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFloat32CopySource,
            kMethods[i],
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(code).getUnownedSlice(),
            toSlice("computeMain"),
            summaries[i])));
        static const uint32_t kParameterWidths[] = {64, 64};
        SLANG_CHECK(_hasPTXParameterWidths(
            summaries[i],
            kParameterWidths,
            SLANG_COUNT_OF(kParameterWidths)));
        SLANG_CHECK(summaries[i].hasGlobalLoad32);
        SLANG_CHECK(summaries[i].hasGlobalStore32);
        SLANG_CHECK(!summaries[i].hasFloatAdd32);
    }
    SLANG_CHECK(_haveEqualPTXParameterWidths(summaries[0], summaries[1]));
}

SLANG_UNIT_TEST(nvvmSlangRealPointerOffsetDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarPointerArithmetic());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMPointerOffsetSource,
        SLANG_EMIT_CUDA_VIA_NVVM,
        nvvmCode,
        nvvmDiagnostics);
    if (SLANG_FAILED(nvvmResult))
    {
        const String text = _getBlobText(nvvmDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmCode != nullptr);

    ComPtr<slang::IBlob> nvrtcCode;
    ComPtr<slang::IBlob> nvrtcDiagnostics;
    const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMPointerOffsetSource,
        SLANG_EMIT_CUDA_VIA_NVRTC,
        nvrtcCode,
        nvrtcDiagnostics);
    if (SLANG_FAILED(nvrtcResult))
    {
        const String text = _getBlobText(nvrtcDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcCode != nullptr);

    PTXEntrySummary nvvmSummary;
    PTXEntrySummary nvrtcSummary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvvmCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvvmSummary)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvrtcCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvrtcSummary)));
    static const uint32_t kParameterWidths[] = {64, 64, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasGlobalLoad32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalLoad32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealFixedDeviceArrayDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarArrayAddressing());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMFixedDeviceArraySource,
        SLANG_EMIT_CUDA_VIA_NVVM,
        nvvmCode,
        nvvmDiagnostics);
    if (SLANG_FAILED(nvvmResult))
    {
        const String text = _getBlobText(nvvmDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmCode != nullptr);

    ComPtr<slang::IBlob> nvrtcCode;
    ComPtr<slang::IBlob> nvrtcDiagnostics;
    const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMFixedDeviceArraySource,
        SLANG_EMIT_CUDA_VIA_NVRTC,
        nvrtcCode,
        nvrtcDiagnostics);
    if (SLANG_FAILED(nvrtcResult))
    {
        const String text = _getBlobText(nvrtcDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcCode != nullptr);

    PTXEntrySummary nvvmSummary;
    PTXEntrySummary nvrtcSummary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvvmCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvvmSummary)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvrtcCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvrtcSummary)));
    static const uint32_t kParameterWidths[] = {64, 64, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasGlobalLoad32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalLoad32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealRawRWStructuredBufferI32DifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsRawRWStructuredBufferI32());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring raw RWStructuredBuffer<int> PTX differential because libNVVM or NVRTC was "
            "not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMRawRWStructuredBufferI32StoreSource,
        SLANG_EMIT_CUDA_VIA_NVVM,
        nvvmCode,
        nvvmDiagnostics);
    if (SLANG_FAILED(nvvmResult))
    {
        const String text = _getBlobText(nvvmDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmCode != nullptr);

    ComPtr<slang::IBlob> nvrtcCode;
    ComPtr<slang::IBlob> nvrtcDiagnostics;
    const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMRawRWStructuredBufferI32StoreSource,
        SLANG_EMIT_CUDA_VIA_NVRTC,
        nvrtcCode,
        nvrtcDiagnostics);
    if (SLANG_FAILED(nvrtcResult))
    {
        const String text = _getBlobText(nvrtcDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcCode != nullptr);

    const String nvvmPTX = _getBlobText(nvvmCode);
    const String nvrtcPTX = _getBlobText(nvrtcCode);
    PTXEntrySummary nvvmSummary;
    PTXEntrySummary nvrtcSummary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvvmPTX.getUnownedSlice(), toSlice("computeMain"), nvvmSummary)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvrtcPTX.getUnownedSlice(), toSlice("computeMain"), nvrtcSummary)));
    static const uint32_t kParameterWidths[] = {8, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(!nvvmSummary.hasGlobalLoad32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvvmSummary.hasMultiply32);
    SLANG_CHECK(!nvrtcSummary.hasGlobalLoad32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasMultiply32);

    String nvvmSignature;
    String nvvmBody;
    String nvrtcSignature;
    String nvrtcBody;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_extractPTXEntry(
        nvvmPTX.getUnownedSlice(),
        toSlice("computeMain"),
        nvvmSignature,
        nvvmBody)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_extractPTXEntry(
        nvrtcPTX.getUnownedSlice(),
        toSlice("computeMain"),
        nvrtcSignature,
        nvrtcBody)));
    SLANG_CHECK(nvvmSignature.indexOf(".param .align 8 .b8") >= 0);
    SLANG_CHECK(nvvmSignature.indexOf("[16]") >= 0);
    SLANG_CHECK(nvrtcSignature.indexOf(".param .align 8 .b8") >= 0);
    SLANG_CHECK(nvrtcSignature.indexOf("[16]") >= 0);
    SLANG_CHECK(_countOccurrences(nvvmSignature.getUnownedSlice(), toSlice(".param")) == 2);
    SLANG_CHECK(_countOccurrences(nvrtcSignature.getUnownedSlice(), toSlice(".param")) == 2);
    SLANG_CHECK(nvvmBody.indexOf("ld.param.u64") >= 0);
    SLANG_CHECK(nvrtcBody.indexOf("ld.param.u64") >= 0);
}

static bool _supportsNVVMScalarTestOperation(
    const NVVMIRBuilder& builder,
    NVVMScalarTestOperation operation)
{
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        return builder.supportsScalarIntegerMultiply();
    case NVVMScalarTestOperation::BitAnd:
        return builder.supportsScalarIntegerBitAnd();
    case NVVMScalarTestOperation::BitOr:
        return builder.supportsScalarIntegerBitOr();
    case NVVMScalarTestOperation::BitXor:
        return builder.supportsScalarIntegerBitXor();
    case NVVMScalarTestOperation::BitNot:
        return builder.supportsScalarIntegerBitNot();
    case NVVMScalarTestOperation::Negate:
        return builder.supportsScalarIntegerNegate();
    case NVVMScalarTestOperation::Equal:
        return builder.supportsScalarIntegerEqual();
    case NVVMScalarTestOperation::NotEqual:
        return builder.supportsScalarIntegerNotEqual();
    case NVVMScalarTestOperation::SignedGreaterThan:
        return builder.supportsScalarIntegerSignedGreaterThan();
    case NVVMScalarTestOperation::SignedLessEqual:
        return builder.supportsScalarIntegerSignedLessEqual();
    case NVVMScalarTestOperation::SignedGreaterEqual:
        return builder.supportsScalarIntegerSignedGreaterEqual();
    }
    return false;
}

static void _checkNVVMScalarPTXEvidence(
    const PTXEntrySummary& summary,
    NVVMScalarPTXEvidence evidence)
{
    switch (evidence)
    {
    case NVVMScalarPTXEvidence::Multiply:
        SLANG_CHECK(summary.hasMultiply32);
        break;
    case NVVMScalarPTXEvidence::BitAnd:
        SLANG_CHECK(summary.hasBitAnd32);
        break;
    case NVVMScalarPTXEvidence::BitOr:
        SLANG_CHECK(summary.hasBitOr32);
        break;
    case NVVMScalarPTXEvidence::BitXor:
        SLANG_CHECK(summary.hasBitXor32);
        SLANG_CHECK(!summary.hasBitOr32);
        break;
    case NVVMScalarPTXEvidence::BitNot:
        SLANG_CHECK(summary.hasBitNot32);
        SLANG_CHECK(!summary.hasBitXor32);
        break;
    case NVVMScalarPTXEvidence::Negate:
        SLANG_CHECK(summary.hasNegate32);
        SLANG_CHECK(!summary.hasBitNot32);
        break;
    case NVVMScalarPTXEvidence::EqualityComparison:
        SLANG_CHECK(summary.hasEqualityComparison32);
        break;
    case NVVMScalarPTXEvidence::SignedComparison:
        SLANG_CHECK(summary.hasSignedComparison32);
        break;
    }
}

// Checks the shared first-active-lane mechanism for one public scalar payload row.
static void _runNVVMSlangWaveReadLaneFirstDifferentialPTX(
    UnitTestContext* unitTestContext,
    const char* source,
    SlangNVVMBuilderFeature readFirstFeature,
    const char* unavailableMessage,
    const uint32_t* parameterWidths,
    Index parameterWidthCount,
    bool expectGlobalLoad)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT));
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(readFirstFeature));

    _runNVVMSlangDifferentialPTX(
        source,
        unavailableMessage,
        parameterWidths,
        parameterWidthCount,
        expectGlobalLoad,
        [](SlangEmitCUDAMethod method, const String& ptx)
        {
            String signature;
            String body;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                _extractPTXEntry(ptx.getUnownedSlice(), toSlice("computeMain"), signature, body)));
            const UnownedStringSlice bodySlice = body.getUnownedSlice();
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("vote.sync.ballot.b32")) == 1);
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("shfl.sync.idx.b32")) == 1);
            if (method == SLANG_EMIT_CUDA_VIA_NVVM)
            {
                SLANG_CHECK(ptx.indexOf("%laneid") >= 0);
                SLANG_CHECK(_countOccurrences(bodySlice, toSlice("popc.b32")) == 1);
            }
            else
            {
                SLANG_CHECK(ptx.indexOf("%tid.x") >= 0);
                SLANG_CHECK(_countOccurrences(bodySlice, toSlice("brev.b32")) == 1);
                SLANG_CHECK(_countOccurrences(bodySlice, toSlice("bfind.shiftamt.u32")) == 1);
            }
        });
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneFirstUIntDifferentialPTX)
{
    static const uint32_t kParameterWidths[] = {64};
    _runNVVMSlangWaveReadLaneFirstDifferentialPTX(
        unitTestContext,
        kDirectNVVMWaveReadLaneFirstUIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_UINT,
        "Ignoring UInt wave-read-lane-first PTX differential because libNVVM or NVRTC was not "
        "found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        false);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneFirstIntDifferentialPTX)
{
    static const uint32_t kParameterWidths[] = {64, 64};
    _runNVVMSlangWaveReadLaneFirstDifferentialPTX(
        unitTestContext,
        kDirectNVVMWaveReadLaneFirstIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_INT,
        "Ignoring Int wave-read-lane-first PTX differential because libNVVM or NVRTC was not "
        "found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        true);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneFirstFloatDifferentialPTX)
{
    static const uint32_t kParameterWidths[] = {64, 64};
    _runNVVMSlangWaveReadLaneFirstDifferentialPTX(
        unitTestContext,
        kDirectNVVMWaveReadLaneFirstFloatSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_FLOAT,
        "Ignoring Float wave-read-lane-first PTX differential because libNVVM or NVRTC was not "
        "found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        true);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveIsFirstLaneDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT));
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_IS_FIRST_LANE));

    static const uint32_t kParameterWidths[] = {64};
    _runNVVMSlangDifferentialPTX(
        kDirectNVVMWaveIsFirstLaneSource,
        "Ignoring wave-is-first-lane PTX differential because libNVVM or NVRTC was not found.",
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        false,
        [](SlangEmitCUDAMethod method, const String& ptx)
        {
            String signature;
            String body;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                _extractPTXEntry(ptx.getUnownedSlice(), toSlice("computeMain"), signature, body)));
            const UnownedStringSlice bodySlice = body.getUnownedSlice();
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("vote.sync.ballot.b32")) == 2);
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("neg.s32")) == 1);
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("and.b32")) >= 1);
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("shl.b32")) >= 1);
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("setp.eq.s32")) == 1);
            if (method == SLANG_EMIT_CUDA_VIA_NVVM)
            {
                SLANG_CHECK(ptx.indexOf("%laneid") >= 0);
            }
            else
            {
                SLANG_CHECK(ptx.indexOf("%tid.x") >= 0);
            }
        });
}

static void _runNVVMSlangWavePredicateDifferentialPTX(
    UnitTestContext* unitTestContext,
    const char* source,
    SlangNVVMBuilderFeature feature,
    const char* unavailableMessage,
    const UnownedStringSlice& intrinsicMnemonic,
    Index expectedSignedNotEqualCount)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX));
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT));
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(feature));

    static const uint32_t kParameterWidths[] = {64, 64};
    _runNVVMSlangDifferentialPTX(
        source,
        unavailableMessage,
        kParameterWidths,
        SLANG_COUNT_OF(kParameterWidths),
        true,
        [intrinsicMnemonic, expectedSignedNotEqualCount](SlangEmitCUDAMethod, const String& ptx)
        {
            String signature;
            String body;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                _extractPTXEntry(ptx.getUnownedSlice(), toSlice("computeMain"), signature, body)));
            const UnownedStringSlice bodySlice = body.getUnownedSlice();
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("vote.sync.ballot.b32")) == 2);
            SLANG_CHECK(_countOccurrences(bodySlice, intrinsicMnemonic) == 1);
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("ld.global.u32")) == 1);
            SLANG_CHECK(
                _countOccurrences(bodySlice, toSlice("setp.ne.s32")) ==
                expectedSignedNotEqualCount);
            SLANG_CHECK(_countOccurrences(bodySlice, toSlice("st.global.u32")) == 1);
        });
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAnyTrueDifferentialPTX)
{
    _runNVVMSlangWavePredicateDifferentialPTX(
        unitTestContext,
        kDirectNVVMWaveActiveAnyTrueSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ANY_TRUE,
        "Ignoring wave-active-any-true PTX differential because libNVVM or NVRTC was not found.",
        toSlice("vote.sync.any.pred"),
        1);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAllTrueDifferentialPTX)
{
    _runNVVMSlangWavePredicateDifferentialPTX(
        unitTestContext,
        kDirectNVVMWaveActiveAllTrueSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_TRUE,
        "Ignoring wave-active-all-true PTX differential because libNVVM or NVRTC was not found.",
        toSlice("vote.sync.all.pred"),
        1);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAllEqualIntDifferentialPTX)
{
    _runNVVMSlangWavePredicateDifferentialPTX(
        unitTestContext,
        kDirectNVVMWaveActiveAllEqualIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_INT,
        "Ignoring signed-i32 wave-active-all-equal PTX differential because libNVVM or NVRTC was "
        "not found.",
        toSlice("match.all.sync.b32"),
        0);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAllEqualUIntDifferentialPTX)
{
    _runNVVMSlangWavePredicateDifferentialPTX(
        unitTestContext,
        kDirectNVVMWaveActiveAllEqualUIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_UINT,
        "Ignoring unsigned-i32 wave-active-all-equal PTX differential because libNVVM or NVRTC "
        "was not found.",
        toSlice("match.all.sync.b32"),
        0);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAllEqualFloatDifferentialPTX)
{
    _runNVVMSlangWavePredicateDifferentialPTX(
        unitTestContext,
        kDirectNVVMWaveActiveAllEqualFloatSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_FLOAT,
        "Ignoring float32 wave-active-all-equal PTX differential because libNVVM or NVRTC was "
        "not found.",
        toSlice("match.all.sync.b32"),
        0);
}

static void _runNVVMScalarDifferentialPTX(
    UnitTestContext* unitTestContext,
    NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(_supportsNVVMScalarTestOperation(preflightBuilder, operation));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        StringBuilder message;
        message << "Ignoring " << testCase.diagnosticName
                << " PTX differential because libNVVM or NVRTC was not found.";
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> code[2];
    const SlangEmitCUDAMethod methods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (Index i = 0; i < SLANG_COUNT_OF(methods); ++i)
    {
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithPTXMethod(
            globalSession,
            testCase.source,
            methods[i],
            code[i],
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code[i] != nullptr);
    }

    PTXEntrySummary summaries[2];
    for (Index i = 0; i < SLANG_COUNT_OF(summaries); ++i)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(code[i]).getUnownedSlice(),
            toSlice("computeMain"),
            summaries[i])));
    }
    static const uint32_t kUnaryParameterWidths[] = {64, 32};
    static const uint32_t kBinaryParameterWidths[] = {64, 32, 32};
    const bool isUnary = testCase.key.family == FakeNVVMBuilderScalarFamily::Unary;
    const uint32_t* parameterWidths = isUnary ? kUnaryParameterWidths : kBinaryParameterWidths;
    const Index parameterCount =
        isUnary ? SLANG_COUNT_OF(kUnaryParameterWidths) : SLANG_COUNT_OF(kBinaryParameterWidths);
    for (const PTXEntrySummary& summary : summaries)
    {
        SLANG_CHECK(_hasPTXParameterWidths(summary, parameterWidths, parameterCount));
        _checkNVVMScalarPTXEvidence(summary, testCase.ptxEvidence);
        SLANG_CHECK(summary.hasGlobalStore32);
    }
    SLANG_CHECK(_haveEqualPTXParameterWidths(summaries[0], summaries[1]));
}

#define NVVM_SCALAR_DIFFERENTIAL_TEST(NAME, OPERATION)                                      \
    SLANG_UNIT_TEST(NAME)                                                                   \
    {                                                                                       \
        _runNVVMScalarDifferentialPTX(unitTestContext, NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_DIFFERENTIAL_TEST(nvvmSlangRealIntegerMultiplyDifferentialPTX, Multiply)
NVVM_SCALAR_DIFFERENTIAL_TEST(nvvmSlangRealIntegerEqualDifferentialPTX, Equal)
NVVM_SCALAR_DIFFERENTIAL_TEST(nvvmSlangRealIntegerNotEqualDifferentialPTX, NotEqual)
NVVM_SCALAR_DIFFERENTIAL_TEST(
    nvvmSlangRealIntegerSignedGreaterThanDifferentialPTX,
    SignedGreaterThan)
NVVM_SCALAR_DIFFERENTIAL_TEST(nvvmSlangRealIntegerSignedLessEqualDifferentialPTX, SignedLessEqual)
NVVM_SCALAR_DIFFERENTIAL_TEST(
    nvvmSlangRealIntegerSignedGreaterEqualDifferentialPTX,
    SignedGreaterEqual)
NVVM_SCALAR_DIFFERENTIAL_TEST(nvvmSlangRealIntegerBitAndDifferentialPTX, BitAnd)
NVVM_SCALAR_DIFFERENTIAL_TEST(nvvmSlangRealIntegerBitOrDifferentialPTX, BitOr)
NVVM_SCALAR_DIFFERENTIAL_TEST(nvvmSlangRealIntegerBitXorDifferentialPTX, BitXor)
NVVM_SCALAR_DIFFERENTIAL_TEST(nvvmSlangRealIntegerBitNotDifferentialPTX, BitNot)
NVVM_SCALAR_DIFFERENTIAL_TEST(nvvmSlangRealIntegerNegateDifferentialPTX, Negate)

#undef NVVM_SCALAR_DIFFERENTIAL_TEST
SLANG_UNIT_TEST(nvvmSlangRealRelaxedGlobalI32AtomicAddDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsRelaxedGlobalI32AtomicAdd());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring relaxed global signed-i32 atomic-add PTX differential because libNVVM or "
            "NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMRelaxedGlobalI32AtomicAddSource,
        SLANG_EMIT_CUDA_VIA_NVVM,
        nvvmCode,
        nvvmDiagnostics);
    if (SLANG_FAILED(nvvmResult))
    {
        const String text = _getBlobText(nvvmDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmCode != nullptr);

    ComPtr<slang::IBlob> nvrtcCode;
    ComPtr<slang::IBlob> nvrtcDiagnostics;
    const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMRelaxedGlobalI32AtomicAddSource,
        SLANG_EMIT_CUDA_VIA_NVRTC,
        nvrtcCode,
        nvrtcDiagnostics);
    if (SLANG_FAILED(nvrtcResult))
    {
        const String text = _getBlobText(nvrtcDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcCode != nullptr);

    PTXEntrySummary nvvmSummary;
    PTXEntrySummary nvrtcSummary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvvmCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvvmSummary)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvrtcCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvrtcSummary)));
    static const uint32_t kParameterWidths[] = {64};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasRelaxedGlobalI32AtomicAdd);
    SLANG_CHECK(nvrtcSummary.hasRelaxedGlobalI32AtomicAdd);
    SLANG_CHECK(!nvvmSummary.hasGlobalLoad32);
    SLANG_CHECK(!nvrtcSummary.hasGlobalLoad32);
    SLANG_CHECK(!nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(!nvrtcSummary.hasGlobalStore32);
    SLANG_CHECK(!nvvmSummary.hasAdd32);
    SLANG_CHECK(!nvrtcSummary.hasAdd32);
}


SLANG_UNIT_TEST(nvvmSlangRealScalarPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarFunctions());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring Slang scalar ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring Slang scalar ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const char* kSources[] = {
        kDirectNVVMWriteScalarSource,
        kDirectNVVMCopyScalarSource,
        kDirectNVVMChooseScalarSource,
        kDirectNVVMIntegerConstantSource,
        kDirectNVVMMergePhiSource,
        kDirectNVVMFiniteLoopSource,
        kDirectNVVMScalarFunctionSource,
    };
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (const char* source : kSources)
    {
        for (SlangEmitCUDAMethod method : kMethods)
        {
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                _compileSlangWithPTXMethod(globalSession, source, method, code, diagnostics)));
            ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
            SLANG_CHECK_ABORT(ptxArtifact != nullptr);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
        }
    }
}

// Numeric V4 families have no synthetic V3 feature bit, so this sentinel requests only the common
// real-provider preflight performed by these integration helpers.
static const SlangNVVMBuilderFeature kNoRequiredLegacyFeature = SLANG_NVVM_BUILDER_FEATURE_COUNT;

static void _runNVVMSlangRealSourcePtxasAccepts(
    UnitTestContext* unitTestContext,
    const char* source,
    SlangNVVMBuilderFeature feature)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    if (feature != kNoRequiredLegacyFeature)
        SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(feature));

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct-source ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct-source ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            _compileSlangWithPTXMethod(globalSession, source, method, code, diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

static void _runNVVMSlangRealFloat32ArithmeticPtxasAccepts(
    UnitTestContext* unitTestContext,
    NVVMFloat32ArithmeticTestOperation testOperation)
{
    const NVVMFloat32ArithmeticTestCase& testCase =
        _getNVVMFloat32ArithmeticTestCase(testOperation);
    _runNVVMSlangRealSourcePtxasAccepts(unitTestContext, testCase.source, testCase.feature);
}

SLANG_UNIT_TEST(nvvmSlangCUDAExecutionPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMCUDAExecutionRuntimeSource,
        SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS);
}

SLANG_UNIT_TEST(nvvmSlangSharedMemoryPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsGlobalStorage());
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMSharedMemorySource,
        SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS);
}

SLANG_UNIT_TEST(nvvmSlangMixedNumericPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMMixedNumericSource,
        kNoRequiredLegacyFeature);
}

#define NVVM_FLOAT32_ARITHMETIC_PTXAS_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                   \
    {                                                       \
        _runNVVMSlangRealFloat32ArithmeticPtxasAccepts(     \
            unitTestContext,                                \
            NVVMFloat32ArithmeticTestOperation::OPERATION); \
    }

NVVM_FLOAT32_ARITHMETIC_PTXAS_TEST(nvvmSlangRealFloat32AddPtxasAccepts, Add)
NVVM_FLOAT32_ARITHMETIC_PTXAS_TEST(nvvmSlangRealFloat32SubtractPtxasAccepts, Subtract)
NVVM_FLOAT32_ARITHMETIC_PTXAS_TEST(nvvmSlangRealFloat32MultiplyPtxasAccepts, Multiply)
NVVM_FLOAT32_ARITHMETIC_PTXAS_TEST(nvvmSlangRealFloat32DividePtxasAccepts, Divide)
NVVM_FLOAT32_ARITHMETIC_PTXAS_TEST(nvvmSlangRealFloat32NegatePtxasAccepts, Negate)

#undef NVVM_FLOAT32_ARITHMETIC_PTXAS_TEST

static void _runNVVMSlangRealFloat32ComparisonPtxasAccepts(
    UnitTestContext* unitTestContext,
    NVVMFloat32ComparisonTestOperation testOperation)
{
    const NVVMFloat32ComparisonTestCase& testCase =
        _getNVVMFloat32ComparisonTestCase(testOperation);
    _runNVVMSlangRealSourcePtxasAccepts(unitTestContext, testCase.source, testCase.feature);
}

#define NVVM_FLOAT32_COMPARISON_PTXAS_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                   \
    {                                                       \
        _runNVVMSlangRealFloat32ComparisonPtxasAccepts(     \
            unitTestContext,                                \
            NVVMFloat32ComparisonTestOperation::OPERATION); \
    }

NVVM_FLOAT32_COMPARISON_PTXAS_TEST(nvvmSlangRealFloat32EqualPtxasAccepts, OrderedEqual)
NVVM_FLOAT32_COMPARISON_PTXAS_TEST(nvvmSlangRealFloat32NotEqualPtxasAccepts, UnorderedNotEqual)
NVVM_FLOAT32_COMPARISON_PTXAS_TEST(nvvmSlangRealFloat32GreaterThanPtxasAccepts, OrderedGreaterThan)
NVVM_FLOAT32_COMPARISON_PTXAS_TEST(nvvmSlangRealFloat32LessEqualPtxasAccepts, OrderedLessEqual)
NVVM_FLOAT32_COMPARISON_PTXAS_TEST(
    nvvmSlangRealFloat32GreaterEqualPtxasAccepts,
    OrderedGreaterEqual)
NVVM_FLOAT32_COMPARISON_PTXAS_TEST(nvvmSlangRealFloat32LessThanPtxasAccepts, OrderedLessThan)

#undef NVVM_FLOAT32_COMPARISON_PTXAS_TEST

SLANG_UNIT_TEST(nvvmSlangRealFloat32ConstantPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMFloat32ConstantSource,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT);
}

SLANG_UNIT_TEST(nvvmSlangRealFloat32PhiPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMFloat32PhiSource,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI);
}

SLANG_UNIT_TEST(nvvmSlangRealFloat32FunctionPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMFloat32FunctionSource,
        SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveLaneIndexPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveLaneIndexSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveLaneCountPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveLaneCountSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneAtUIntPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveReadLaneAtUIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneAtIntPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveReadLaneAtIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneAtFloatPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveReadLaneAtFloatSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveMaskPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveActiveMaskSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneFirstUIntPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveReadLaneFirstUIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_UINT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneFirstIntPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveReadLaneFirstIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_INT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveReadLaneFirstFloatPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveReadLaneFirstFloatSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_FLOAT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveIsFirstLanePtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveIsFirstLaneSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_IS_FIRST_LANE);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAnyTruePtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveActiveAnyTrueSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ANY_TRUE);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAllTruePtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveActiveAllTrueSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_TRUE);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAllEqualIntPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveActiveAllEqualIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_INT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAllEqualUIntPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveActiveAllEqualUIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_UINT);
}

SLANG_UNIT_TEST(nvvmSlangRealWaveActiveAllEqualFloatPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMWaveActiveAllEqualFloatSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_FLOAT);
}

SLANG_UNIT_TEST(nvvmSlangRealUnmaskedWaveReadLaneAtUIntPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMUnmaskedWaveReadLaneAtUIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT);
}

SLANG_UNIT_TEST(nvvmSlangRealUnmaskedWaveReadLaneAtIntPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMUnmaskedWaveReadLaneAtIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT);
}

SLANG_UNIT_TEST(nvvmSlangRealUnmaskedWaveReadLaneAtFloatPtxasAccepts)
{
    _runNVVMSlangRealSourcePtxasAccepts(
        unitTestContext,
        kDirectNVVMUnmaskedWaveReadLaneAtFloatSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT);
}

SLANG_UNIT_TEST(nvvmSlangRealFloat32CopyPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD));

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-copy ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-copy ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFloat32CopySource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealPointerOffsetPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarPointerArithmetic());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMPointerOffsetSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealFixedDeviceArrayPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarArrayAddressing());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFixedDeviceArraySource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealRawRWStructuredBufferI32PtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsRawRWStructuredBufferI32());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring raw RWStructuredBuffer<int> ptxas test because CUDA_PATH does not contain "
            "ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring raw RWStructuredBuffer<int> ptxas test because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMRawRWStructuredBufferI32StoreSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

static void _runNVVMScalarPtxas(UnitTestContext* unitTestContext, NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(_supportsNVVMScalarTestOperation(preflightBuilder, operation));

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        StringBuilder message;
        message << "Ignoring " << testCase.diagnosticName
                << " ptxas test because CUDA_PATH does not contain ptxas.";
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        StringBuilder message;
        message << "Ignoring " << testCase.diagnosticName
                << " ptxas test because libNVVM or NVRTC was not found.";
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            _compileSlangWithPTXMethod(globalSession, testCase.source, method, code, diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

#define NVVM_SCALAR_PTXAS_TEST(NAME, OPERATION)                                   \
    SLANG_UNIT_TEST(NAME)                                                         \
    {                                                                             \
        _runNVVMScalarPtxas(unitTestContext, NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerMultiplyPtxasAccepts, Multiply)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerBitAndPtxasAccepts, BitAnd)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerBitOrPtxasAccepts, BitOr)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerBitXorPtxasAccepts, BitXor)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerBitNotPtxasAccepts, BitNot)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerNegatePtxasAccepts, Negate)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerEqualPtxasAccepts, Equal)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerNotEqualPtxasAccepts, NotEqual)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerSignedGreaterThanPtxasAccepts, SignedGreaterThan)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerSignedLessEqualPtxasAccepts, SignedLessEqual)
NVVM_SCALAR_PTXAS_TEST(nvvmSlangRealIntegerSignedGreaterEqualPtxasAccepts, SignedGreaterEqual)

#undef NVVM_SCALAR_PTXAS_TEST
SLANG_UNIT_TEST(nvvmSlangRealRelaxedGlobalI32AtomicAddPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsRelaxedGlobalI32AtomicAdd());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring relaxed global signed-i32 atomic-add ptxas test because CUDA_PATH does not "
            "contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring relaxed global signed-i32 atomic-add ptxas test because libNVVM or NVRTC "
            "was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMRelaxedGlobalI32AtomicAddSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}


SLANG_UNIT_TEST(nvvmSlangScalarRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarFunctions());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar runtime differential because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar runtime differential because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    int computeMinor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMinor,
            kCudaDeviceAttributeComputeCapabilityMinor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar runtime differential because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar runtime differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    struct RuntimeCase
    {
        const char* source;
        ScalarRuntimeOperation operation;
        int x;
        int y;
        int expected;
    };
    static const RuntimeCase kCases[] = {
        {kDirectNVVMWriteScalarSource, ScalarRuntimeOperation::Write, 37, 0, 37},
        {kDirectNVVMCopyScalarSource, ScalarRuntimeOperation::Copy, -17, 0, -17},
        {kDirectNVVMChooseScalarSource, ScalarRuntimeOperation::Choose, 2, 5, 7},
        {kDirectNVVMChooseScalarSource, ScalarRuntimeOperation::Choose, 7, 3, 4},
        {kDirectNVVMChooseScalarSource, ScalarRuntimeOperation::Choose, 5, 5, 0},
        {kDirectNVVMChooseScalarSource, ScalarRuntimeOperation::Choose, -2, 1, -1},
        {kDirectNVVMIntegerConstantSource, ScalarRuntimeOperation::Write, 41, 0, 42},
        {kDirectNVVMIntegerConstantSource, ScalarRuntimeOperation::Write, -2, 0, -1},
        {kDirectNVVMMergePhiSource, ScalarRuntimeOperation::Choose, 2, 5, 2},
        {kDirectNVVMMergePhiSource, ScalarRuntimeOperation::Choose, 7, 3, 3},
        {kDirectNVVMMergePhiSource, ScalarRuntimeOperation::Choose, 5, 5, 5},
        {kDirectNVVMMergePhiSource, ScalarRuntimeOperation::Choose, -2, 1, -2},
        {kDirectNVVMFiniteLoopSource, ScalarRuntimeOperation::Write, 0, 0, 0},
        {kDirectNVVMFiniteLoopSource, ScalarRuntimeOperation::Write, 5, 0, 10},
        {kDirectNVVMFiniteLoopSource, ScalarRuntimeOperation::Write, 7, 0, 21},
        {kDirectNVVMScalarFunctionSource, ScalarRuntimeOperation::Write, 5, 0, 13},
        {kDirectNVVMScalarFunctionSource, ScalarRuntimeOperation::Write, -2, 0, -1},
    };
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (const auto& runtimeCase : kCases)
    {
        for (SlangEmitCUDAMethod method : kMethods)
        {
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult compileResult = _compileSlangWithPTXMethod(
                globalSession,
                runtimeCase.source,
                method,
                code,
                diagnostics);
            if (SLANG_FAILED(compileResult))
            {
                const String text = _getBlobText(diagnostics);
                if (text.getLength())
                    getTestReporter()->message(TestMessageType::Info, text.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                runtimeCase.operation,
                runtimeCase.x,
                runtimeCase.y,
                runtimeCase.expected)));
        }
    }
}

template<typename TExecute>
static void _runNVVMSlangSourceRuntimeMatchesNVRTC(
    UnitTestContext* unitTestContext,
    SlangNVVMBuilderFeature feature,
    const char* source,
    TExecute execute)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    if (feature != kNoRequiredLegacyFeature)
        SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(feature));

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct-source runtime test because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct-source runtime test because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct-source runtime test because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct-source runtime test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult =
            _compileSlangWithPTXMethod(globalSession, source, method, code, diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(execute(cuda, code)));
    }
}

static void _runNVVMSlangFloat32ArithmeticRuntimeMatchesNVRTC(
    UnitTestContext* unitTestContext,
    NVVMFloat32ArithmeticTestOperation testOperation)
{
    const NVVMFloat32ArithmeticTestCase& testCase =
        _getNVVMFloat32ArithmeticTestCase(testOperation);
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(testCase.feature));

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-arithmetic runtime because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-arithmetic runtime because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-arithmetic runtime because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-arithmetic runtime because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult =
            _compileSlangWithPTXMethod(globalSession, testCase.source, method, code, diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        for (Index i = 0; i < testCase.runtimeCaseCount; ++i)
        {
            const NVVMFloat32ArithmeticRuntimeCase& runtimeCase = testCase.runtimeCases[i];
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runFloat32ArithmeticKernel(
                cuda,
                code,
                testCase.operandCount,
                runtimeCase.left,
                runtimeCase.right,
                runtimeCase.expected)));
        }
    }
}

#define NVVM_FLOAT32_ARITHMETIC_RUNTIME_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                     \
    {                                                         \
        _runNVVMSlangFloat32ArithmeticRuntimeMatchesNVRTC(    \
            unitTestContext,                                  \
            NVVMFloat32ArithmeticTestOperation::OPERATION);   \
    }

NVVM_FLOAT32_ARITHMETIC_RUNTIME_TEST(nvvmSlangFloat32AddRuntimeMatchesNVRTC, Add)
NVVM_FLOAT32_ARITHMETIC_RUNTIME_TEST(nvvmSlangFloat32SubtractRuntimeMatchesNVRTC, Subtract)
NVVM_FLOAT32_ARITHMETIC_RUNTIME_TEST(nvvmSlangFloat32MultiplyRuntimeMatchesNVRTC, Multiply)
NVVM_FLOAT32_ARITHMETIC_RUNTIME_TEST(nvvmSlangFloat32DivideRuntimeMatchesNVRTC, Divide)
NVVM_FLOAT32_ARITHMETIC_RUNTIME_TEST(nvvmSlangFloat32NegateRuntimeMatchesNVRTC, Negate)

#undef NVVM_FLOAT32_ARITHMETIC_RUNTIME_TEST

static void _runNVVMSlangFloat32ComparisonRuntimeMatchesNVRTC(
    UnitTestContext* unitTestContext,
    NVVMFloat32ComparisonTestOperation testOperation)
{
    const NVVMFloat32ComparisonTestCase& testCase =
        _getNVVMFloat32ComparisonTestCase(testOperation);
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsFeature(testCase.feature));

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-comparison runtime because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-comparison runtime because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-comparison runtime because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-comparison runtime because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult =
            _compileSlangWithPTXMethod(globalSession, testCase.source, method, code, diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        for (Index i = 0; i < testCase.runtimeCaseCount; ++i)
        {
            const NVVMFloat32ComparisonRuntimeCase& runtimeCase = testCase.runtimeCases[i];
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runFloat32ComparisonKernel(
                cuda,
                code,
                runtimeCase.left,
                runtimeCase.right,
                runtimeCase.expected)));
        }
    }
}

#define NVVM_FLOAT32_COMPARISON_RUNTIME_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                     \
    {                                                         \
        _runNVVMSlangFloat32ComparisonRuntimeMatchesNVRTC(    \
            unitTestContext,                                  \
            NVVMFloat32ComparisonTestOperation::OPERATION);   \
    }

NVVM_FLOAT32_COMPARISON_RUNTIME_TEST(nvvmSlangFloat32EqualRuntimeMatchesNVRTC, OrderedEqual)
NVVM_FLOAT32_COMPARISON_RUNTIME_TEST(nvvmSlangFloat32NotEqualRuntimeMatchesNVRTC, UnorderedNotEqual)
NVVM_FLOAT32_COMPARISON_RUNTIME_TEST(
    nvvmSlangFloat32GreaterThanRuntimeMatchesNVRTC,
    OrderedGreaterThan)
NVVM_FLOAT32_COMPARISON_RUNTIME_TEST(nvvmSlangFloat32LessEqualRuntimeMatchesNVRTC, OrderedLessEqual)
NVVM_FLOAT32_COMPARISON_RUNTIME_TEST(
    nvvmSlangFloat32GreaterEqualRuntimeMatchesNVRTC,
    OrderedGreaterEqual)
NVVM_FLOAT32_COMPARISON_RUNTIME_TEST(nvvmSlangFloat32LessThanRuntimeMatchesNVRTC, OrderedLessThan)

#undef NVVM_FLOAT32_COMPARISON_RUNTIME_TEST

SLANG_UNIT_TEST(nvvmSlangFloat32ConstantRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT));

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-constant runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-constant runtime differential because no CUDA device is "
            "available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-constant runtime differential because the device is older than "
            "sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-constant runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFloat32ConstantSource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runFloat32ConstantKernel(cuda, code, 1.5f)));
    }
}

SLANG_UNIT_TEST(nvvmSlangFloat32PhiRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI,
        kDirectNVVMFloat32PhiSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        {
            SLANG_RETURN_ON_FAIL(_runFloat32PhiKernel(cuda, code, 1, 1.5f, -2.25f, 1.5f));
            SLANG_RETURN_ON_FAIL(_runFloat32PhiKernel(cuda, code, 0, 1.5f, -2.25f, -2.25f));
            SLANG_RETURN_ON_FAIL(_runFloat32PhiKernel(cuda, code, -7, -0.0f, 0.0f, -0.0f));
            return _runFloat32PhiKernel(cuda, code, 0, -0.0f, 0.0f, 0.0f);
        });
}

SLANG_UNIT_TEST(nvvmSlangFloat32FunctionRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS,
        kDirectNVVMFloat32FunctionSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        {
            SLANG_RETURN_ON_FAIL(_runFloat32ArithmeticKernel(cuda, code, 2, 1.5f, 2.25f, 3.75f));
            SLANG_RETURN_ON_FAIL(_runFloat32ArithmeticKernel(cuda, code, 2, -8.0f, 0.5f, -7.5f));
            SLANG_RETURN_ON_FAIL(_runFloat32ArithmeticKernel(cuda, code, 2, -0.0f, -0.0f, -0.0f));
            return _runFloat32ArithmeticKernel(cuda, code, 2, 0.0f, -0.0f, 0.0f);
        });
}

SLANG_UNIT_TEST(nvvmSlangCUDAExecutionRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS,
        kDirectNVVMCUDAExecutionRuntimeSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runCUDAExecutionKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangSharedMemoryRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsGlobalStorage());
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS,
        kDirectNVVMSharedMemorySource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runSharedMemoryKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangMixedNumericRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        kNoRequiredLegacyFeature,
        kDirectNVVMMixedNumericSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runMixedNumericKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveLaneIndexRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX,
        kDirectNVVMWaveLaneIndexSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveLaneIndexKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveLaneCountRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT,
        kDirectNVVMWaveLaneCountSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveLaneCountKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneAtUIntRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT,
        kDirectNVVMWaveReadLaneAtUIntSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        {
            SLANG_RETURN_ON_FAIL(_runWaveReadLaneAtUIntKernel(cuda, code, 0));
            return _runWaveReadLaneAtUIntKernel(cuda, code, 7);
        });
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneAtIntRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT,
        kDirectNVVMWaveReadLaneAtIntSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        {
            SLANG_RETURN_ON_FAIL(_runWaveReadLaneAtIntKernel(cuda, code, 0));
            return _runWaveReadLaneAtIntKernel(cuda, code, 7);
        });
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneAtFloatRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT,
        kDirectNVVMWaveReadLaneAtFloatSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        {
            SLANG_RETURN_ON_FAIL(_runWaveReadLaneAtFloatKernel(cuda, code, 0));
            return _runWaveReadLaneAtFloatKernel(cuda, code, 7);
        });
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveMaskRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT,
        kDirectNVVMWaveActiveMaskSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveActiveMaskKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneFirstUIntRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_UINT,
        kDirectNVVMWaveReadLaneFirstUIntSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveReadLaneFirstUIntKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneFirstIntRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_INT,
        kDirectNVVMWaveReadLaneFirstIntSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveReadLaneFirstIntKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneFirstFloatRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_FLOAT,
        kDirectNVVMWaveReadLaneFirstFloatSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveReadLaneFirstFloatKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveIsFirstLaneRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_IS_FIRST_LANE,
        kDirectNVVMWaveIsFirstLaneSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveIsFirstLaneKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAnyTrueRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ANY_TRUE,
        kDirectNVVMWaveActiveAnyTrueSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveActiveAnyTrueKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAllTrueRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_TRUE,
        kDirectNVVMWaveActiveAllTrueSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveActiveAllTrueKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAllEqualIntRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_INT,
        kDirectNVVMWaveActiveAllEqualIntSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveActiveAllEqualIntKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAllEqualUIntRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_UINT,
        kDirectNVVMWaveActiveAllEqualUIntSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveActiveAllEqualUIntKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAllEqualFloatRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_FLOAT,
        kDirectNVVMWaveActiveAllEqualFloatSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        { return _runWaveActiveAllEqualFloatKernel(cuda, code); });
}

SLANG_UNIT_TEST(nvvmSlangUnmaskedWaveReadLaneAtUIntRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT,
        kDirectNVVMUnmaskedWaveReadLaneAtUIntSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        {
            SLANG_RETURN_ON_FAIL(_runUnmaskedWaveReadLaneAtUIntKernel(cuda, code, 0));
            return _runUnmaskedWaveReadLaneAtUIntKernel(cuda, code, 7);
        });
}

SLANG_UNIT_TEST(nvvmSlangUnmaskedWaveReadLaneAtIntRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT,
        kDirectNVVMUnmaskedWaveReadLaneAtIntSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        {
            SLANG_RETURN_ON_FAIL(_runUnmaskedWaveReadLaneAtIntKernel(cuda, code, 0));
            return _runUnmaskedWaveReadLaneAtIntKernel(cuda, code, 7);
        });
}

SLANG_UNIT_TEST(nvvmSlangUnmaskedWaveReadLaneAtFloatRuntimeMatchesNVRTC)
{
    _runNVVMSlangSourceRuntimeMatchesNVRTC(
        unitTestContext,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT,
        kDirectNVVMUnmaskedWaveReadLaneAtFloatSource,
        [](CudaDriverApi& cuda, ISlangBlob* code) -> SlangResult
        {
            SLANG_RETURN_ON_FAIL(_runUnmaskedWaveReadLaneAtFloatKernel(cuda, code, 0));
            return _runUnmaskedWaveReadLaneAtFloatKernel(cuda, code, 7);
        });
}

SLANG_UNIT_TEST(nvvmSlangFloat32CopyRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(
        preflightBuilder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD));

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-copy runtime differential because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-copy runtime differential because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-copy runtime differential because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring float32-copy runtime differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const float kValues[] = {3.75f, -7.5f, 0.0f, 1024.0f};
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFloat32CopySource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        for (float value : kValues)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runFloat32CopyKernel(cuda, code, value)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangPointerOffsetRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarPointerArithmetic());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset runtime differential because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset runtime differential because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset runtime differential because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset runtime differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMPointerOffsetSource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runPointerOffsetKernel(cuda, code, 2, false)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runPointerOffsetKernel(cuda, code, -1, true)));
    }
}

SLANG_UNIT_TEST(nvvmSlangFixedDeviceArrayRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarArrayAddressing());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array runtime differential because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array runtime differential because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array runtime differential because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array runtime differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    static const int kIndices[] = {0, 3};
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFixedDeviceArraySource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        for (int index : kIndices)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runPointerOffsetKernel(cuda, code, index, false)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangRawRWStructuredBufferI32RuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsRawRWStructuredBufferI32());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring raw RWStructuredBuffer<int> runtime differential because the CUDA driver "
            "is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring raw RWStructuredBuffer<int> runtime differential because no CUDA device is "
            "available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring raw RWStructuredBuffer<int> runtime differential because the device is "
            "older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring raw RWStructuredBuffer<int> runtime differential because libNVVM or NVRTC "
            "was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMRawRWStructuredBufferI32StoreSource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runRawRWStructuredBufferI32StoreKernel(cuda, code)));
    }
}

struct NVVMScalarRuntimeCase
{
    int x;
    int y;
    int expected;
};

static void _getNVVMScalarRuntimeCases(
    NVVMScalarTestOperation operation,
    const NVVMScalarRuntimeCase*& outCases,
    Index& outCaseCount)
{
#define RETURN_CASES(CASES)               \
    outCases = CASES;                     \
    outCaseCount = SLANG_COUNT_OF(CASES); \
    return

    static const NVVMScalarRuntimeCase kMultiplyCases[] = {
        {6, 7, 42},
        {-7, 6, -42},
        {0, -19, 0},
    };
    static const NVVMScalarRuntimeCase kEqualCases[] = {
        {0, 0, 1},
        {-7, -7, 1},
        {-7, 7, 0},
        {INT_MIN, INT_MAX, 0},
    };
    static const NVVMScalarRuntimeCase kNotEqualCases[] = {
        {0, 0, 0},
        {-7, -7, 0},
        {-7, 7, 1},
        {INT_MIN, INT_MAX, 1},
    };
    static const NVVMScalarRuntimeCase kSignedGreaterThanCases[] = {
        {0, 0, 0},
        {-7, -7, 0},
        {-7, 7, 0},
        {7, -7, 1},
        {INT_MIN, INT_MAX, 0},
        {INT_MAX, INT_MIN, 1},
    };
    static const NVVMScalarRuntimeCase kSignedLessEqualCases[] = {
        {0, 0, 1},
        {-7, -7, 1},
        {-7, 7, 1},
        {7, -7, 0},
        {INT_MIN, INT_MAX, 1},
        {INT_MAX, INT_MIN, 0},
    };
    static const NVVMScalarRuntimeCase kSignedGreaterEqualCases[] = {
        {0, 0, 1},
        {-7, -7, 1},
        {-7, 7, 0},
        {7, -7, 1},
        {INT_MIN, INT_MAX, 0},
        {INT_MAX, INT_MIN, 1},
    };
    static const NVVMScalarRuntimeCase kBitAndCases[] = {
        {0x5a, 0x3c, 0x18},
        {-1, 0x12345678, 0x12345678},
        {-2, -4, -4},
        {0, -1, 0},
    };
    static const NVVMScalarRuntimeCase kBitOrCases[] = {
        {0x5a, 0x3c, 0x7e},
        {-16, 3, -13},
        {0, -1, -1},
        {0x55555555, 0x0f0f0f0f, 0x5f5f5f5f},
    };
    static const NVVMScalarRuntimeCase kBitXorCases[] = {
        {0x5a, 0x3c, 0x66},
        {-1, 0x12345678, -305419897},
        {-16, -1, 15},
        {0, -1, -1},
    };
    static const NVVMScalarRuntimeCase kBitNotCases[] = {
        {0, 0, -1},
        {-1, 0, 0},
        {0x55555555, 0, -1431655766},
        {-16, 0, 15},
    };
    static const NVVMScalarRuntimeCase kNegateCases[] = {
        {0, 0, 0},
        {1, 0, -1},
        {-7, 0, 7},
        {-2147483647 - 1, 0, -2147483647 - 1},
    };

    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        RETURN_CASES(kMultiplyCases);
    case NVVMScalarTestOperation::BitAnd:
        RETURN_CASES(kBitAndCases);
    case NVVMScalarTestOperation::BitOr:
        RETURN_CASES(kBitOrCases);
    case NVVMScalarTestOperation::BitXor:
        RETURN_CASES(kBitXorCases);
    case NVVMScalarTestOperation::BitNot:
        RETURN_CASES(kBitNotCases);
    case NVVMScalarTestOperation::Negate:
        RETURN_CASES(kNegateCases);
    case NVVMScalarTestOperation::Equal:
        RETURN_CASES(kEqualCases);
    case NVVMScalarTestOperation::NotEqual:
        RETURN_CASES(kNotEqualCases);
    case NVVMScalarTestOperation::SignedGreaterThan:
        RETURN_CASES(kSignedGreaterThanCases);
    case NVVMScalarTestOperation::SignedLessEqual:
        RETURN_CASES(kSignedLessEqualCases);
    case NVVMScalarTestOperation::SignedGreaterEqual:
        RETURN_CASES(kSignedGreaterEqualCases);
    }
    SLANG_UNEXPECTED("unknown NVVM scalar runtime operation");

#undef RETURN_CASES
}

static void _runNVVMScalarRuntime(
    UnitTestContext* unitTestContext,
    NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(_supportsNVVMScalarTestOperation(preflightBuilder, operation));

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        StringBuilder message;
        message << "Ignoring " << testCase.diagnosticName
                << " runtime differential because the CUDA driver is unavailable.";
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        StringBuilder message;
        message << "Ignoring " << testCase.diagnosticName
                << " runtime differential because no CUDA device is available.";
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        StringBuilder message;
        message << "Ignoring " << testCase.diagnosticName
                << " runtime differential because the device is older than sm_70.";
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        StringBuilder message;
        message << "Ignoring " << testCase.diagnosticName
                << " runtime differential because libNVVM or NVRTC was not found.";
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        SLANG_IGNORE_TEST;
    }

    const NVVMScalarRuntimeCase* runtimeCases = nullptr;
    Index runtimeCaseCount = 0;
    _getNVVMScalarRuntimeCases(operation, runtimeCases, runtimeCaseCount);
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult =
            _compileSlangWithPTXMethod(globalSession, testCase.source, method, code, diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        for (Index i = 0; i < runtimeCaseCount; ++i)
        {
            const NVVMScalarRuntimeCase& runtimeCase = runtimeCases[i];
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                testCase.runtimeOperation,
                runtimeCase.x,
                runtimeCase.y,
                runtimeCase.expected)));
        }
    }
}

#define NVVM_SCALAR_RUNTIME_TEST(NAME, OPERATION)                                   \
    SLANG_UNIT_TEST(NAME)                                                           \
    {                                                                               \
        _runNVVMScalarRuntime(unitTestContext, NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerMultiplyRuntimeMatchesNVRTC, Multiply)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerEqualRuntimeMatchesNVRTC, Equal)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerNotEqualRuntimeMatchesNVRTC, NotEqual)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerSignedGreaterThanRuntimeMatchesNVRTC, SignedGreaterThan)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerSignedLessEqualRuntimeMatchesNVRTC, SignedLessEqual)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerSignedGreaterEqualRuntimeMatchesNVRTC, SignedGreaterEqual)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerBitAndRuntimeMatchesNVRTC, BitAnd)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerBitOrRuntimeMatchesNVRTC, BitOr)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerBitXorRuntimeMatchesNVRTC, BitXor)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerBitNotRuntimeMatchesNVRTC, BitNot)
NVVM_SCALAR_RUNTIME_TEST(nvvmSlangIntegerNegateRuntimeMatchesNVRTC, Negate)

#undef NVVM_SCALAR_RUNTIME_TEST
SLANG_UNIT_TEST(nvvmSlangRelaxedGlobalI32AtomicAddRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsRelaxedGlobalI32AtomicAdd());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring relaxed global signed-i32 atomic-add runtime differential because the CUDA "
            "driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring relaxed global signed-i32 atomic-add runtime differential because no CUDA "
            "device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring relaxed global signed-i32 atomic-add runtime differential because the "
            "device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring relaxed global signed-i32 atomic-add runtime differential because libNVVM "
            "or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMRelaxedGlobalI32AtomicAddSource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(_runRelaxedGlobalI32AtomicAddKernel(cuda, code, 16, 128, 17)));
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderPtxasAcceptsEmptyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring generated-bitcode ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    static const char kKernelName[] = "slangSlice3aPtxasEmpty";
    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _buildEmptyNVVMKernel(builder, toSlice(kKernelName), assemblyBlob, bitcodeBlob)));

    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMBitcode(
        cudaRoot,
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        outputArtifact);
    if (compileResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring generated-bitcode ptxas test because CUDA_PATH does not contain libNVVM.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kKernelName)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(outputArtifact, ptxasPath)));
}

SLANG_UNIT_TEST(nvvmIRBuilderPtxasAcceptsScalarReferenceKernels)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics;
    String bitcodeDiagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarReferenceModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    ComPtr<IArtifact> nvvmArtifact;
    const SlangResult nvvmResult = _compileRealNVVMBitcode(
        cudaRoot,
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        nvvmArtifact);
    if (nvvmResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar ptxas test because CUDA_PATH does not contain libNVVM.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmArtifact != nullptr);

    ComPtr<IArtifact> nvrtcArtifact;
    const SlangResult nvrtcResult =
        _compileRealNVRTCSource(toSlice(kScalarReferenceCUDASource), nvrtcArtifact);
    if (nvrtcResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar ptxas test because NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcArtifact != nullptr);

    SLANG_CHECK(_ptxContainsEntry(nvvmArtifact, toSlice(kWriteScalarKernelName)));
    SLANG_CHECK(_ptxContainsEntry(nvvmArtifact, toSlice(kCopyScalarKernelName)));
    SLANG_CHECK(_ptxContainsEntry(nvrtcArtifact, toSlice(kWriteScalarKernelName)));
    SLANG_CHECK(_ptxContainsEntry(nvrtcArtifact, toSlice(kCopyScalarKernelName)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(nvvmArtifact, ptxasPath)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(nvrtcArtifact, ptxasPath)));
}
