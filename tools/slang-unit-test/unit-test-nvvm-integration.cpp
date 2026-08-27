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

SLANG_UNIT_TEST(nvvmSlangRealIntegerMultiplyDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerMultiply());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerMultiplySource,
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
        kDirectNVVMIntegerMultiplySource,
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
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasMultiply32);
    SLANG_CHECK(nvrtcSummary.hasMultiply32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerEqualDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerEqual());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-equality PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerEqualSource,
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
        kDirectNVVMIntegerEqualSource,
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
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasEqualityComparison32);
    SLANG_CHECK(nvrtcSummary.hasEqualityComparison32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerNotEqualDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerNotEqual());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-inequality PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerNotEqualSource,
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
        kDirectNVVMIntegerNotEqualSource,
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
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasEqualityComparison32);
    SLANG_CHECK(nvrtcSummary.hasEqualityComparison32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}


SLANG_UNIT_TEST(nvvmSlangRealIntegerSignedGreaterThanDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerSignedGreaterThan());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-greater-than PTX differential because libNVVM or NVRTC was "
            "not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerSignedGreaterThanSource,
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
        kDirectNVVMIntegerSignedGreaterThanSource,
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
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasSignedComparison32);
    SLANG_CHECK(nvrtcSummary.hasSignedComparison32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerSignedLessEqualDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerSignedLessEqual());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-less-equal PTX differential because libNVVM or NVRTC was "
            "not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerSignedLessEqualSource,
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
        kDirectNVVMIntegerSignedLessEqualSource,
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
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasSignedComparison32);
    SLANG_CHECK(nvrtcSummary.hasSignedComparison32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerSignedGreaterEqualDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerSignedGreaterEqual());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-greater-equal PTX differential because libNVVM or NVRTC was "
            "not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerSignedGreaterEqualSource,
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
        kDirectNVVMIntegerSignedGreaterEqualSource,
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
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasSignedComparison32);
    SLANG_CHECK(nvrtcSummary.hasSignedComparison32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}


SLANG_UNIT_TEST(nvvmSlangRealIntegerBitAndDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitAnd());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerBitAndSource,
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
        kDirectNVVMIntegerBitAndSource,
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
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasBitAnd32);
    SLANG_CHECK(nvrtcSummary.hasBitAnd32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerBitOrDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitOr());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-OR PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerBitOrSource,
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
        kDirectNVVMIntegerBitOrSource,
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
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasBitOr32);
    SLANG_CHECK(nvrtcSummary.hasBitOr32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerBitXorDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitXor());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-XOR PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerBitXorSource,
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
        kDirectNVVMIntegerBitXorSource,
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
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasBitXor32);
    SLANG_CHECK(nvrtcSummary.hasBitXor32);
    SLANG_CHECK(!nvvmSummary.hasBitOr32);
    SLANG_CHECK(!nvrtcSummary.hasBitOr32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}


SLANG_UNIT_TEST(nvvmSlangRealIntegerBitNotDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitNot());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-NOT PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerBitNotSource,
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
        kDirectNVVMIntegerBitNotSource,
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
    static const uint32_t kParameterWidths[] = {64, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasBitNot32);
    SLANG_CHECK(nvrtcSummary.hasBitNot32);
    SLANG_CHECK(!nvvmSummary.hasBitXor32);
    SLANG_CHECK(!nvrtcSummary.hasBitXor32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}


SLANG_UNIT_TEST(nvvmSlangRealIntegerNegateDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerNegate());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-negate PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerNegateSource,
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
        kDirectNVVMIntegerNegateSource,
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
    static const uint32_t kParameterWidths[] = {64, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasNegate32);
    SLANG_CHECK(nvrtcSummary.hasNegate32);
    SLANG_CHECK(!nvvmSummary.hasBitNot32);
    SLANG_CHECK(!nvrtcSummary.hasBitNot32);
    SLANG_CHECK(!nvvmSummary.hasSubtract32);
    SLANG_CHECK(!nvrtcSummary.hasSubtract32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}


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

SLANG_UNIT_TEST(nvvmSlangRealIntegerMultiplyPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerMultiply());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply ptxas test because CUDA_PATH does not contain ptxas.");
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
            "Ignoring integer-multiply ptxas test because libNVVM or NVRTC was not found.");
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
            kDirectNVVMIntegerMultiplySource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerBitAndPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitAnd());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND ptxas test because CUDA_PATH does not contain ptxas.");
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
            "Ignoring integer-bit-AND ptxas test because libNVVM or NVRTC was not found.");
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
            kDirectNVVMIntegerBitAndSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerBitOrPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitOr());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-OR ptxas test because CUDA_PATH does not contain ptxas.");
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
            "Ignoring integer-bit-OR ptxas test because libNVVM or NVRTC was not found.");
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
            kDirectNVVMIntegerBitOrSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerBitXorPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitXor());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-XOR ptxas test because CUDA_PATH does not contain ptxas.");
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
            "Ignoring integer-bit-XOR ptxas test because libNVVM or NVRTC was not found.");
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
            kDirectNVVMIntegerBitXorSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}


SLANG_UNIT_TEST(nvvmSlangRealIntegerBitNotPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitNot());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-NOT ptxas test because CUDA_PATH does not contain ptxas.");
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
            "Ignoring integer-bit-NOT ptxas test because libNVVM or NVRTC was not found.");
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
            kDirectNVVMIntegerBitNotSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}


SLANG_UNIT_TEST(nvvmSlangRealIntegerNegatePtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerNegate());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-negate ptxas test because CUDA_PATH does not contain ptxas.");
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
            "Ignoring integer-negate ptxas test because libNVVM or NVRTC was not found.");
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
            kDirectNVVMIntegerNegateSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerEqualPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerEqual());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-equality ptxas test because CUDA_PATH does not contain ptxas.");
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
            "Ignoring integer-equality ptxas test because libNVVM or NVRTC was not found.");
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
            kDirectNVVMIntegerEqualSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerNotEqualPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerNotEqual());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-inequality ptxas test because CUDA_PATH does not contain ptxas.");
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
            "Ignoring integer-inequality ptxas test because libNVVM or NVRTC was not found.");
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
            kDirectNVVMIntegerNotEqualSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}


SLANG_UNIT_TEST(nvvmSlangRealIntegerSignedGreaterThanPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerSignedGreaterThan());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-greater-than ptxas test because CUDA_PATH does not contain "
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
            "Ignoring integer-signed-greater-than ptxas test because libNVVM or NVRTC was not "
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
            kDirectNVVMIntegerSignedGreaterThanSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerSignedLessEqualPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerSignedLessEqual());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-less-equal ptxas test because CUDA_PATH does not contain "
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
            "Ignoring integer-signed-less-equal ptxas test because libNVVM or NVRTC was not "
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
            kDirectNVVMIntegerSignedLessEqualSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerSignedGreaterEqualPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerSignedGreaterEqual());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-greater-equal ptxas test because CUDA_PATH does not contain "
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
            "Ignoring integer-signed-greater-equal ptxas test because libNVVM or NVRTC was not "
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
            kDirectNVVMIntegerSignedGreaterEqualSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}


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

SLANG_UNIT_TEST(nvvmSlangIntegerMultiplyRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerMultiply());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply runtime differential because no CUDA device is available.");
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
            "Ignoring integer-multiply runtime differential because the device is older than "
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
            "Ignoring integer-multiply runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct MultiplyRuntimeCase
    {
        int x;
        int y;
        int expected;
    };
    static const MultiplyRuntimeCase kCases[] = {
        {6, 7, 42},
        {-7, 6, -42},
        {0, -19, 0},
    };
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
            kDirectNVVMIntegerMultiplySource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::Multiply,
                runtimeCase.x,
                runtimeCase.y,
                runtimeCase.expected)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangIntegerEqualRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerEqual());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-equality runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-equality runtime differential because no CUDA device is available.");
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
            "Ignoring integer-equality runtime differential because the device is older than "
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
            "Ignoring integer-equality runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct EqualRuntimeCase
    {
        int left;
        int right;
        int expected;
    };
    static const EqualRuntimeCase kCases[] = {
        {0, 0, 1},
        {-7, -7, 1},
        {-7, 7, 0},
        {INT_MIN, INT_MAX, 0},
    };
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
            kDirectNVVMIntegerEqualSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::Equal,
                runtimeCase.left,
                runtimeCase.right,
                runtimeCase.expected)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangIntegerNotEqualRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerNotEqual());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-inequality runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-inequality runtime differential because no CUDA device is "
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
            "Ignoring integer-inequality runtime differential because the device is older than "
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
            "Ignoring integer-inequality runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct NotEqualRuntimeCase
    {
        int left;
        int right;
        int expected;
    };
    static const NotEqualRuntimeCase kCases[] = {
        {0, 0, 0},
        {-7, -7, 0},
        {-7, 7, 1},
        {INT_MIN, INT_MAX, 1},
    };
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
            kDirectNVVMIntegerNotEqualSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::NotEqual,
                runtimeCase.left,
                runtimeCase.right,
                runtimeCase.expected)));
        }
    }
}


SLANG_UNIT_TEST(nvvmSlangIntegerSignedGreaterThanRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerSignedGreaterThan());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-greater-than runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-greater-than runtime differential because no CUDA device is "
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
            "Ignoring integer-signed-greater-than runtime differential because the device is older "
            "than "
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
            "Ignoring integer-signed-greater-than runtime differential because libNVVM or NVRTC "
            "was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct SignedGreaterThanRuntimeCase
    {
        int left;
        int right;
        int expected;
    };
    static const SignedGreaterThanRuntimeCase kCases[] = {
        {0, 0, 0},
        {-7, -7, 0},
        {-7, 7, 0},
        {7, -7, 1},
        {INT_MIN, INT_MAX, 0},
        {INT_MAX, INT_MIN, 1},
    };
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
            kDirectNVVMIntegerSignedGreaterThanSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::GreaterThan,
                runtimeCase.left,
                runtimeCase.right,
                runtimeCase.expected)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangIntegerSignedLessEqualRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerSignedLessEqual());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-less-equal runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-less-equal runtime differential because no CUDA device is "
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
            "Ignoring integer-signed-less-equal runtime differential because the device is older "
            "than sm_70.");
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
            "Ignoring integer-signed-less-equal runtime differential because libNVVM or NVRTC "
            "was not found.");
        SLANG_IGNORE_TEST;
    }

    struct SignedLessEqualRuntimeCase
    {
        int left;
        int right;
        int expected;
    };
    static const SignedLessEqualRuntimeCase kCases[] = {
        {0, 0, 1},
        {-7, -7, 1},
        {-7, 7, 1},
        {7, -7, 0},
        {INT_MIN, INT_MAX, 1},
        {INT_MAX, INT_MIN, 0},
    };
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
            kDirectNVVMIntegerSignedLessEqualSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::LessEqual,
                runtimeCase.left,
                runtimeCase.right,
                runtimeCase.expected)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangIntegerSignedGreaterEqualRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerSignedGreaterEqual());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-greater-equal runtime differential because the CUDA driver "
            "is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-signed-greater-equal runtime differential because no CUDA device is "
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
            "Ignoring integer-signed-greater-equal runtime differential because the device is "
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
            "Ignoring integer-signed-greater-equal runtime differential because libNVVM or NVRTC "
            "was not found.");
        SLANG_IGNORE_TEST;
    }

    struct SignedGreaterEqualRuntimeCase
    {
        int left;
        int right;
        int expected;
    };
    static const SignedGreaterEqualRuntimeCase kCases[] = {
        {0, 0, 1},
        {-7, -7, 1},
        {-7, 7, 0},
        {7, -7, 1},
        {INT_MIN, INT_MAX, 0},
        {INT_MAX, INT_MIN, 1},
    };
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
            kDirectNVVMIntegerSignedGreaterEqualSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::GreaterEqual,
                runtimeCase.left,
                runtimeCase.right,
                runtimeCase.expected)));
        }
    }
}


SLANG_UNIT_TEST(nvvmSlangIntegerBitAndRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitAnd());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND runtime differential because no CUDA device is available.");
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
            "Ignoring integer-bit-AND runtime differential because the device is older than "
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
            "Ignoring integer-bit-AND runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct BitAndRuntimeCase
    {
        int x;
        int y;
        int expected;
    };
    static const BitAndRuntimeCase kCases[] = {
        {0x5a, 0x3c, 0x18},
        {-1, 0x12345678, 0x12345678},
        {-2, -4, -4},
        {0, -1, 0},
    };
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
            kDirectNVVMIntegerBitAndSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::BitAnd,
                runtimeCase.x,
                runtimeCase.y,
                runtimeCase.expected)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangIntegerBitOrRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitOr());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-OR runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-OR runtime differential because no CUDA device is available.");
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
            "Ignoring integer-bit-OR runtime differential because the device is older than "
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
            "Ignoring integer-bit-OR runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct BitOrRuntimeCase
    {
        int x;
        int y;
        int expected;
    };
    static const BitOrRuntimeCase kCases[] = {
        {0x5a, 0x3c, 0x7e},
        {-16, 3, -13},
        {0, -1, -1},
        {0x55555555, 0x0f0f0f0f, 0x5f5f5f5f},
    };
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
            kDirectNVVMIntegerBitOrSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::BitOr,
                runtimeCase.x,
                runtimeCase.y,
                runtimeCase.expected)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangIntegerBitXorRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitXor());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-XOR runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-XOR runtime differential because no CUDA device is available.");
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
            "Ignoring integer-bit-XOR runtime differential because the device is older than "
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
            "Ignoring integer-bit-XOR runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct BitXorRuntimeCase
    {
        int x;
        int y;
        int expected;
    };
    static const BitXorRuntimeCase kCases[] = {
        {0x5a, 0x3c, 0x66},
        {-1, 0x12345678, -305419897},
        {-16, -1, 15},
        {0, -1, -1},
    };
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
            kDirectNVVMIntegerBitXorSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::BitXor,
                runtimeCase.x,
                runtimeCase.y,
                runtimeCase.expected)));
        }
    }
}


SLANG_UNIT_TEST(nvvmSlangIntegerBitNotRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitNot());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-NOT runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-NOT runtime differential because no CUDA device is available.");
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
            "Ignoring integer-bit-NOT runtime differential because the device is older than "
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
            "Ignoring integer-bit-NOT runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct BitNotRuntimeCase
    {
        int x;
        int expected;
    };
    static const BitNotRuntimeCase kCases[] = {
        {0, -1},
        {-1, 0},
        {0x55555555, -1431655766},
        {-16, 15},
    };
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
            kDirectNVVMIntegerBitNotSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::BitNot,
                runtimeCase.x,
                0,
                runtimeCase.expected)));
        }
    }
}


SLANG_UNIT_TEST(nvvmSlangIntegerNegateRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerNegate());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-negate runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-negate runtime differential because no CUDA device is available.");
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
            "Ignoring integer-negate runtime differential because the device is older than "
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
            "Ignoring integer-negate runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct NegateRuntimeCase
    {
        int x;
        int expected;
    };
    static const NegateRuntimeCase kCases[] = {
        {0, 0},
        {1, -1},
        {-7, 7},
        {-2147483647 - 1, -2147483647 - 1},
    };
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
            kDirectNVVMIntegerNegateSource,
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
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::Negate,
                runtimeCase.x,
                0,
                runtimeCase.expected)));
        }
    }
}


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
