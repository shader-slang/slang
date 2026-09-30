// unit-test-nvvm-emitter.cpp

#include "unit-test-nvvm-support.h"

// Gives fake-emitter tests that intentionally request a libdevice operation the same coherent
// toolkit shape required by production preflight. The fake bytes are never parsed by the fake
// compiler; their presence proves the module dependency is carried through discovery.
static SlangResult _configureFakeDirectNVVMLibdevice(
    slang::IGlobalSession* globalSession,
    TempDirectory& toolkit)
{
    static const uint8_t kLibdevice[] = {0x42, 0x43, 0xc0, 0xde, 0x7e, 0x12};
    SLANG_RETURN_ON_FAIL(_createTempDirectory(toolkit));
    String candidatePath;
    String libdevicePath;
    SLANG_RETURN_ON_FAIL(_createFakeNVVMToolkit(
        toolkit.path,
        kLibdevice,
        sizeof(kLibdevice),
        candidatePath,
        libdevicePath));
    globalSession->setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVVM, toolkit.path.getBuffer());
    return SLANG_OK;
}

// Counts calls to the named noinline source helper. Execution-register helper calls must not
// change assertions about a storage helper's ABI or how many times that helper is invoked.
static Index _countFakeNVVMNoInlineHelperCalls(const char* name, size_t parameterCount)
{
    Index helper = -1;
    for (Index i = 0; i < gFakeNVVMBuilder.functionNames.getCount(); ++i)
    {
        const Index type = gFakeNVVMBuilder.functionTypeIndices[i];
        if ((gFakeNVVMBuilder.functionFlags[i] & SLANG_NVVM_FUNCTION_FLAG_NO_INLINE) &&
            gFakeNVVMBuilder.functionNames[i].indexOf(name) >= 0 &&
            gFakeNVVMBuilder.functionTypeParameterCounts[type] == parameterCount)
        {
            SLANG_CHECK_ABORT(helper == -1);
            helper = i;
        }
    }
    SLANG_CHECK_ABORT(helper >= 0);
    Index count = 0;
    for (Index callee : gFakeNVVMBuilder.callCalleeFunctionIndices)
        count += callee == helper;
    return count;
}

// Finds the scalar call by the intrinsic emitted inside its callee, independently of generated
// helper symbols and declaration order.
static FakeNVVMBuilderValueRef _findFakeNVVMNamedIntrinsicCall(const char* name)
{
    Index call = -1;
    for (Index i = 0; i < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount(); ++i)
    {
        const auto intrinsic = gFakeNVVMBuilder.namedIntrinsicFunctionNames.tryGetValue(
            gFakeNVVMBuilder.callCalleeFunctionIndices[i]);
        if (intrinsic && *intrinsic == name)
        {
            SLANG_CHECK_ABORT(call == -1);
            call = i;
        }
    }
    SLANG_CHECK_ABORT(call >= 0);
    return {FakeNVVMBuilderValueKind::Call, call};
}

// Matches one binary dataflow edge by its actual operands. The group-index test uses this to
// check arithmetic semantics instead of counting vector extracts that canonicalization can fold.
static FakeNVVMBuilderValueRef _findFakeNVVMScalarBinary(
    SlangNVVMValueOperation operation,
    FakeNVVMBuilderValueRef left,
    FakeNVVMBuilderValueRef right)
{
    for (Index i = 0; i < gFakeNVVMBuilder.scalarOperations.getCount(); ++i)
    {
        const auto& candidate = gFakeNVVMBuilder.scalarOperations[i];
        if (candidate.key.operation == operation && candidate.operandCount == 2 &&
            candidate.operands[0].kind == left.kind && candidate.operands[0].index == left.index &&
            candidate.operands[0].functionIndex == left.functionIndex &&
            candidate.operands[1].kind == right.kind &&
            candidate.operands[1].index == right.index &&
            candidate.operands[1].functionIndex == right.functionIndex)
            return {FakeNVVMBuilderValueKind::ScalarOperation, i};
    }
    SLANG_CHECK_ABORT(false);
    return {};
}

SLANG_UNIT_TEST(nvvmSlangRoutesGenericScalarFamilies)
{
    enum class Family
    {
        Unary,
        Binary,
        Compare,
    };
    struct Case
    {
        const char* source;
        Family family;
        uint32_t operation;
    };
    static const Case kCases[] = {
        {kDirectNVVMIntegerNegateSource, Family::Unary, SLANG_NVVM_VALUE_OP_NEGATE},
        {kDirectNVVMIntegerMultiplySource, Family::Binary, SLANG_NVVM_VALUE_OP_MULTIPLY},
        {kDirectNVVMIntegerEqualSource, Family::Compare, SLANG_NVVM_VALUE_OP_EQUAL},
    };

    for (const auto& testCase : kCases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                _compileSlangWithDirectNVVM(globalSession, testCase.source, code, diagnostics)));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

            if (testCase.family == Family::Unary)
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(
                        FakeNVVMBuilderScalarFamily::Unary)] == 1);
            }
            else if (testCase.family == Family::Binary)
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(
                        FakeNVVMBuilderScalarFamily::Binary)] == 1);
            }
            else
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(
                        FakeNVVMBuilderScalarFamily::Compare)] == 1);
            }
            SLANG_CHECK(gFakeNVVMBuilder.emittedValueOperations.getCount() == 1);
            SLANG_CHECK(gFakeNVVMBuilder.emittedValueOperations[0].operation == testCase.operation);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    }
}

static void _runNVVMSlangFloat32ArithmeticUsesDirectPipeline(
    NVVMFloat32ArithmeticTestOperation testOperation)
{
    const NVVMFloat32ArithmeticTestCase& testCase =
        _getNVVMFloat32ArithmeticTestCase(testOperation);
    const FakeNVVMBuilderScalarFamily family = testCase.operandCount == 1
                                                   ? FakeNVVMBuilderScalarFamily::FloatingUnary
                                                   : FakeNVVMBuilderScalarFamily::FloatingBinary;
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            _compileSlangWithDirectNVVM(globalSession, testCase.source, code, diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.floatingPointBitWidth == 32);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes[0] == _getFakeNVVMBuilderFloatType());
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds.getCount() == testCase.operandCount + 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[0] ==
            FakeNVVMBuilderParameterTypeKind::FloatPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[1] ==
            FakeNVVMBuilderParameterTypeKind::Float);
        if (testCase.operandCount == 2)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[2] ==
                FakeNVVMBuilderParameterTypeKind::Float);
        }
        SLANG_CHECK(gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(family)] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 1);
        const FakeNVVMBuilderScalarOperation& operation = gFakeNVVMBuilder.scalarOperations[0];
        SLANG_CHECK(operation.key.family == family);
        SLANG_CHECK(operation.key.operation == testCase.operation);
        SLANG_CHECK(operation.operandCount == testCase.operandCount);
        SLANG_CHECK(operation.operands[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(operation.operands[0].index == 1);
        if (testCase.operandCount == 2)
        {
            SLANG_CHECK(operation.operands[1].kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(operation.operands[1].index == 2);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::ScalarOperation);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

#define NVVM_FLOAT32_ARITHMETIC_DIRECT_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                    \
    {                                                        \
        _runNVVMSlangFloat32ArithmeticUsesDirectPipeline(    \
            NVVMFloat32ArithmeticTestOperation::OPERATION);  \
    }

NVVM_FLOAT32_ARITHMETIC_DIRECT_TEST(nvvmSlangFloat32AddUsesDirectPipeline, Add)
NVVM_FLOAT32_ARITHMETIC_DIRECT_TEST(nvvmSlangFloat32SubtractUsesDirectPipeline, Subtract)
NVVM_FLOAT32_ARITHMETIC_DIRECT_TEST(nvvmSlangFloat32MultiplyUsesDirectPipeline, Multiply)
NVVM_FLOAT32_ARITHMETIC_DIRECT_TEST(nvvmSlangFloat32DivideUsesDirectPipeline, Divide)
NVVM_FLOAT32_ARITHMETIC_DIRECT_TEST(nvvmSlangFloat32NegateUsesDirectPipeline, Negate)

#undef NVVM_FLOAT32_ARITHMETIC_DIRECT_TEST

static void _runNVVMSlangFloat32ComparisonUsesDirectPipeline(
    NVVMFloat32ComparisonTestOperation testOperation)
{
    const NVVMFloat32ComparisonTestCase& testCase =
        _getNVVMFloat32ComparisonTestCase(testOperation);
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            _compileSlangWithDirectNVVM(globalSession, testCase.source, code, diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.floatingPointBitWidth == 32);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterTypeKinds.getCount() == 3);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[0] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[1] ==
            FakeNVVMBuilderParameterTypeKind::Float);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[2] ==
            FakeNVVMBuilderParameterTypeKind::Float);

        SLANG_CHECK(
            gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(
                FakeNVVMBuilderScalarFamily::FloatingCompare)] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 1);
        const FakeNVVMBuilderScalarOperation& comparison = gFakeNVVMBuilder.scalarOperations[0];
        SLANG_CHECK(comparison.key.family == FakeNVVMBuilderScalarFamily::FloatingCompare);
        SLANG_CHECK(comparison.key.operation == testCase.operation);
        SLANG_CHECK(comparison.operandCount == 2);
        SLANG_CHECK(comparison.operands[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(comparison.operands[0].index == 1);
        SLANG_CHECK(comparison.operands[1].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(comparison.operands[1].index == 2);
        SLANG_CHECK(comparison.callerBlockIndex == gFakeNVVMBuilder.conditionalSourceBlockIndex);

        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.addPhiIncomingCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::ScalarPhi);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

#define NVVM_FLOAT32_COMPARISON_DIRECT_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                    \
    {                                                        \
        _runNVVMSlangFloat32ComparisonUsesDirectPipeline(    \
            NVVMFloat32ComparisonTestOperation::OPERATION);  \
    }

NVVM_FLOAT32_COMPARISON_DIRECT_TEST(nvvmSlangFloat32EqualUsesDirectPipeline, OrderedEqual)
NVVM_FLOAT32_COMPARISON_DIRECT_TEST(nvvmSlangFloat32NotEqualUsesDirectPipeline, UnorderedNotEqual)
NVVM_FLOAT32_COMPARISON_DIRECT_TEST(
    nvvmSlangFloat32GreaterThanUsesDirectPipeline,
    OrderedGreaterThan)
NVVM_FLOAT32_COMPARISON_DIRECT_TEST(nvvmSlangFloat32LessEqualUsesDirectPipeline, OrderedLessEqual)
NVVM_FLOAT32_COMPARISON_DIRECT_TEST(
    nvvmSlangFloat32GreaterEqualUsesDirectPipeline,
    OrderedGreaterEqual)
NVVM_FLOAT32_COMPARISON_DIRECT_TEST(nvvmSlangFloat32LessThanUsesDirectPipeline, OrderedLessThan)

#undef NVVM_FLOAT32_COMPARISON_DIRECT_TEST

SLANG_UNIT_TEST(nvvmSlangFloat32ConstantUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloat32ConstantSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.floatingPointBitWidth == 32);
        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointConstantCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.floatingPointConstantBitWidths.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.floatingPointConstantBitWidths[0] == 32);
        SLANG_CHECK(gFakeNVVMBuilder.floatingPointConstantBitPatterns.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.floatingPointConstantBitPatterns[0] == UINT64_C(0x3fc00000));
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterTypeKinds.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[0] ==
            FakeNVVMBuilderParameterTypeKind::FloatPointer);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::FloatingPointConstant);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFloat32PhiUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloat32PhiSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.functionParameterTypeKinds.getCount() == 4);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[0] ==
            FakeNVVMBuilderParameterTypeKind::FloatPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[1] ==
            FakeNVVMBuilderParameterTypeKind::Integer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[2] ==
            FakeNVVMBuilderParameterTypeKind::Float);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[3] ==
            FakeNVVMBuilderParameterTypeKind::Float);
        SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.addPhiIncomingCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes[0] == _getFakeNVVMBuilderFloatType());
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiIncomingValueRefs.getCount() == 2);
        const FakeNVVMBuilderValueRef firstIncoming =
            gFakeNVVMBuilder.scalarPhiIncomingValueRefs[0];
        const FakeNVVMBuilderValueRef secondIncoming =
            gFakeNVVMBuilder.scalarPhiIncomingValueRefs[1];
        SLANG_CHECK(firstIncoming.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(secondIncoming.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(
            (firstIncoming.index == 2 && secondIncoming.index == 3) ||
            (firstIncoming.index == 3 && secondIncoming.index == 2));
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::ScalarPhi);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFloat32FunctionsUseDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloat32FunctionSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeResultKinds.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[0] == FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[1] == FakeNVVMBuilderResultTypeKind::Float);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[0] == 3);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[1] == 2);
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterTypeKinds.getCount() == 5);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[0] ==
            FakeNVVMBuilderParameterTypeKind::FloatPointer);
        for (Index i = 1; i < 5; ++i)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[i] ==
                FakeNVVMBuilderParameterTypeKind::Float);
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.callCalleeFunctionIndices[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.callArgumentCounts[0] == 2);
        SLANG_CHECK(gFakeNVVMBuilder.callResultTypes[0] == _getFakeNVVMBuilderFloatType());
        const Index argumentOffset = gFakeNVVMBuilder.callArgumentOffsets[0];
        const FakeNVVMBuilderValueRef leftArgument =
            gFakeNVVMBuilder.callArgumentValueRefs[argumentOffset];
        const FakeNVVMBuilderValueRef rightArgument =
            gFakeNVVMBuilder.callArgumentValueRefs[argumentOffset + 1];
        SLANG_CHECK(leftArgument.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(leftArgument.functionIndex == 0);
        SLANG_CHECK(leftArgument.index == 1);
        SLANG_CHECK(rightArgument.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(rightArgument.functionIndex == 0);
        SLANG_CHECK(rightArgument.index == 2);

        SLANG_CHECK(
            gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(
                FakeNVVMBuilderScalarFamily::FloatingBinary)] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 1);
        const FakeNVVMBuilderScalarOperation& addition = gFakeNVVMBuilder.scalarOperations[0];
        SLANG_CHECK(addition.key.family == FakeNVVMBuilderScalarFamily::FloatingBinary);
        SLANG_CHECK(addition.key.operation == SLANG_NVVM_VALUE_OP_ADD);
        SLANG_CHECK(addition.operands[0].functionIndex == 1);
        SLANG_CHECK(addition.operands[0].index == 0);
        SLANG_CHECK(addition.operands[1].functionIndex == 1);
        SLANG_CHECK(addition.operands[1].index == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarReturnValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarReturnValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ScalarOperation);
        SLANG_CHECK(gFakeNVVMBuilder.scalarReturnValueRefs[0].index == 0);

        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangWaveLaneIndexUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMWaveLaneIndexSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeResultKinds.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[0] == FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[1] == FakeNVVMBuilderResultTypeKind::Integer);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[1] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterTypeKinds.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[0] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);

        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicNames[0] == "llvm.nvvm.read.ptx.sreg.laneid");
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicCallerBlockIndices[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarReturnValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarReturnValueRefs[0].kind == FakeNVVMBuilderValueKind::Intrinsic);

        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.callCalleeFunctionIndices[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.callCallerBlockIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.callArgumentCounts[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.callResultTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCUDAExecutionUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCUDAExecutionSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.barrier0"));
        for (const char* reg : {"tid", "ctaid", "ntid", "nctaid"})
        {
            for (char axis = 'x'; axis <= 'z'; ++axis)
            {
                StringBuilder name;
                name << "llvm.nvvm.read.ptx.sreg." << reg << "." << axis;
                SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains(name));
            }
        }
        SLANG_CHECK(gFakeNVVMBuilder.workgroupBarrierCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 12);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangIntegerVectorSwizzleUsesGenericConstruction)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMIntegerVectorSwizzleSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 9);
        Index swizzleConstruction = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.vectorConstructResultTypes.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.vectorConstructResultTypes[i] == _getFakeNVVMBuilderVectorType(2))
            {
                SLANG_CHECK(swizzleConstruction == -1);
                swizzleConstruction = i;
            }
        }
        SLANG_CHECK_ABORT(swizzleConstruction >= 0);
        const Index constructOffset =
            gFakeNVVMBuilder.vectorConstructElementOffsets[swizzleConstruction];
        for (Index i = 0; i < 2; ++i)
        {
            const FakeNVVMBuilderValueRef element =
                gFakeNVVMBuilder.vectorConstructElementValueRefs[constructOffset + i];
            SLANG_CHECK(element.kind == FakeNVVMBuilderValueKind::VectorElement);
            SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[element.index] == uint32_t(i));
        }

        bool sawUnsignedVectorMultiply = false;
        bool sawUnsignedVectorAdd = false;
        bool sawSignedVectorConversion = false;
        for (Index i = 0; i < gFakeNVVMBuilder.scalarOperations.getCount(); ++i)
        {
            const FakeNVVMBuilderScalarOperation& operation = gFakeNVVMBuilder.scalarOperations[i];
            const SlangNVVMValueTypeDesc& resultType = operation.resultType;
            sawUnsignedVectorMultiply =
                sawUnsignedVectorMultiply ||
                (operation.key.operation == SLANG_NVVM_VALUE_OP_MULTIPLY &&
                 resultType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER &&
                 resultType.laneCount == 3);
            sawUnsignedVectorAdd = sawUnsignedVectorAdd ||
                                   (operation.key.operation == SLANG_NVVM_VALUE_OP_ADD &&
                                    resultType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER &&
                                    resultType.laneCount == 3);
            sawSignedVectorConversion =
                sawSignedVectorConversion ||
                (operation.key.operation == SLANG_NVVM_VALUE_OP_INTEGER_CONVERT &&
                 resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER &&
                 resultType.bitWidth == 32 && resultType.laneCount == 2);
        }
        SLANG_CHECK(sawUnsignedVectorMultiply);
        SLANG_CHECK(sawUnsignedVectorAdd);
        SLANG_CHECK(sawSignedVectorConversion);

        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags[0] == SLANG_NVVM_LOAD_FLAG_INVARIANT);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// Match the complete independently stated conversion signature, not merely BIT_REINTERPRET.
// The fake provider's integer vector handles collapse widths, so the descriptor also proves i16.
static bool _isHalfVectorBoundaryBitcast(
    const FakeNVVMBuilderScalarOperation& operation,
    uint32_t width,
    bool encode)
{
    const SlangNVVMValueTypeDesc half = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 16, width};
    const SlangNVVMValueTypeDesc integer = {SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER, 16, width};
    return operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET &&
           operation.operandCount == 1 &&
           NVVMSemantics::areSameType(operation.resultType, encode ? integer : half) &&
           NVVMSemantics::areSameType(operation.operandTypes[0], encode ? half : integer);
}

SLANG_UNIT_TEST(nvvmSlangVectorConstructionFlattensMixedOperands)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFlattenedVectorConstructionSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        Index flattenedConstructIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.vectorConstructResultTypes.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.vectorConstructResultTypes[i] ==
                _getFakeNVVMBuilderVectorType(4, FakeNVVMBuilderScalarTypeKind::Half))
            {
                flattenedConstructIndex = i;
                break;
            }
        }
        SLANG_CHECK_ABORT(flattenedConstructIndex >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.vectorConstructElementCounts[flattenedConstructIndex] == 4);
        const Index elementOffset =
            gFakeNVVMBuilder.vectorConstructElementOffsets[flattenedConstructIndex];
        const FakeNVVMBuilderValueRef first =
            gFakeNVVMBuilder.vectorConstructElementValueRefs[elementOffset];
        const FakeNVVMBuilderValueRef second =
            gFakeNVVMBuilder.vectorConstructElementValueRefs[elementOffset + 1];
        SLANG_CHECK_ABORT(
            first.kind == FakeNVVMBuilderValueKind::VectorElement && first.index >= 0 &&
            first.index < gFakeNVVMBuilder.vectorElementIndices.getCount());
        SLANG_CHECK_ABORT(
            second.kind == FakeNVVMBuilderValueKind::VectorElement && second.index >= 0 &&
            second.index < gFakeNVVMBuilder.vectorElementIndices.getCount());
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[first.index] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[second.index] == 1);
        const FakeNVVMBuilderValueRef firstBase =
            gFakeNVVMBuilder.vectorElementBaseValueRefs[first.index];
        SLANG_CHECK_ABORT(
            firstBase.kind == FakeNVVMBuilderValueKind::ScalarOperation && firstBase.index >= 0 &&
            firstBase.index < gFakeNVVMBuilder.scalarOperations.getCount());
        const auto& decode = gFakeNVVMBuilder.scalarOperations[firstBase.index];
        SLANG_CHECK_ABORT(_isHalfVectorBoundaryBitcast(decode, 2, false));
        const auto call = decode.operands[0];
        SLANG_CHECK_ABORT(
            call.kind == FakeNVVMBuilderValueKind::Call && call.index >= 0 &&
            call.index < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount());
        const auto physicalHalf2 =
            _getFakeNVVMBuilderVectorType(2, FakeNVVMBuilderScalarTypeKind::Integer);
        SLANG_CHECK(gFakeNVVMBuilder.callResultTypes[call.index] == physicalHalf2);
        SLANG_CHECK(decode.callerBlockIndex == gFakeNVVMBuilder.callCallerBlockIndices[call.index]);

        Index makePairFunction = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.functionNames.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.functionNames[i].indexOf("makePair") >= 0)
            {
                SLANG_CHECK(makePairFunction == -1);
                makePairFunction = i;
            }
        }
        SLANG_CHECK_ABORT(makePairFunction >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.callCalleeFunctionIndices[call.index] == makePairFunction);
        const Index makePairType = gFakeNVVMBuilder.functionTypeIndices[makePairFunction];
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeResultTypes[makePairType] == physicalHalf2);
        Index makePairCallCount = 0;
        for (Index callee : gFakeNVVMBuilder.callCalleeFunctionIndices)
            makePairCallCount += callee == makePairFunction;
        SLANG_CHECK(makePairCallCount == 1);

        // Both constructor lanes must come from this one decoded call snapshot.
        const FakeNVVMBuilderValueRef secondBase =
            gFakeNVVMBuilder.vectorElementBaseValueRefs[second.index];
        SLANG_CHECK(secondBase.kind == firstBase.kind);
        SLANG_CHECK(secondBase.index == firstBase.index);
        SLANG_CHECK(secondBase.functionIndex == firstBase.functionIndex);
        SLANG_CHECK(
            gFakeNVVMBuilder.vectorConstructElementValueRefs[elementOffset + 2].kind ==
            FakeNVVMBuilderValueKind::ScalarOperation);
        SLANG_CHECK(
            gFakeNVVMBuilder.vectorConstructElementValueRefs[elementOffset + 3].kind ==
            FakeNVVMBuilderValueKind::ScalarOperation);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFloatMatrixValuesUseLegalizedAggregates)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloatMatrixValueSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder trace;
            trace << "matrix fake trace: modules " << gFakeNVVMBuilder.createModuleCallCount
                  << "; arrays " << gFakeNVVMBuilder.getArrayTypeCallCount << "; aggregate makes "
                  << gFakeNVVMBuilder.emitAggregateConstructCallCount << "; aggregate extracts "
                  << gFakeNVVMBuilder.emitAggregateElementExtractCallCount << "; vector makes "
                  << gFakeNVVMBuilder.emitVectorConstructCallCount << "; vector extracts "
                  << gFakeNVVMBuilder.emitSequentialElementExtractCallCount << "; phis "
                  << gFakeNVVMBuilder.emitPhiCallCount << "; phi incoming "
                  << gFakeNVVMBuilder.addPhiIncomingCallCount << "; value ops "
                  << gFakeNVVMBuilder.emittedValueOperations.getCount();
            getTestReporter()->message(TestMessageType::TestFailure, trace.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.arrayElementType ==
            _getFakeNVVMBuilderVectorType(2, FakeNVVMBuilderScalarTypeKind::Float));
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateConstructCallCount >= 2);
        for (auto resultType : gFakeNVVMBuilder.aggregateConstructResultTypes)
            SLANG_CHECK(resultType == _getFakeNVVMBuilderArrayType());

        bool sawArrayPhi = false;
        for (auto phiType : gFakeNVVMBuilder.scalarPhiTypes)
            sawArrayPhi = sawArrayPhi || phiType == _getFakeNVVMBuilderArrayType();
        SLANG_CHECK(sawArrayPhi);

        bool sawSelectedRow = false;
        for (Index i = 0; i < gFakeNVVMBuilder.aggregateElementBaseValueRefs.getCount(); ++i)
        {
            const FakeNVVMBuilderValueRef base = gFakeNVVMBuilder.aggregateElementBaseValueRefs[i];
            sawSelectedRow = sawSelectedRow || (base.kind == FakeNVVMBuilderValueKind::ScalarPhi &&
                                                gFakeNVVMBuilder.aggregateElementIndices[i] == 1 &&
                                                gFakeNVVMBuilder.aggregateElementTypeKinds[i] ==
                                                    FakeNVVMBuilderScalarTypeKind::Float2);
        }
        SLANG_CHECK(sawSelectedRow);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangMatrixMemoryUsesSequentialPointerContract)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMMatrixMemorySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder trace;
            trace << "matrix-memory fake trace: functions "
                  << gFakeNVVMBuilder.declareFunctionCallCount << "; arrays "
                  << gFakeNVVMBuilder.getArrayTypeCallCount << "; locals "
                  << gFakeNVVMBuilder.emitLocalStorageCallCount << "; sequential pointers "
                  << gFakeNVVMBuilder.emitSequentialElementPointerCallCount << "; loads "
                  << gFakeNVVMBuilder.emitLoadCallCount << "; stores "
                  << gFakeNVVMBuilder.emitStoreCallCount << "; calls "
                  << gFakeNVVMBuilder.emitCallCallCount << "; blocks "
                  << gFakeNVVMBuilder.createBlockCallCount << "; sequential extracts "
                  << gFakeNVVMBuilder.emitSequentialElementExtractCallCount << "; struct types "
                  << gFakeNVVMBuilder.getStructTypeCallCount << "; field pointers "
                  << gFakeNVVMBuilder.emitStructFieldPointerCallCount << "; aggregate extracts "
                  << gFakeNVVMBuilder.emitAggregateElementExtractCallCount
                  << "; aggregate constructs " << gFakeNVVMBuilder.emitAggregateConstructCallCount
                  << "; vector constructs " << gFakeNVVMBuilder.emitVectorConstructCallCount
                  << "; value operations " << gFakeNVVMBuilder.scalarOperations.getCount()
                  << "; completed aggregates "
                  << gFakeNVVMBuilder.aggregateConstructResultTypes.getCount()
                  << "; completed vectors "
                  << gFakeNVVMBuilder.vectorConstructResultTypes.getCount()
                  << "; completed sequential extracts "
                  << gFakeNVVMBuilder.vectorElementIndices.getCount() << "; emitted operations "
                  << gFakeNVVMBuilder.emittedValueOperations.getCount();
            getTestReporter()->message(TestMessageType::TestFailure, trace.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 4);
        SLANG_CHECK(
            gFakeNVVMBuilder.arrayElementType ==
            _getFakeNVVMBuilderVectorType(4, FakeNVVMBuilderScalarTypeKind::Float));

        bool sawPhysicalStorageParameterGroupField = false;
        for (auto fieldType : gFakeNVVMBuilder.structFieldTypes)
        {
            sawPhysicalStorageParameterGroupField |=
                fieldType == _getFakeNVVMBuilderScalarStructPointerType();
        }
        SLANG_CHECK(sawPhysicalStorageParameterGroupField);
        bool sawPhysicalStorageArrayField = false;
        for (auto fieldType : gFakeNVVMBuilder.scalarStructFieldTypes)
            sawPhysicalStorageArrayField |= fieldType == _getFakeNVVMBuilderArrayType();
        SLANG_CHECK(sawPhysicalStorageArrayField);

        Index internalFunctionCount = 0;
        bool sawCollidingExportName = false;
        for (Index i = 0; i < gFakeNVVMBuilder.functionNames.getCount(); ++i)
        {
            const bool isPrivateGenerated =
                gFakeNVVMBuilder.functionNames[i].startsWith("__slang_nvvm_internal_");
            if (gFakeNVVMBuilder.functionNames[i] == "__slang_nvvm_internal_0")
            {
                sawCollidingExportName = true;
                SLANG_CHECK(gFakeNVVMBuilder.functionLinkages[i] == SLANG_NVVM_LINKAGE_EXTERNAL);
            }
            else if (isPrivateGenerated)
            {
                SLANG_CHECK(gFakeNVVMBuilder.functionLinkages[i] == SLANG_NVVM_LINKAGE_INTERNAL);
            }
            if (gFakeNVVMBuilder.functionLinkages[i] == SLANG_NVVM_LINKAGE_INTERNAL)
                ++internalFunctionCount;
            SLANG_CHECK(gFakeNVVMBuilder.functionNames[i].getLength() != 0);
        }
        SLANG_CHECK(sawCollidingExportName);
        SLANG_CHECK(internalFunctionCount >= 1);

        bool sawSequentialElementPointer = false;
        bool sawVectorLanePointer = false;
        for (auto resultTypeKind : gFakeNVVMBuilder.sequentialElementPointerTypeKinds)
        {
            sawSequentialElementPointer |= resultTypeKind == FakeNVVMBuilderScalarTypeKind::Float4;
            sawVectorLanePointer |= resultTypeKind == FakeNVVMBuilderScalarTypeKind::Float;
        }
        SLANG_CHECK(sawSequentialElementPointer);
        SLANG_CHECK(sawVectorLanePointer);

        bool sawWholeArrayLoad = false;
        bool sawParameterGroupStoragePointerLoad = false;
        for (auto resultTypeKind : gFakeNVVMBuilder.loadResultTypeKinds)
        {
            sawWholeArrayLoad |= resultTypeKind == FakeNVVMBuilderScalarTypeKind::NumericArray;
            sawParameterGroupStoragePointerLoad |=
                resultTypeKind == FakeNVVMBuilderScalarTypeKind::ScalarStructPointer;
        }
        SLANG_CHECK(sawWholeArrayLoad);
        SLANG_CHECK(sawParameterGroupStoragePointerLoad);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount > 16);
        SLANG_CHECK(
            gFakeNVVMBuilder.emitSequentialElementExtractCallCount ==
            gFakeNVVMBuilder.vectorElementIndices.getCount());
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangStructuredMatrixMemoryUsesPhysicalResourceStorage)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMStructuredMatrixMemorySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder trace;
            trace << "structured-matrix fake trace: arrays "
                  << gFakeNVVMBuilder.getArrayTypeCallCount << "; structs "
                  << gFakeNVVMBuilder.getStructTypeCallCount << "; field pointers "
                  << gFakeNVVMBuilder.emitStructFieldPointerCallCount << "; sequential pointers "
                  << gFakeNVVMBuilder.emitSequentialElementPointerCallCount << "; pointer offsets "
                  << gFakeNVVMBuilder.emitPointerOffsetCallCount << "; loads "
                  << gFakeNVVMBuilder.emitLoadCallCount << "; stores "
                  << gFakeNVVMBuilder.emitStoreCallCount << "; operations "
                  << gFakeNVVMBuilder.scalarOperations.getCount();
            getTestReporter()->message(TestMessageType::TestFailure, trace.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 4);
        SLANG_CHECK(
            gFakeNVVMBuilder.arrayElementType ==
            _getFakeNVVMBuilderVectorType(4, FakeNVVMBuilderScalarTypeKind::Float));

        bool sawPhysicalArrayField = false;
        for (auto fieldType : gFakeNVVMBuilder.scalarStructFieldTypes)
            sawPhysicalArrayField |= fieldType == _getFakeNVVMBuilderArrayType();
        SLANG_CHECK(sawPhysicalArrayField);

        bool sawRowPointer = false;
        bool sawLanePointer = false;
        for (auto resultTypeKind : gFakeNVVMBuilder.sequentialElementPointerTypeKinds)
        {
            sawRowPointer |= resultTypeKind == FakeNVVMBuilderScalarTypeKind::Float4;
            sawLanePointer |= resultTypeKind == FakeNVVMBuilderScalarTypeKind::Float;
        }
        SLANG_CHECK(sawRowPointer);
        SLANG_CHECK(sawLanePointer);

        bool sawFloatToSignedI32Bits = false;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            sawFloatToSignedI32Bits |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET &&
                operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER &&
                operation.resultType.bitWidth == 32 && operation.resultType.laneCount == 1 &&
                operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                operation.operandTypes[0].bitWidth == 32 &&
                operation.operandTypes[0].laneCount == 1;
        }
        SLANG_CHECK(sawFloatToSignedI32Bits);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount >= 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount >= 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangDynamicLocalVectorStoreUsesSequentialPointerContract)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMDynamicLocalVectorStoreSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawHalfLanePointer = false;
        for (auto resultTypeKind : gFakeNVVMBuilder.sequentialElementPointerTypeKinds)
            sawHalfLanePointer |= resultTypeKind == FakeNVVMBuilderScalarTypeKind::Half;
        SLANG_CHECK(sawHalfLanePointer);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangVectorOperationFamiliesUseTypedDescriptors)
{
#if !(SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY)
    SLANG_IGNORE_TEST;
    return;
#endif
    _resetDirectNVVMFakes();
    TempDirectory toolkit;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(_configureFakeDirectNVVMLibdevice(globalSession, toolkit)));

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMVectorOperationFamilySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder state;
            state << "direct NVVM result " << int(result) << "; types "
                  << gFakeNVVMBuilder.getVectorTypeCallCount << "; constructs "
                  << gFakeNVVMBuilder.emitVectorConstructCallCount << "; operations "
                  << gFakeNVVMBuilder.scalarOperations.getCount() << "; extracts "
                  << gFakeNVVMBuilder.emitSequentialElementExtractCallCount << "; stores "
                  << gFakeNVVMBuilder.emitStoreCallCount << "; modules "
                  << gFakeNVVMBuilder.createModuleCallCount << "; programs "
                  << gFakeNVVM.createProgramCallCount;
            for (const FakeNVVMBuilderScalarOperation& operation :
                 gFakeNVVMBuilder.scalarOperations)
            {
                state << "; op " << operation.key.operation << " type " << operation.resultType.kind
                      << "/" << operation.resultType.bitWidth << "/"
                      << operation.resultType.laneCount;
            }
            getTestReporter()->message(TestMessageType::TestFailure, state.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawSignedI32x2RightShift = false;
        bool sawSignedI32x2VectorScalarAdd = false;
        bool sawSignedI32x2ScalarVectorSubtract = false;
        bool sawSignedI8x2Divide = false;
        bool sawSignedI8x2Remainder = false;
        bool sawSignedI8x2LessThan = false;
        bool sawFloat32x3Add = false;
        int float32ScalarFmodCount = 0;
        bool sawFloat32x3VectorScalarAdd = false;
        bool sawFloat32x3LessThan = false;
        bool sawBooleanVectorNot = false;
        bool sawBooleanVectorScalarAnd = false;
        bool sawBooleanVectorOr = false;
        bool sawBooleanVectorEqual = false;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            const SlangNVVMValueTypeDesc& type = operation.resultType;
            sawSignedI32x2RightShift =
                sawSignedI32x2RightShift ||
                (operation.key.operation == SLANG_NVVM_VALUE_OP_SHIFT_RIGHT &&
                 type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER && type.bitWidth == 32 &&
                 type.laneCount == 2 && operation.operandTypes[0].laneCount == 2 &&
                 operation.operandTypes[1].laneCount == 1);
            sawSignedI32x2VectorScalarAdd =
                sawSignedI32x2VectorScalarAdd ||
                (operation.key.operation == SLANG_NVVM_VALUE_OP_ADD &&
                 type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER && type.bitWidth == 32 &&
                 type.laneCount == 2 && operation.operandTypes[0].laneCount == 2 &&
                 operation.operandTypes[1].laneCount == 1);
            sawSignedI32x2ScalarVectorSubtract =
                sawSignedI32x2ScalarVectorSubtract ||
                (operation.key.operation == SLANG_NVVM_VALUE_OP_SUBTRACT &&
                 type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER && type.bitWidth == 32 &&
                 type.laneCount == 2 && operation.operandTypes[0].laneCount == 1 &&
                 operation.operandTypes[1].laneCount == 2);
            sawSignedI8x2Divide =
                sawSignedI8x2Divide || (operation.key.operation == SLANG_NVVM_VALUE_OP_DIVIDE &&
                                        type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER &&
                                        type.bitWidth == 8 && type.laneCount == 2);
            sawSignedI8x2Remainder = sawSignedI8x2Remainder ||
                                     (operation.key.operation == SLANG_NVVM_VALUE_OP_REMAINDER &&
                                      type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER &&
                                      type.bitWidth == 8 && type.laneCount == 2);
            sawSignedI8x2LessThan =
                sawSignedI8x2LessThan ||
                (operation.key.operation == SLANG_NVVM_VALUE_OP_LESS_THAN &&
                 type.kind == SLANG_NVVM_VALUE_TYPE_BOOL && type.bitWidth == 1 &&
                 type.laneCount == 2 && operation.operandTypes[0].laneCount == 2 &&
                 operation.operandTypes[1].laneCount == 1);
            sawFloat32x3Add =
                sawFloat32x3Add || (operation.key.operation == SLANG_NVVM_VALUE_OP_ADD &&
                                    type.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                                    type.bitWidth == 32 && type.laneCount == 3);
            if (operation.key.operation == SLANG_NVVM_VALUE_OP_FMOD &&
                type.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT && type.bitWidth == 32 &&
                type.laneCount == 1 && operation.operandTypes[0].laneCount == 1 &&
                operation.operandTypes[1].laneCount == 1)
            {
                ++float32ScalarFmodCount;
            }
            sawFloat32x3VectorScalarAdd =
                sawFloat32x3VectorScalarAdd ||
                (operation.key.operation == SLANG_NVVM_VALUE_OP_ADD &&
                 type.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT && type.bitWidth == 32 &&
                 type.laneCount == 3 && operation.operandTypes[0].laneCount == 3 &&
                 operation.operandTypes[1].laneCount == 1);
            sawFloat32x3LessThan =
                sawFloat32x3LessThan ||
                (operation.key.operation == SLANG_NVVM_VALUE_OP_LESS_THAN &&
                 type.kind == SLANG_NVVM_VALUE_TYPE_BOOL && type.bitWidth == 1 &&
                 type.laneCount == 3 &&
                 operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                 operation.operandTypes[0].laneCount == 3 &&
                 operation.operandTypes[1].laneCount == 3);
            sawBooleanVectorNot = sawBooleanVectorNot ||
                                  (operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_NOT &&
                                   type.kind == SLANG_NVVM_VALUE_TYPE_BOOL && type.bitWidth == 1 &&
                                   type.laneCount == 2 && operation.operandTypes[0].laneCount == 2);
            sawBooleanVectorScalarAnd =
                sawBooleanVectorScalarAnd ||
                (operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_AND &&
                 type.kind == SLANG_NVVM_VALUE_TYPE_BOOL && type.bitWidth == 1 &&
                 type.laneCount == 2 && operation.operandTypes[0].laneCount == 2 &&
                 operation.operandTypes[1].laneCount == 1);
            sawBooleanVectorOr = sawBooleanVectorOr ||
                                 (operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_OR &&
                                  type.kind == SLANG_NVVM_VALUE_TYPE_BOOL && type.bitWidth == 1 &&
                                  type.laneCount == 2 && operation.operandTypes[0].laneCount == 2 &&
                                  operation.operandTypes[1].laneCount == 2);
            sawBooleanVectorEqual = sawBooleanVectorEqual ||
                                    (operation.key.operation == SLANG_NVVM_VALUE_OP_EQUAL &&
                                     type.kind == SLANG_NVVM_VALUE_TYPE_BOOL &&
                                     type.bitWidth == 1 && type.laneCount == 2 &&
                                     operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_BOOL &&
                                     operation.operandTypes[0].laneCount == 2 &&
                                     operation.operandTypes[1].laneCount == 2);
        }
        SLANG_CHECK(sawSignedI32x2RightShift);
        SLANG_CHECK(sawSignedI32x2VectorScalarAdd);
        SLANG_CHECK(sawSignedI32x2ScalarVectorSubtract);
        SLANG_CHECK(sawSignedI8x2Divide);
        SLANG_CHECK(sawSignedI8x2Remainder);
        SLANG_CHECK(sawSignedI8x2LessThan);
        SLANG_CHECK(sawFloat32x3Add);
        SLANG_CHECK(float32ScalarFmodCount == 3);
        SLANG_CHECK(sawFloat32x3VectorScalarAdd);
        SLANG_CHECK(sawFloat32x3LessThan);
        SLANG_CHECK(sawBooleanVectorNot);
        SLANG_CHECK(sawBooleanVectorScalarAnd);
        SLANG_CHECK(sawBooleanVectorOr);
        SLANG_CHECK(sawBooleanVectorEqual);

        bool sawIntegerExtract = false;
        bool sawBooleanExtract = false;
        bool sawFloatExtract = false;
        for (FakeNVVMBuilderScalarTypeKind typeKind : gFakeNVVMBuilder.vectorElementTypeKinds)
        {
            sawIntegerExtract =
                sawIntegerExtract || typeKind == FakeNVVMBuilderScalarTypeKind::Integer;
            sawBooleanExtract =
                sawBooleanExtract || typeKind == FakeNVVMBuilderScalarTypeKind::Boolean;
            sawFloatExtract = sawFloatExtract || typeKind == FakeNVVMBuilderScalarTypeKind::Float;
        }
        SLANG_CHECK(sawIntegerExtract);
        SLANG_CHECK(sawBooleanExtract);
        SLANG_CHECK(sawFloatExtract);
        SLANG_CHECK(gFakeNVVMBuilder.emitVectorConstructCallCount > 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount >= 8);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 10);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFloat64VectorAlgebraUsesTypedDescriptors)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloat64VectorAlgebraSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder state;
            state << "Float64 vector fake trace: result " << int(result) << "; vector types "
                  << gFakeNVVMBuilder.getVectorTypeCallCount << "; operations "
                  << gFakeNVVMBuilder.scalarOperations.getCount() << "; extracts "
                  << gFakeNVVMBuilder.emitSequentialElementExtractCallCount << "; stores "
                  << gFakeNVVMBuilder.emitStoreCallCount << "; modules "
                  << gFakeNVVMBuilder.createModuleCallCount << "; programs "
                  << gFakeNVVM.createProgramCallCount;
            for (const FakeNVVMBuilderScalarOperation& operation :
                 gFakeNVVMBuilder.scalarOperations)
            {
                state << "; op " << operation.key.operation << " type " << operation.resultType.kind
                      << "/" << operation.resultType.bitWidth << "/"
                      << operation.resultType.laneCount;
            }
            getTestReporter()->message(TestMessageType::TestFailure, state.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawAdd = false;
        bool sawSubtract = false;
        bool sawMultiply = false;
        bool sawDivide = false;
        bool sawIntegerToFloat = false;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            const SlangNVVMValueTypeDesc& type = operation.resultType;
            const bool isFloat64x2 = type.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                                     type.bitWidth == 64 && type.laneCount == 2;
            if (!isFloat64x2)
                continue;
            sawAdd |= operation.key.operation == SLANG_NVVM_VALUE_OP_ADD;
            sawSubtract |= operation.key.operation == SLANG_NVVM_VALUE_OP_SUBTRACT;
            sawMultiply |= operation.key.operation == SLANG_NVVM_VALUE_OP_MULTIPLY;
            sawDivide |= operation.key.operation == SLANG_NVVM_VALUE_OP_DIVIDE;
            sawIntegerToFloat |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT &&
                operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER &&
                operation.operandTypes[0].bitWidth == 32 &&
                operation.operandTypes[0].laneCount == 2;
        }
        SLANG_CHECK(sawAdd);
        SLANG_CHECK(sawSubtract);
        SLANG_CHECK(sawMultiply);
        SLANG_CHECK(sawDivide);
        SLANG_CHECK(sawIntegerToFloat);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangTypedSelectUsesGenericValueOperation)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMTypedSelectSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder state;
            state << "typed-select fake trace: result " << int(result) << "; operations "
                  << gFakeNVVMBuilder.scalarOperations.getCount() << "; selects "
                  << gFakeNVVMBuilder
                         .scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Select)]
                  << "; calls " << gFakeNVVMBuilder.emitCallCallCount << "; stores "
                  << gFakeNVVMBuilder.emitStoreCallCount << "; modules "
                  << gFakeNVVMBuilder.createModuleCallCount << "; programs "
                  << gFakeNVVM.createProgramCallCount;
            for (const FakeNVVMBuilderScalarOperation& operation :
                 gFakeNVVMBuilder.scalarOperations)
            {
                state << "; op " << operation.key.operation << " family "
                      << uint32_t(operation.key.family) << " type " << operation.resultType.kind
                      << "/" << operation.resultType.bitWidth << "/"
                      << operation.resultType.laneCount;
            }
            getTestReporter()->message(TestMessageType::TestFailure, state.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawBooleanVectorSelect = false;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            sawBooleanVectorSelect =
                sawBooleanVectorSelect ||
                (operation.key.family == FakeNVVMBuilderScalarFamily::Select &&
                 operation.key.operation == SLANG_NVVM_VALUE_OP_SELECT &&
                 operation.operandCount == 3 &&
                 operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_BOOL &&
                 operation.resultType.bitWidth == 1 && operation.resultType.laneCount == 2 &&
                 operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_BOOL &&
                 operation.operandTypes[0].laneCount == 2 &&
                 NVVMSemantics::areSameType(operation.resultType, operation.operandTypes[1]) &&
                 NVVMSemantics::areSameType(operation.resultType, operation.operandTypes[2]));
        }
        SLANG_CHECK(sawBooleanVectorSelect);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Select)] ==
            1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangScalarShiftDivideRemainderUseTypedOperations)
{
    struct OperationCase
    {
        const char* source;
        SlangNVVMValueOperation operation;
    };
    const OperationCase cases[] = {
        {kDirectNVVMIntegerLeftShiftSource, SLANG_NVVM_VALUE_OP_SHIFT_LEFT},
        {kDirectNVVMIntegerRightShiftSource, SLANG_NVVM_VALUE_OP_SHIFT_RIGHT},
        {kDirectNVVMIntegerDivideSource, SLANG_NVVM_VALUE_OP_DIVIDE},
        {kDirectNVVMIntegerRemainderSource, SLANG_NVVM_VALUE_OP_REMAINDER},
    };

    for (const OperationCase& operationCase : cases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
                globalSession,
                operationCase.source,
                code,
                diagnostics)));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

            bool sawOperation = false;
            for (const FakeNVVMBuilderScalarOperation& operation :
                 gFakeNVVMBuilder.scalarOperations)
            {
                const SlangNVVMValueTypeDesc& type = operation.resultType;
                sawOperation =
                    sawOperation || (operation.key.operation == operationCase.operation &&
                                     type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER &&
                                     type.bitWidth == 32 && type.laneCount == 1);
            }
            SLANG_CHECK(sawOperation);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangDynamicVectorIndexUsesValueHandle)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMDynamicVectorIndexSource,
            code,
            diagnostics);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndexValueRefs.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[0] == UINT32_MAX);
        const FakeNVVMBuilderValueRef indexRef = gFakeNVVMBuilder.vectorElementIndexValueRefs[0];
        SLANG_CHECK(indexRef.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(indexRef.functionIndex == 0);
        SLANG_CHECK(indexRef.index == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangReadOnlyStructuredMatrixMemoryUsesStorageBufferPointer)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMReadOnlyStructuredMatrixMemorySource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawInvariantPhysicalLoad = false;
        for (SlangNVVMLoadFlags flags : gFakeNVVMBuilder.loadFlags)
            sawInvariantPhysicalLoad |= flags == SLANG_NVVM_LOAD_FLAG_INVARIANT;
        SLANG_CHECK(sawInvariantPhysicalLoad);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCUDATypeLayoutQueriesUseDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCUDATypeLayoutSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.scalarReturnValueRefs.getCount() == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 9);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emittedValueOperations.getCount() == 0);

        const int64_t expectedValues[] = {1, 4, 4, 16, 8, 32, 8, 0, 16};
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == SLANG_COUNT_OF(expectedValues));
        for (Index storeIndex = 0; storeIndex < SLANG_COUNT_OF(expectedValues); ++storeIndex)
        {
            const FakeNVVMBuilderValueRef storedValue = gFakeNVVMBuilder.storeValueRefs[storeIndex];
            SLANG_CHECK(storedValue.kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK(storedValue.index >= 0);
            SLANG_CHECK(storedValue.index < gFakeNVVMBuilder.integerConstantValues.getCount());
            SLANG_CHECK(gFakeNVVMBuilder.integerConstantBitWidths[storedValue.index] == 32);
            SLANG_CHECK(
                gFakeNVVMBuilder.integerConstantValues[storedValue.index] ==
                expectedValues[storeIndex]);
        }
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCUDAAggregateLayoutQueriesFoldBeforeDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCUDAAggregateLayoutSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 9);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emittedValueOperations.getCount() == 0);

        const int64_t expectedValues[] = {48, 0, 16, 20, 44, 4, 8, 8, 48};
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == SLANG_COUNT_OF(expectedValues));
        for (Index storeIndex = 0; storeIndex < SLANG_COUNT_OF(expectedValues); ++storeIndex)
        {
            const FakeNVVMBuilderValueRef storedValue = gFakeNVVMBuilder.storeValueRefs[storeIndex];
            SLANG_CHECK(storedValue.kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK(storedValue.index >= 0);
            SLANG_CHECK(storedValue.index < gFakeNVVMBuilder.integerConstantValues.getCount());
            SLANG_CHECK(gFakeNVVMBuilder.integerConstantBitWidths[storedValue.index] == 32);
            SLANG_CHECK(
                gFakeNVVMBuilder.integerConstantValues[storedValue.index] ==
                expectedValues[storeIndex]);
        }
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangConventionalComputeUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMConventionalComputeSource,
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
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getStructTypeCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldTypes[0] == _getFakeNVVMBuilderResourceViewType());
        SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageValueType == _getFakeNVVMBuilderStructType());
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageLinkage == SLANG_NVVM_LINKAGE_EXTERNAL);
        SLANG_CHECK(
            gFakeNVVMBuilder.globalStorageAddressSpace == SLANG_NVVM_ADDRESS_SPACE_CONSTANT);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageAlignment == 8);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageNames.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageNames[0] == "SLANG_globalParams");

        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 9);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 3);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarOperations[0].key.operation == SLANG_NVVM_VALUE_OP_MULTIPLY);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations[1].key.operation == SLANG_NVVM_VALUE_OP_ADD);
        for (Index i = 0; i < 2; ++i)
        {
            SLANG_CHECK(NVVMSemantics::areSameType(
                gFakeNVVMBuilder.scalarOperations[i].resultType,
                NVVMSemantics::kUnsignedI32x3));
        }
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarOperations[2].key.operation ==
            SLANG_NVVM_VALUE_OP_INTEGER_CONVERT);
        SLANG_CHECK(NVVMSemantics::areSameType(
            gFakeNVVMBuilder.scalarOperations[2].resultType,
            NVVMSemantics::kSignedI32));
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 1);

        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldPointerBaseValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::GlobalStorage);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldPointerIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignment == 8);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::ResourceView);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.aggregateElementIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 10);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangConventionalScalarParameterBlockUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMConventionalScalarParameterBlockSource,
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
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getStructTypeCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.structFieldTypes.getCount() == 3);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldTypes[1] == _getFakeNVVMBuilderScalarStructPointerType());
        SLANG_CHECK(gFakeNVVMBuilder.structFieldTypes[2] == _getFakeNVVMBuilderResourceViewType());
        SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageValueType == _getFakeNVVMBuilderStructType());
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageAlignment == 8);

        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 4);
        const uint32_t expectedFieldIndices[] = {2, 0, 1, 0};
        for (Index i = 0; i < SLANG_COUNT_OF(expectedFieldIndices); ++i)
            SLANG_CHECK(gFakeNVVMBuilder.structFieldPointerIndices[i] == expectedFieldIndices[i]);
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldPointerBaseValueRefs[3].kind ==
            FakeNVVMBuilderValueKind::Load);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 9);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[7] ==
            FakeNVVMBuilderScalarTypeKind::ScalarStructPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[8] == FakeNVVMBuilderScalarTypeKind::Integer);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags.getCount() == 9);
        for (SlangNVVMLoadFlags flags : gFakeNVVMBuilder.loadFlags)
            SLANG_CHECK(flags == SLANG_NVVM_LOAD_FLAG_INVARIANT);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 6);

        const int64_t expectedLayoutValues[] = {0, 8, 16, 8};
        for (Index storeIndex = 0; storeIndex < SLANG_COUNT_OF(expectedLayoutValues); ++storeIndex)
        {
            const FakeNVVMBuilderValueRef storedValue = gFakeNVVMBuilder.storeValueRefs[storeIndex];
            SLANG_CHECK(storedValue.kind == FakeNVVMBuilderValueKind::ScalarOperation);
            SLANG_CHECK(storedValue.index >= 0);
            SLANG_CHECK(storedValue.index < gFakeNVVMBuilder.scalarOperations.getCount());
            const FakeNVVMBuilderScalarOperation& conversion =
                gFakeNVVMBuilder.scalarOperations[storedValue.index];
            SLANG_CHECK(conversion.key.operation == SLANG_NVVM_VALUE_OP_INTEGER_CONVERT);
            SLANG_CHECK(conversion.operandCount == 1);
            SLANG_CHECK(conversion.operands[0].kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK(
                gFakeNVVMBuilder.integerConstantValues[conversion.operands[0].index] ==
                expectedLayoutValues[storeIndex]);
        }
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[4].kind == FakeNVVMBuilderValueKind::Load);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[5].kind == FakeNVVMBuilderValueKind::Load);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangConventionalScalarConstantBufferUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMConventionalScalarConstantBufferSource,
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
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getStructTypeCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes[1] == _getFakeNVVMBuilderFloatType());
        SLANG_CHECK(gFakeNVVMBuilder.structFieldTypes.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldTypes[0] == _getFakeNVVMBuilderScalarStructPointerType());
        SLANG_CHECK(gFakeNVVMBuilder.structFieldTypes[1] == _getFakeNVVMBuilderResourceViewType());
        SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageValueType == _getFakeNVVMBuilderStructType());

        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 3);
        const uint32_t expectedFieldIndices[] = {1, 0, 0};
        for (Index i = 0; i < SLANG_COUNT_OF(expectedFieldIndices); ++i)
            SLANG_CHECK(gFakeNVVMBuilder.structFieldPointerIndices[i] == expectedFieldIndices[i]);
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldPointerBaseValueRefs[2].kind ==
            FakeNVVMBuilderValueKind::Load);

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags.getCount() == 3);
        for (SlangNVVMLoadFlags flags : gFakeNVVMBuilder.loadFlags)
            SLANG_CHECK(flags == SLANG_NVVM_LOAD_FLAG_INVARIANT);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[1] ==
            FakeNVVMBuilderScalarTypeKind::ScalarStructPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[2] == FakeNVVMBuilderScalarTypeKind::Integer);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Load);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangLoadedParameterGroupValueUsesExactImmutablePointer)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMLoadedParameterGroupValueSource,
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
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawLoadedParameterGroupValue = false;
        for (Index loadIndex = 0; loadIndex < gFakeNVVMBuilder.loadResultTypeKinds.getCount();
             ++loadIndex)
        {
            if (gFakeNVVMBuilder.loadResultTypeKinds[loadIndex] ==
                FakeNVVMBuilderScalarTypeKind::ScalarStruct)
            {
                sawLoadedParameterGroupValue = true;
                SLANG_CHECK(
                    gFakeNVVMBuilder.loadFlags[loadIndex] == SLANG_NVVM_LOAD_FLAG_INVARIANT);
            }
        }
        SLANG_CHECK(sawLoadedParameterGroupValue);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMUnsupportedLoadedCompactParameterGroupValueSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        const String diagnosticText = _getBlobText(diagnostics);
        SLANG_CHECK(diagnosticText.indexOf("E52017") >= 0);
        SLANG_CHECK(diagnosticText.indexOf("loaded parameter-group value representation") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCompactParameterGroupVectorsUseDistinctStorageRepresentation)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCompactParameterGroupVectorSource,
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
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementType == _getFakeNVVMBuilderFloatType());
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes.getCount() == 2);
        for (auto fieldType : gFakeNVVMBuilder.scalarStructFieldTypes)
            SLANG_CHECK(fieldType == _getFakeNVVMBuilderArrayType());

        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount >= 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitVectorConstructCallCount >= 1);
        bool sawCompactVectorLoad = false;
        for (Index i = 0; i < gFakeNVVMBuilder.loadResultTypeKinds.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.loadResultTypeKinds[i] ==
                FakeNVVMBuilderScalarTypeKind::NumericArray)
            {
                sawCompactVectorLoad = true;
                SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[i] == 4);
                SLANG_CHECK(gFakeNVVMBuilder.loadFlags[i] == SLANG_NVVM_LOAD_FLAG_INVARIANT);
            }
        }
        SLANG_CHECK(sawCompactVectorLoad);
        bool sawValueVectorParameter = false;
        for (auto parameterType : gFakeNVVMBuilder.functionParameterTypeKinds)
        {
            sawValueVectorParameter |=
                parameterType == FakeNVVMBuilderParameterTypeKind::ValueVector;
        }
        SLANG_CHECK(sawValueVectorParameter);
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount >= 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFloat64ValueFamilyUsesGenericTypedOperations)
{
#if !(SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY)
    SLANG_IGNORE_TEST;
    return;
#endif
    _resetDirectNVVMFakes();
    TempDirectory toolkit;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(_configureFakeDirectNVVMLibdevice(globalSession, toolkit)));

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloat64ValueFamilySource,
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
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawFloat64Type = false;
        for (uint32_t bitWidth : gFakeNVVMBuilder.floatingPointTypeBitWidths)
            sawFloat64Type |= bitWidth == 64;
        SLANG_CHECK(sawFloat64Type);

        SLANG_CHECK(
            gFakeNVVMBuilder.floatingPointConstantBitWidths.getCount() ==
            gFakeNVVMBuilder.floatingPointConstantBitPatterns.getCount());
        bool sawFloat64Constant = false;
        bool sawExactThreeConstant = false;
        for (Index i = 0; i < gFakeNVVMBuilder.floatingPointConstantBitWidths.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.floatingPointConstantBitWidths[i] == 64)
            {
                sawFloat64Constant = true;
                sawExactThreeConstant |= gFakeNVVMBuilder.floatingPointConstantBitPatterns[i] ==
                                         UINT64_C(0x4008000000000000);
            }
        }
        SLANG_CHECK(sawFloat64Constant);
        SLANG_CHECK(sawExactThreeConstant);

        bool sawDoubleParameter = false;
        for (auto parameterKind : gFakeNVVMBuilder.functionParameterTypeKinds)
            sawDoubleParameter |= parameterKind == FakeNVVMBuilderParameterTypeKind::Double;
        SLANG_CHECK(sawDoubleParameter);

        bool sawDoubleResult = false;
        for (auto resultKind : gFakeNVVMBuilder.functionTypeResultKinds)
            sawDoubleResult |= resultKind == FakeNVVMBuilderResultTypeKind::Double;
        SLANG_CHECK(sawDoubleResult);

        bool sawFloat64Unary = false;
        bool sawFloat64Binary = false;
        bool sawFloat64Compare = false;
        bool sawIntegerToFloat64 = false;
        bool sawFloat64ToInteger = false;
        bool sawFloat64WidthConversion = false;
        bool sawFloat64Select = false;
        bool sawFloat64BitReinterpret = false;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            const bool hasFloat64Result =
                operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                operation.resultType.bitWidth == 64 && operation.resultType.laneCount == 1;
            const bool hasFloat64Operand =
                operation.operandCount > 0 &&
                operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                operation.operandTypes[0].bitWidth == 64 &&
                operation.operandTypes[0].laneCount == 1;
            sawFloat64Unary |= operation.key.family == FakeNVVMBuilderScalarFamily::FloatingUnary &&
                               operation.key.operation == SLANG_NVVM_VALUE_OP_NEGATE &&
                               hasFloat64Result;
            sawFloat64Binary |=
                operation.key.family == FakeNVVMBuilderScalarFamily::FloatingBinary &&
                hasFloat64Result;
            sawFloat64Compare |=
                operation.key.family == FakeNVVMBuilderScalarFamily::FloatingCompare &&
                hasFloat64Operand;
            sawIntegerToFloat64 |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT && hasFloat64Result;
            sawFloat64ToInteger |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER &&
                hasFloat64Operand;
            sawFloat64WidthConversion |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT &&
                (hasFloat64Result || hasFloat64Operand);
            sawFloat64Select |=
                operation.key.family == FakeNVVMBuilderScalarFamily::Select && hasFloat64Result;
            sawFloat64BitReinterpret |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET &&
                (hasFloat64Result || hasFloat64Operand);
        }
        SLANG_CHECK(sawFloat64Unary);
        SLANG_CHECK(sawFloat64Binary);
        SLANG_CHECK(sawFloat64Compare);
        SLANG_CHECK(sawIntegerToFloat64);
        SLANG_CHECK(sawFloat64ToInteger);
        SLANG_CHECK(sawFloat64WidthConversion);
        SLANG_CHECK(sawFloat64Select);
        SLANG_CHECK(sawFloat64BitReinterpret);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangConventionalSamplerStorageUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMConventionalSamplerStorageSource,
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
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        // The CUDA collector moves the unsized array after all fixed-size fields. The provider
        // therefore sees sampler storage, the used float resource, and the pointer-plus-count
        // array in exact CUDA ABI order.
        SLANG_CHECK(gFakeNVVMBuilder.getStructTypeCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldTypes.getCount() == 3);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldTypes[1] ==
            _getFakeNVVMBuilderResourceViewType(FakeNVVMBuilderScalarTypeKind::Float));
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldTypes[2] ==
            _getFakeNVVMBuilderResourceViewType(FakeNVVMBuilderScalarTypeKind::Integer));
        SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageAlignment == 8);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageNames[0] == "SLANG_globalParams");

        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldPointerIndices[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::ResourceView);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangMultidimensionalWaveUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMMultidimensionalWaveSource,
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
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getStructTypeCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes[0] == _getFakeNVVMBuilderFloatType());
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldTypes[0] ==
            _getFakeNVVMBuilderResourceViewType(FakeNVVMBuilderScalarTypeKind::Float));
        // SV_GroupIndex is (tid.z * ntid.y + tid.y) * ntid.x + tid.x. Match this
        // complete producer-to-consumer chain even when vector extracts fold to scalar reads.
        auto index = _findFakeNVVMScalarBinary(
            SLANG_NVVM_VALUE_OP_MULTIPLY,
            _findFakeNVVMNamedIntrinsicCall("llvm.nvvm.read.ptx.sreg.tid.z"),
            _findFakeNVVMNamedIntrinsicCall("llvm.nvvm.read.ptx.sreg.ntid.y"));
        index = _findFakeNVVMScalarBinary(
            SLANG_NVVM_VALUE_OP_ADD,
            index,
            _findFakeNVVMNamedIntrinsicCall("llvm.nvvm.read.ptx.sreg.tid.y"));
        index = _findFakeNVVMScalarBinary(
            SLANG_NVVM_VALUE_OP_MULTIPLY,
            index,
            _findFakeNVVMNamedIntrinsicCall("llvm.nvvm.read.ptx.sreg.ntid.x"));
        index = _findFakeNVVMScalarBinary(
            SLANG_NVVM_VALUE_OP_ADD,
            index,
            _findFakeNVVMNamedIntrinsicCall("llvm.nvvm.read.ptx.sreg.tid.x"));
        SLANG_CHECK(index.kind == FakeNVVMBuilderValueKind::ScalarOperation);

        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.aggregateElementTypeKinds.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.aggregateElementTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::Float);
        SLANG_CHECK(
            gFakeNVVMBuilder.aggregateElementTypeKinds[1] == FakeNVVMBuilderScalarTypeKind::Float);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangSharedMemoryUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMSharedMemorySource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementType == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 64);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageValueType == _getFakeNVVMBuilderArrayType());
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageAddressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageAlignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageNames.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageNames[0].indexOf("sharedValues") >= 0);

        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 2);
        for (Index i = 0; i < 2; ++i)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.sequentialElementPointerBaseValueRefs[i].kind ==
                FakeNVVMBuilderValueKind::GlobalStorage);
            SLANG_CHECK(gFakeNVVMBuilder.sequentialElementPointerBaseValueRefs[i].index == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitAtomicOperationCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.workgroupBarrierCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::SequentialElementPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::SequentialElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangUnsignedSharedArrayIndexUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMUnsignedSharedArrayIndexSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageAddressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.sequentialElementPointerIndexValueRefs.getCount() == 2);
        for (Index elementIndex = 0; elementIndex < 2; ++elementIndex)
        {
            const FakeNVVMBuilderValueRef index =
                gFakeNVVMBuilder.sequentialElementPointerIndexValueRefs[elementIndex];
            SLANG_CHECK(index.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(index.functionIndex == 0);
            SLANG_CHECK(index.index == size_t(elementIndex + 1));
        }
        SLANG_CHECK(gFakeNVVMBuilder.workgroupBarrierCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 2);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangWaveLaneCountUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMWaveLaneCountSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeResultKinds.getCount() == 3);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[0] == FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[1] == FakeNVVMBuilderResultTypeKind::Integer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[2] == FakeNVVMBuilderResultTypeKind::Integer);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[1] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[2] == 0);

        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicNames[0] == "llvm.nvvm.read.ptx.sreg.laneid");
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicNames[1] == "llvm.nvvm.read.ptx.sreg.warpsize");
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicCallerBlockIndices[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicCallerBlockIndices[1] == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.scalarReturnValueRefs.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarReturnValueRefs[0].kind == FakeNVVMBuilderValueKind::Intrinsic);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarReturnValueRefs[1].kind == FakeNVVMBuilderValueKind::Intrinsic);

        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.callCalleeFunctionIndices[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.callCalleeFunctionIndices[1] == 2);
        SLANG_CHECK(gFakeNVVMBuilder.callCallerBlockIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.callCallerBlockIndices[1] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}


SLANG_UNIT_TEST(nvvmSlangWaveActiveMaskUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMWaveActiveMaskSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations.getCount() == 2);
        Index ballotIntrinsicIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT)
            {
                ballotIntrinsicIndex = i;
            }
        }
        SLANG_CHECK_ABORT(ballotIntrinsicIndex >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[ballotIntrinsicIndex] == 2);
        const Index ballotArgumentOffset =
            gFakeNVVMBuilder.intrinsicArgumentOffsets[ballotIntrinsicIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.intrinsicArgumentValueRefs[ballotArgumentOffset + 0].kind ==
            FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(
            gFakeNVVMBuilder.intrinsicArgumentValueRefs[ballotArgumentOffset + 1].kind ==
            FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == -1);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[1] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantBitWidths[0] == 32);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantBitWidths[1] == 1);

        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 2);
        Index activeMaskCallIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.callArgumentCounts.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.callArgumentCounts[i] == 1)
                activeMaskCallIndex = i;
        }
        SLANG_CHECK_ABORT(activeMaskCallIndex >= 0);
        const Index callArgumentOffset = gFakeNVVMBuilder.callArgumentOffsets[activeMaskCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset].kind ==
            FakeNVVMBuilderValueKind::Intrinsic);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// Compiles one public scalar wave-read fixture and checks its canonical mask-to-operation topology.


SLANG_UNIT_TEST(nvvmSlangWaveMaskMatchUsesTypedIntrinsic)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMWaveMaskMatchSwitchSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        Index matchIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH)
                matchIndex = i;
        }
        SLANG_CHECK_ABORT(matchIndex >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[matchIndex] == 2);
        const SlangNVVMValueTypeDesc& resultType =
            gFakeNVVMBuilder.intrinsicResultTypes[matchIndex];
        SLANG_CHECK(resultType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER);
        SLANG_CHECK(resultType.bitWidth == 32);
        SLANG_CHECK(resultType.laneCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFloat32CopyUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloat32CopySource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.floatingPointBitWidth == 32);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes[0] == _getFakeNVVMBuilderFloatType());
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterTypeKinds.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[0] ==
            FakeNVVMBuilderParameterTypeKind::FloatPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[1] ==
            FakeNVVMBuilderParameterTypeKind::FloatPointer);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags[0] == SLANG_NVVM_LOAD_FLAG_NONE);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs[0].index == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadResultTypeKinds.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::Float);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Load);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
        SLANG_CHECK(
            gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(
                FakeNVVMBuilderScalarFamily::FloatingBinary)] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(
                FakeNVVMBuilderScalarFamily::FloatingUnary)] == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFloat16ValuesUseGenericTypedPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloat16ValueSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder state;
            state << "Float16 fake state: modules=" << gFakeNVVMBuilder.createModuleCallCount
                  << ", functions=" << gFakeNVVMBuilder.declareFunctionCallCount
                  << ", operations=" << gFakeNVVMBuilder.scalarOperations.getCount()
                  << ", vector-constructs=" << gFakeNVVMBuilder.emitVectorConstructCallCount
                  << ", sequential-extracts="
                  << gFakeNVVMBuilder.emitSequentialElementExtractCallCount
                  << ", calls=" << gFakeNVVMBuilder.emitCallCallCount
                  << ", phis=" << gFakeNVVMBuilder.emitPhiCallCount;
            getTestReporter()->message(TestMessageType::Info, state.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        const SlangNVVMTypeHandle halfType = _getFakeNVVMBuilderHalfType();
        const SlangNVVMTypeHandle half2Type =
            _getFakeNVVMBuilderVectorType(2, FakeNVVMBuilderScalarTypeKind::Half);
        const SlangNVVMTypeHandle physicalHalf2Type =
            _getFakeNVVMBuilderVectorType(2, FakeNVVMBuilderScalarTypeKind::Integer);
        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.getVectorTypeCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.floatingPointConstantBitWidths.getCount() >= 2);
        for (uint32_t bitWidth : gFakeNVVMBuilder.floatingPointConstantBitWidths)
            SLANG_CHECK(bitWidth == 16 || bitWidth == 32);

        Index chooseFunction = -1;
        Index adjustFunction = -1;
        for (Index functionIndex = 0; functionIndex < gFakeNVVMBuilder.functionNames.getCount();
             ++functionIndex)
        {
            const String& name = gFakeNVVMBuilder.functionNames[functionIndex];
            if (name.indexOf("chooseHalf2") >= 0)
                chooseFunction = functionIndex;
            else if (name.indexOf("adjustHalf2") >= 0)
                adjustFunction = functionIndex;
        }
        SLANG_CHECK_ABORT(chooseFunction >= 0);
        SLANG_CHECK_ABORT(adjustFunction >= 0);
        const Index chooseType = gFakeNVVMBuilder.functionTypeIndices[chooseFunction];
        const Index adjustType = gFakeNVVMBuilder.functionTypeIndices[adjustFunction];
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeResultTypes[chooseType] == physicalHalf2Type);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeResultTypes[adjustType] == physicalHalf2Type);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[chooseType] == 3);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[adjustType] == 1);
        const Index chooseParameterOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[chooseType];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[chooseParameterOffset] == physicalHalf2Type);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[chooseParameterOffset + 1] ==
            physicalHalf2Type);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[chooseParameterOffset + 2] ==
            _getFakeNVVMBuilderBooleanType());
        const Index adjustParameterOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[adjustType];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[adjustParameterOffset] == physicalHalf2Type);

        bool sawScalarFloatToHalf = false;
        bool sawScalarIntegerToHalf = false;
        bool sawVectorFloatToHalf = false;
        bool sawHalfToFloat = false;
        bool sawHalfToInteger = false;
        bool sawHalfAdd = false;
        bool sawHalfNegate = false;
        bool sawHalfCompare = false;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            const SlangNVVMValueTypeDesc& resultType = operation.resultType;
            const SlangNVVMValueTypeDesc& operandType = operation.operandTypes[0];
            if (operation.key.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT &&
                resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                resultType.bitWidth == 16)
            {
                sawScalarFloatToHalf |= resultType.laneCount == 1 && operandType.bitWidth == 32;
                sawVectorFloatToHalf |= resultType.laneCount == 2 && operandType.laneCount == 2 &&
                                        operandType.bitWidth == 32;
            }
            if (operation.key.operation == SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT &&
                resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                resultType.bitWidth == 16 && resultType.laneCount == 1)
            {
                sawScalarIntegerToHalf = true;
            }
            if (operation.key.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT &&
                resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                resultType.bitWidth == 32 && operandType.bitWidth == 16)
            {
                sawHalfToFloat = true;
            }
            if (operation.key.operation == SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER &&
                operandType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                operandType.bitWidth == 16)
            {
                sawHalfToInteger = true;
            }
            sawHalfAdd |= operation.key.operation == SLANG_NVVM_VALUE_OP_ADD &&
                          resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                          resultType.bitWidth == 16;
            sawHalfNegate |= operation.key.operation == SLANG_NVVM_VALUE_OP_NEGATE &&
                             resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                             resultType.bitWidth == 16;
            sawHalfCompare |= operation.key.operation == SLANG_NVVM_VALUE_OP_LESS_THAN &&
                              operandType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                              operandType.bitWidth == 16;
        }
        SLANG_CHECK(sawScalarFloatToHalf);
        SLANG_CHECK(sawScalarIntegerToHalf);
        SLANG_CHECK(sawVectorFloatToHalf);
        SLANG_CHECK(sawHalfToFloat);
        SLANG_CHECK(sawHalfToInteger);
        SLANG_CHECK(sawHalfAdd);
        SLANG_CHECK(sawHalfNegate);
        SLANG_CHECK(sawHalfCompare);

        bool sawHalf2Phi = false;
        for (SlangNVVMTypeHandle phiType : gFakeNVVMBuilder.scalarPhiTypes)
            sawHalf2Phi |= phiType == half2Type;
        SLANG_CHECK(sawHalf2Phi);
        bool sawPhysicalHalf2Call = false;
        for (SlangNVVMTypeHandle callType : gFakeNVVMBuilder.callResultTypes)
            sawPhysicalHalf2Call |= callType == physicalHalf2Type;
        SLANG_CHECK(sawPhysicalHalf2Call);
        bool sawHalfElement = false;
        for (FakeNVVMBuilderScalarTypeKind elementKind : gFakeNVVMBuilder.vectorElementTypeKinds)
        {
            sawHalfElement |= elementKind == FakeNVVMBuilderScalarTypeKind::Half;
        }
        SLANG_CHECK(sawHalfElement);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(halfType != nullptr);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// Each helper deliberately performs native Half arithmetic between the two physical crossings.
// The runtime input and all consumed lanes keep the parameter/result boundaries observable.
static const char kHalfVectorBoundarySource[] = R"(
[noinline] half2 boundary2(half2 value) { return -value; }
[noinline] half3 boundary3(half3 value) { return -value; }
[noinline] half4 boundary4(half4 value) { return -value; }

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input)
{
    half4 value = half4(input, input + 1.0, input + 2.0, input + 3.0);
    half2 a = boundary2(value.xy);
    half3 b = boundary3(value.xyz);
    half4 c = boundary4(value);
    *destination = float(a.x) + float(a.y) + float(b.x) + float(b.y) + float(b.z) +
        float(c.x) + float(c.y) + float(c.z) + float(c.w);
}
)";

// All three widths belong to one signature: preflight must not stop at the first Half boundary.
static const char kMixedHalfVectorBoundarySource[] = R"(
[noinline]
half4 mixedBoundary(half2 a, half3 b, half4 c)
{
    return half4(a.x + a.y, b.x + b.y + b.z, c.x + c.y, c.z + c.w);
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input)
{
    half4 value = half4(input, input + 1.0, input + 2.0, input + 3.0);
    half4 result = mixedBoundary(value.xy, value.xyz, value);
    *destination = float(result.x) + float(result.y) + float(result.z) + float(result.w);
}
)";

SLANG_UNIT_TEST(nvvmSlangHalfVectorsCrossAllFourHelperBoundaries)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const auto result = _compileSlangWithDirectNVVM(
            globalSession,
            kHalfVectorBoundarySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        const char* names[] = {"boundary2", "boundary3", "boundary4"};
        for (uint32_t width = 2; width <= 4; ++width)
        {
            Index function = -1;
            for (Index i = 0; i < gFakeNVVMBuilder.functionNames.getCount(); ++i)
            {
                if (gFakeNVVMBuilder.functionNames[i].indexOf(names[width - 2]) >= 0)
                {
                    SLANG_CHECK(function == -1);
                    function = i;
                }
            }
            SLANG_CHECK_ABORT(function >= 0);
            const auto physical =
                _getFakeNVVMBuilderVectorType(width, FakeNVVMBuilderScalarTypeKind::Integer);
            const Index functionType = gFakeNVVMBuilder.functionTypeIndices[function];
            SLANG_CHECK(gFakeNVVMBuilder.functionTypeResultTypes[functionType] == physical);
            SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionType] == 1);
            const Index parameterOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionType];
            SLANG_CHECK(gFakeNVVMBuilder.functionParameterTypes[parameterOffset] == physical);

            Index call = -1;
            for (Index i = 0; i < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount(); ++i)
            {
                if (gFakeNVVMBuilder.callCalleeFunctionIndices[i] == function)
                {
                    SLANG_CHECK(call == -1);
                    call = i;
                }
            }
            SLANG_CHECK_ABORT(call >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.callResultTypes[call] == physical);
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.callArgumentCounts[call] == 1);
            const auto argument =
                gFakeNVVMBuilder.callArgumentValueRefs[gFakeNVVMBuilder.callArgumentOffsets[call]];
            SLANG_CHECK_ABORT(argument.kind == FakeNVVMBuilderValueKind::ScalarOperation);
            const auto& encoding = gFakeNVVMBuilder.scalarOperations[argument.index];
            SLANG_CHECK(_isHalfVectorBoundaryBitcast(encoding, width, true));
            SLANG_CHECK(encoding.callerBlockIndex == gFakeNVVMBuilder.callCallerBlockIndices[call]);

            Index entryDecode = -1;
            Index resultDecode = -1;
            for (Index i = 0; i < gFakeNVVMBuilder.scalarOperations.getCount(); ++i)
            {
                const auto& operation = gFakeNVVMBuilder.scalarOperations[i];
                if (!_isHalfVectorBoundaryBitcast(operation, width, false))
                    continue;
                const auto operand = operation.operands[0];
                if (operand.kind == FakeNVVMBuilderValueKind::Parameter &&
                    operand.functionIndex == function && operand.index == 0)
                {
                    SLANG_CHECK(entryDecode == -1);
                    entryDecode = i;
                    SLANG_CHECK(
                        gFakeNVVMBuilder.blockFunctionIndices[operation.callerBlockIndex] ==
                        function);
                }
                if (operand.kind == FakeNVVMBuilderValueKind::Call && operand.index == call)
                {
                    SLANG_CHECK(resultDecode == -1);
                    resultDecode = i;
                    SLANG_CHECK(
                        operation.callerBlockIndex ==
                        gFakeNVVMBuilder.callCallerBlockIndices[call]);
                }
            }
            SLANG_CHECK_ABORT(entryDecode >= 0);
            SLANG_CHECK_ABORT(resultDecode >= 0);

            Index returnCount = 0;
            for (Index i = 0; i < gFakeNVVMBuilder.scalarReturnValueRefs.getCount(); ++i)
            {
                if (gFakeNVVMBuilder
                        .blockFunctionIndices[gFakeNVVMBuilder.scalarReturnBlockIndices[i]] !=
                    function)
                    continue;
                ++returnCount;
                const auto returned = gFakeNVVMBuilder.scalarReturnValueRefs[i];
                SLANG_CHECK_ABORT(returned.kind == FakeNVVMBuilderValueKind::ScalarOperation);
                const auto& returnEncoding = gFakeNVVMBuilder.scalarOperations[returned.index];
                SLANG_CHECK(_isHalfVectorBoundaryBitcast(returnEncoding, width, true));
                SLANG_CHECK(
                    returnEncoding.callerBlockIndex ==
                    gFakeNVVMBuilder.scalarReturnBlockIndices[i]);
                const auto bodyValue = returnEncoding.operands[0];
                SLANG_CHECK_ABORT(bodyValue.kind == FakeNVVMBuilderValueKind::ScalarOperation);
                const auto& negate = gFakeNVVMBuilder.scalarOperations[bodyValue.index];
                SLANG_CHECK(negate.key.operation == SLANG_NVVM_VALUE_OP_NEGATE);
                const SlangNVVMValueTypeDesc nativeHalf = {
                    SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
                    16,
                    width};
                SLANG_CHECK(NVVMSemantics::areSameType(negate.resultType, nativeHalf));
                SLANG_CHECK_ABORT(negate.operandCount == 1);
                SLANG_CHECK(NVVMSemantics::areSameType(negate.operandTypes[0], nativeHalf));
                SLANG_CHECK(negate.operands[0].kind == FakeNVVMBuilderValueKind::ScalarOperation);
                SLANG_CHECK(negate.operands[0].index == entryDecode);
            }
            SLANG_CHECK(returnCount == 1);
            // The caller consumes the decoded canonical result, not the physical call value.
            uint32_t observedLanes = 0;
            for (Index i = 0; i < gFakeNVVMBuilder.vectorElementBaseValueRefs.getCount(); ++i)
            {
                const auto base = gFakeNVVMBuilder.vectorElementBaseValueRefs[i];
                if (base.kind == FakeNVVMBuilderValueKind::ScalarOperation &&
                    base.index == resultDecode)
                {
                    SLANG_CHECK(
                        gFakeNVVMBuilder.vectorElementTypeKinds[i] ==
                        FakeNVVMBuilderScalarTypeKind::Half);
                    const auto lane = gFakeNVVMBuilder.vectorElementIndices[i];
                    SLANG_CHECK_ABORT(lane < width);
                    observedLanes |= 1u << lane;
                }
            }
            SLANG_CHECK(observedLanes == (1u << width) - 1u);
        }
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangHalfVectorBoundaryCapabilitiesPreflightEveryWidth)
{
    for (uint32_t width = 2; width <= 4; ++width)
    {
        for (bool encode : {false, true})
        {
            const SlangNVVMValueTypeDesc half = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 16, width};
            const SlangNVVMValueTypeDesc integer = {
                SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
                16,
                width};
            const SlangNVVMValueTypeDesc operand = encode ? half : integer;
            const SlangNVVMValueOperationDesc rejected =
                {SLANG_NVVM_VALUE_OP_BIT_REINTERPRET, encode ? integer : half, &operand, 1};
            _resetDirectNVVMFakes();
            _rejectFakeNVVMBuilderValueOperation(rejected);
            {
                ComPtr<slang::IGlobalSession> globalSession;
                SLANG_CHECK_ABORT(
                    slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) ==
                    SLANG_OK);
                ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
                globalSession->setSharedLibraryLoader(loader);
                ComPtr<slang::IBlob> code;
                ComPtr<slang::IBlob> diagnostics;
                SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
                    globalSession,
                    kMixedHalfVectorBoundarySource,
                    code,
                    diagnostics)));
                SLANG_CHECK(code == nullptr);
                const String text = _getBlobText(diagnostics);
                SLANG_CHECK(text.indexOf("E52018") >= 0);
                SLANG_CHECK(
                    text.indexOf(
                        encode ? "physical Half helper ABI encoding"
                               : "canonical Half helper ABI decoding") >= 0);
                SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
                SLANG_CHECK(gFakeNVVMBuilder.isOperationSupportedCallCount > 0);
                SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
                SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
            }
            SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
            SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangOpaqueHalfHelpersUseTypedFloatConversions)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMOpaqueHalfConversionSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        Index floatConvertCount = 0;
        bool sawFloatToHalf = false;
        bool sawHalfToFloat = false;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            if (operation.key.operation != SLANG_NVVM_VALUE_OP_FLOAT_CONVERT)
                continue;

            ++floatConvertCount;
            SLANG_CHECK(operation.key.family == FakeNVVMBuilderScalarFamily::FloatingUnary);
            SLANG_CHECK(operation.operandCount == 1);
            const SlangNVVMValueTypeDesc& resultType = operation.resultType;
            const SlangNVVMValueTypeDesc& operandType = operation.operandTypes[0];
            sawFloatToHalf |= resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                              resultType.bitWidth == 16 && resultType.laneCount == 1 &&
                              operandType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                              operandType.bitWidth == 32 && operandType.laneCount == 1;
            sawHalfToFloat |= resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                              resultType.bitWidth == 32 && resultType.laneCount == 1 &&
                              operandType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                              operandType.bitWidth == 16 && operandType.laneCount == 1;
        }
        SLANG_CHECK(floatConvertCount == 2);
        SLANG_CHECK(sawFloatToHalf);
        SLANG_CHECK(sawHalfToFloat);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// Checks vector API roles by their types, independently of generated helper names and order.
SLANG_UNIT_TEST(nvvmSlangHalfConversionAPIsPreserveVectorLanes)
{
    static const char source[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> output,
    uniform float x, uniform float y, uniform float z, uniform float w)
{
    float2 pair = f16tof32(f32tof16_(float2(x, y)));
    float3 triple = f16tof32(f32tof16_(float3(x, y, z)));
    float4 quad = f16tof32(f32tof16_(float4(x, y, z, w)));
    output[0] = pair.x;
    output[1] = pair.y;
    output[2] = triple.x;
    output[3] = triple.y;
    output[4] = triple.z;
    output[5] = quad.x;
    output[6] = quad.y;
    output[7] = quad.z;
    output[8] = quad.w;
}
)";
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result =
            _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        // A role is (lane count, direction), not a position in emitted operation order.
        uint32_t narrowWidths = 0;
        uint32_t widenWidths = 0;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            if (operation.key.operation != SLANG_NVVM_VALUE_OP_FLOAT_CONVERT)
                continue;
            SLANG_CHECK(operation.key.family == FakeNVVMBuilderScalarFamily::FloatingUnary);
            SLANG_CHECK_ABORT(operation.operandCount == 1);
            const auto& input = operation.operandTypes[0];
            const auto& output = operation.resultType;
            SLANG_CHECK(input.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT);
            SLANG_CHECK(output.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT);
            SLANG_CHECK_ABORT(input.laneCount >= 2 && input.laneCount <= 4);
            SLANG_CHECK(output.laneCount == input.laneCount);
            if (input.bitWidth == 32)
            {
                SLANG_CHECK(output.bitWidth == 16);
                narrowWidths |= 1u << input.laneCount;
            }
            else
            {
                SLANG_CHECK(input.bitWidth == 16);
                SLANG_CHECK(output.bitWidth == 32);
                widenWidths |= 1u << input.laneCount;
            }
        }
        SLANG_CHECK(narrowWidths == ((1u << 2) | (1u << 3) | (1u << 4)));
        SLANG_CHECK(widenWidths == narrowWidths);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangLocalVectorSwizzlePromotesToGenericValues)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMLocalVectorSwizzleSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        // This source has no surviving local allocation. These calls prove its pure lane update
        // was flattened through the generic vector value path.
        SLANG_CHECK(gFakeNVVMBuilder.emitVectorConstructCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount >= 7);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        for (FakeNVVMBuilderScalarTypeKind elementKind : gFakeNVVMBuilder.vectorElementTypeKinds)
        {
            SLANG_CHECK(elementKind == FakeNVVMBuilderScalarTypeKind::Half);
        }
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangStatefulAggregateHelpersUseGenericLocalPointers)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMStatefulAggregateHelperSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitLocalStorageCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.localStorageValueTypes.getCount() == 2);
        for (Index storageIndex = 0;
             storageIndex < gFakeNVVMBuilder.localStorageValueTypes.getCount();
             ++storageIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.localStorageValueTypes[storageIndex] ==
                _getFakeNVVMBuilderScalarStructType());
            SLANG_CHECK(gFakeNVVMBuilder.localStorageAlignments[storageIndex] == 4);
        }

        bool sawStructResult = false;
        bool sawMutableStructParameter = false;
        for (Index functionTypeIndex = 0;
             functionTypeIndex < gFakeNVVMBuilder.functionTypeResultKinds.getCount();
             ++functionTypeIndex)
        {
            sawStructResult |= gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
                               FakeNVVMBuilderResultTypeKind::ScalarStruct;
            const Index parameterOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
            const size_t parameterCount =
                gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex];
            for (size_t parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex)
            {
                sawMutableStructParameter |=
                    gFakeNVVMBuilder
                        .functionParameterTypeKinds[parameterOffset + Index(parameterIndex)] ==
                    FakeNVVMBuilderParameterTypeKind::ScalarStructPointer;
            }
        }
        SLANG_CHECK(sawStructResult);
        SLANG_CHECK(sawMutableStructParameter);

        bool sawLocalPointerCall = false;
        for (const FakeNVVMBuilderValueRef argument : gFakeNVVMBuilder.callArgumentValueRefs)
            sawLocalPointerCall |= argument.kind == FakeNVVMBuilderValueKind::LocalStorage;
        SLANG_CHECK(sawLocalPointerCall);
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount >= 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangThreadLocalGlobalUsesExplicitContext)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMThreadLocalGlobalContextSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        // The source global is per invocation. It must become one entry-local context, not one
        // provider global shared by every CUDA thread.
        SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitLocalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.localStorageValueTypes.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.localStorageValueTypes[0] == _getFakeNVVMBuilderScalarStructType());
        SLANG_CHECK(gFakeNVVMBuilder.localStorageAlignments[0] == 4);

        bool sawContextPointerParameter = false;
        for (const auto parameterTypeKind : gFakeNVVMBuilder.functionParameterTypeKinds)
        {
            sawContextPointerParameter |=
                parameterTypeKind == FakeNVVMBuilderParameterTypeKind::ScalarStructPointer;
        }
        SLANG_CHECK(sawContextPointerParameter);

        bool passedEntryLocalContext = false;
        for (const FakeNVVMBuilderValueRef argument : gFakeNVVMBuilder.callArgumentValueRefs)
        {
            passedEntryLocalContext |= argument.kind == FakeNVVMBuilderValueKind::LocalStorage;
        }
        SLANG_CHECK(passedEntryLocalContext);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangBooleanGlobalUsesEntryLocalContext)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        const char source[] = R"(
            static bool flag = false;
            [noinline] bool flip()
            {
                flag = !flag;
                return flag;
            }
            [CUDAKernel]
            void computeMain(
                uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
                uniform int value)
            {
                flag = value != 0;
                *destination = flip() ? 1 : 0;
            }
        )";
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result =
            _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        // Boolean state has the same per-invocation lifetime as the integer context above.
        // Passing its entry-local address must not create shared provider-global storage.
        SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitLocalStorageCallCount == 1);
        SLANG_CHECK_ABORT(gFakeNVVMBuilder.scalarStructFieldTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes[0] == _getFakeNVVMBuilderBooleanType());
        bool sawContextParameter = false;
        for (const auto kind : gFakeNVVMBuilder.functionParameterTypeKinds)
            sawContextParameter |= kind == FakeNVVMBuilderParameterTypeKind::ScalarStructPointer;
        SLANG_CHECK(sawContextParameter);
        bool passedEntryLocalContext = false;
        for (const auto argument : gFakeNVVMBuilder.callArgumentValueRefs)
            passedEntryLocalContext |= argument.kind == FakeNVVMBuilderValueKind::LocalStorage;
        SLANG_CHECK(passedEntryLocalContext);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangSelectedScalarTruthinessUsesTypedInequality)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMScalarTruthinessSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawSignedInteger = false;
        bool sawUnsignedInteger = false;
        bool sawFloat16 = false;
        bool sawFloat32 = false;
        bool sawBool = false;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            if (operation.key.operation != SLANG_NVVM_VALUE_OP_NOT_EQUAL ||
                operation.resultType.kind != SLANG_NVVM_VALUE_TYPE_BOOL ||
                operation.resultType.laneCount != 1 || operation.operandCount != 2 ||
                !NVVMSemantics::areSameType(operation.operandTypes[0], operation.operandTypes[1]))
            {
                continue;
            }
            const SlangNVVMValueTypeDesc& operandType = operation.operandTypes[0];
            const bool isDirectParameter =
                operation.operands[0].kind == FakeNVVMBuilderValueKind::Parameter;
            bool isDecodedHalfParameter = false;
            if (operation.operands[0].kind == FakeNVVMBuilderValueKind::ScalarOperation &&
                operation.operands[0].index >= 0 &&
                operation.operands[0].index < gFakeNVVMBuilder.scalarOperations.getCount())
            {
                const FakeNVVMBuilderScalarOperation& producer =
                    gFakeNVVMBuilder.scalarOperations[operation.operands[0].index];
                isDecodedHalfParameter =
                    operandType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                    operandType.bitWidth == 16 &&
                    producer.key.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET &&
                    NVVMSemantics::areSameType(producer.resultType, NVVMSemantics::kFloat16) &&
                    NVVMSemantics::areSameType(
                        producer.operandTypes[0],
                        NVVMSemantics::kUnsignedI16) &&
                    producer.operands[0].kind == FakeNVVMBuilderValueKind::Parameter;
            }
            SLANG_CHECK(isDirectParameter || isDecodedHalfParameter);
            const bool hasIntegerZero =
                operation.operands[1].kind == FakeNVVMBuilderValueKind::IntegerConstant;
            const bool hasFloatingPointZero =
                operation.operands[1].kind == FakeNVVMBuilderValueKind::FloatingPointConstant;
            sawSignedInteger |= operandType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER &&
                                operandType.bitWidth == 32 && hasIntegerZero;
            sawUnsignedInteger |= operandType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER &&
                                  operandType.bitWidth == 32 && hasIntegerZero;
            sawFloat16 |= operandType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                          operandType.bitWidth == 16 && hasFloatingPointZero;
            sawFloat32 |= operandType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                          operandType.bitWidth == 32 && hasFloatingPointZero;
            sawBool |= operandType.kind == SLANG_NVVM_VALUE_TYPE_BOOL &&
                       operandType.bitWidth == 1 && hasIntegerZero;
        }
        SLANG_CHECK(sawSignedInteger);
        SLANG_CHECK(sawUnsignedInteger);
        SLANG_CHECK(sawFloat16);
        SLANG_CHECK(sawFloat32);
        SLANG_CHECK(!sawBool);
        bool returnedBoolParameter = false;
        for (Index i = 0; i < gFakeNVVMBuilder.scalarReturnValueRefs.getCount(); ++i)
        {
            const auto ref = gFakeNVVMBuilder.scalarReturnValueRefs[i];
            const auto function =
                gFakeNVVMBuilder.blockFunctionIndices[gFakeNVVMBuilder.scalarReturnBlockIndices[i]];
            const auto type = gFakeNVVMBuilder.functionTypeIndices[function];
            returnedBoolParameter |=
                ref.kind == FakeNVVMBuilderValueKind::Parameter &&
                gFakeNVVMBuilder.functionTypeResultTypes[type] == _getFakeNVVMBuilderBooleanType();
        }
        SLANG_CHECK(returnedBoolParameter);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCopyableValuesAndNumericBorrowsCrossHelperBoundaries)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCopyableValueHelperSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawCopyableValueResult = false;
        bool sawCopyableValueParameter = false;
        bool sawMutableNumericParameter = false;
        for (Index functionTypeIndex = 0;
             functionTypeIndex < gFakeNVVMBuilder.functionTypeResultKinds.getCount();
             ++functionTypeIndex)
        {
            sawCopyableValueResult |= gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
                                      FakeNVVMBuilderResultTypeKind::ScalarStruct;
            const Index parameterOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
            const size_t parameterCount =
                gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex];
            for (size_t parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex)
            {
                const FakeNVVMBuilderParameterTypeKind parameterKind =
                    gFakeNVVMBuilder
                        .functionParameterTypeKinds[parameterOffset + Index(parameterIndex)];
                sawCopyableValueParameter |=
                    parameterKind == FakeNVVMBuilderParameterTypeKind::ScalarStruct;
                sawMutableNumericParameter |=
                    parameterKind == FakeNVVMBuilderParameterTypeKind::Pointer;
            }
        }
        SLANG_CHECK(sawCopyableValueResult);
        SLANG_CHECK(sawCopyableValueParameter);
        SLANG_CHECK(sawMutableNumericParameter);

        bool passedNumericLocalStorage = false;
        for (const FakeNVVMBuilderValueRef argument : gFakeNVVMBuilder.callArgumentValueRefs)
        {
            passedNumericLocalStorage |=
                argument.kind == FakeNVVMBuilderValueKind::LocalStorage && argument.index >= 0 &&
                argument.index < gFakeNVVMBuilder.localStorageValueTypes.getCount() &&
                gFakeNVVMBuilder.localStorageValueTypes[argument.index] ==
                    _getFakeNVVMBuilderIntegerType();
        }
        SLANG_CHECK(passedNumericLocalStorage);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// A readonly borrow must keep the same native field storage used by mutable helper calls.
SLANG_UNIT_TEST(nvvmSlangBorrowedFloat3KeepsNativeMemoryRepresentation)
{
    _resetDirectNVVMFakes();
    {
        const char* source = R"SLANG(
            struct Payload { float3 value; float sentinel; }
            [noinline] float3 read(__constref Payload p) { return p.value; }
            [noinline] void replace(inout Payload p, float3 value) { p.value = value; }
            RWStructuredBuffer<float> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p;
                p.value = float3(float(tid.x), 2.0f, 3.0f);
                p.sentinel = 9.0f;
                let before = read(p);
                replace(p, float3(4.0f, 5.0f, 6.0f));
                let after = read(p);
                outputBuffer[0] = before.x + after.y + p.sentinel;
            }
        )SLANG";
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(session, source, code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        bool sawNativeVectorLoad = false;
        for (Index i = 0; i < gFakeNVVMBuilder.loadResultTypeKinds.getCount(); ++i)
        {
            const auto kind = gFakeNVVMBuilder.loadResultTypeKinds[i];
            SLANG_CHECK(kind != FakeNVVMBuilderScalarTypeKind::NumericArray);
            if (kind == FakeNVVMBuilderScalarTypeKind::Float3)
            {
                sawNativeVectorLoad = true;
                SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[i] == 16);
                // A readonly borrow can refer to mutable caller storage. Read permission
                // does not establish an immutable location for invariant-load metadata.
                SLANG_CHECK(gFakeNVVMBuilder.loadFlags[i] == SLANG_NVVM_LOAD_FLAG_NONE);
            }
        }
        SLANG_CHECK(sawNativeVectorLoad);
        SLANG_CHECK(_countFakeNVVMNoInlineHelperCalls("read", 1) == 2);
        SLANG_CHECK(_countFakeNVVMNoInlineHelperCalls("replace", 2) == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRecursiveCopyableValuesCrossHelperBoundaries)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMRecursiveCopyableValueHelperSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder trace;
            trace << "recursive-copyable fake trace: arrays "
                  << gFakeNVVMBuilder.getArrayTypeCallCount << "; structs "
                  << gFakeNVVMBuilder.getStructTypeCallCount << "; local storage "
                  << gFakeNVVMBuilder.emitLocalStorageCallCount << "; field pointers "
                  << gFakeNVVMBuilder.emitStructFieldPointerCallCount << "; sequential pointers "
                  << gFakeNVVMBuilder.emitSequentialElementPointerCallCount << "; loads "
                  << gFakeNVVMBuilder.emitLoadCallCount << "; stores "
                  << gFakeNVVMBuilder.emitStoreCallCount << "; calls "
                  << gFakeNVVMBuilder.emitCallCallCount << "; value returns "
                  << gFakeNVVMBuilder.emitValueReturnCallCount << "; kernel marks "
                  << gFakeNVVMBuilder.markFunctionAsKernelCallCount;
            getTestReporter()->message(TestMessageType::TestFailure, trace.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawArrayField = false;
        for (const auto fieldType : gFakeNVVMBuilder.scalarStructFieldTypes)
            sawArrayField |= fieldType == _getFakeNVVMBuilderArrayType();
        SLANG_CHECK(sawArrayField);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLocalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount >= 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangTexture2DGathersUseOneTypedOperationFamily)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMTexture2DGatherSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        SLANG_CHECK(gFakeNVVMBuilder.textureOperations.getCount() == 4);
        bool sawComponents[4] = {};
        for (const auto& operation : gFakeNVVMBuilder.textureOperations)
        {
            SLANG_CHECK(operation.operation == SLANG_NVVM_TEXTURE_OP_GATHER);
            SLANG_CHECK(operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D);
            SLANG_CHECK(operation.isArray == 0);
            SLANG_CHECK(operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT);
            SLANG_CHECK(operation.elementType.bitWidth == 32);
            SLANG_CHECK(operation.elementType.laneCount == 4);
            SLANG_CHECK_ABORT(operation.component < SLANG_COUNT_OF(sawComponents));
            sawComponents[operation.component] = true;
        }
        for (bool sawComponent : sawComponents)
            SLANG_CHECK(sawComponent);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangImplicitTextureSamplesUseProducerSemantics)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMTexture2DImplicitSampleSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        SLANG_CHECK(gFakeNVVMBuilder.textureOperations.getCount() == 1);
        for (const auto& operation : gFakeNVVMBuilder.textureOperations)
        {
            SLANG_CHECK(operation.operation == SLANG_NVVM_TEXTURE_OP_SAMPLE);
            SLANG_CHECK(operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D);
            SLANG_CHECK(operation.isArray == 0);
            SLANG_CHECK(operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT);
            SLANG_CHECK(operation.elementType.bitWidth == 32);
            SLANG_CHECK(operation.elementType.laneCount == 4);
        }
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangResourceStructsCrossLocalAndHelperBoundaries)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMResourceStructHelperSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawResourceStructParameter = false;
        bool sawResourceStructResult = false;
        for (Index functionTypeIndex = 0;
             functionTypeIndex < gFakeNVVMBuilder.functionTypeResultKinds.getCount();
             ++functionTypeIndex)
        {
            sawResourceStructResult |=
                gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
                    FakeNVVMBuilderResultTypeKind::ScalarStruct &&
                gFakeNVVMBuilder.functionTypeResultTypes[functionTypeIndex] ==
                    _getFakeNVVMBuilderScalarStructType();
            const Index parameterOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
            const size_t parameterCount =
                gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex];
            for (size_t parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex)
            {
                sawResourceStructParameter |=
                    gFakeNVVMBuilder
                        .functionParameterTypeKinds[parameterOffset + Index(parameterIndex)] ==
                    FakeNVVMBuilderParameterTypeKind::ScalarStruct;
            }
        }
        SLANG_CHECK(sawResourceStructParameter);
        SLANG_CHECK(sawResourceStructResult);
        bool sawResourceStructCall = false;
        for (Index callIndex = 0; callIndex < gFakeNVVMBuilder.callResultKinds.getCount();
             ++callIndex)
        {
            sawResourceStructCall |= gFakeNVVMBuilder.callResultKinds[callIndex] ==
                                         FakeNVVMBuilderResultTypeKind::ScalarStruct &&
                                     gFakeNVVMBuilder.callResultTypes[callIndex] ==
                                         _getFakeNVVMBuilderScalarStructType();
        }
        SLANG_CHECK(sawResourceStructCall);
        SLANG_CHECK(gFakeNVVMBuilder.emitLocalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.localStorageValueTypes.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.localStorageValueTypes[0] == _getFakeNVVMBuilderScalarStructType());
        SLANG_CHECK(gFakeNVVMBuilder.localStorageAlignments[0] == 8);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount >= 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount >= 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.textureOperations.getCount() == 1);
        const SlangNVVMTextureOperationDesc& textureOperation =
            gFakeNVVMBuilder.textureOperations[0];
        SLANG_CHECK(textureOperation.operation == SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL);
        SLANG_CHECK(textureOperation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D);
        SLANG_CHECK(textureOperation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT);
        SLANG_CHECK(textureOperation.elementType.bitWidth == 32);
        SLANG_CHECK(textureOperation.elementType.laneCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount >= 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangAggregateValueArraysAndMutableHelpersUseGenericOperations)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMAggregateValueArraySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder trace;
            trace << "aggregate-value fake trace: structs "
                  << gFakeNVVMBuilder.getStructTypeCallCount << "; arrays "
                  << gFakeNVVMBuilder.getArrayTypeCallCount << "; functions "
                  << gFakeNVVMBuilder.declareFunctionCallCount << "; local storage "
                  << gFakeNVVMBuilder.emitLocalStorageCallCount << "; field pointers "
                  << gFakeNVVMBuilder.emitStructFieldPointerCallCount << "; constructs "
                  << gFakeNVVMBuilder.emitAggregateConstructCallCount << "; sequential extracts "
                  << gFakeNVVMBuilder.emitSequentialElementExtractCallCount << "; calls "
                  << gFakeNVVMBuilder.emitCallCallCount;
            getTestReporter()->message(TestMessageType::TestFailure, trace.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawMutableAggregateParameter = false;
        for (Index functionTypeIndex = 0;
             functionTypeIndex < gFakeNVVMBuilder.functionTypeResultKinds.getCount();
             ++functionTypeIndex)
        {
            const Index parameterOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
            const size_t parameterCount =
                gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex];
            for (size_t parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex)
            {
                sawMutableAggregateParameter |=
                    gFakeNVVMBuilder
                        .functionParameterTypeKinds[parameterOffset + Index(parameterIndex)] ==
                    FakeNVVMBuilderParameterTypeKind::ScalarStructPointer;
            }
        }
        SLANG_CHECK(sawMutableAggregateParameter);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementType == _getFakeNVVMBuilderScalarStructType());
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateConstructCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangComposesLocalAggregateElementAndFieldAddresses)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMComposableAggregateAddressSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool sawSequentialFieldBase = false;
        bool sawHelperParameterFieldBase = false;
        for (const auto base : gFakeNVVMBuilder.structFieldPointerBaseValueRefs)
        {
            sawSequentialFieldBase |=
                base.kind == FakeNVVMBuilderValueKind::SequentialElementPointer;
            sawHelperParameterFieldBase |= base.kind == FakeNVVMBuilderValueKind::Parameter;
        }
        SLANG_CHECK(sawSequentialFieldBase);
        SLANG_CHECK(sawHelperParameterFieldBase);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount >= 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangResourceArrayStorageUsesGenericAggregateOperations)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMResourceArrayStorageSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder trace;
            trace << "resource-array fake trace: arrays " << gFakeNVVMBuilder.getArrayTypeCallCount
                  << "; structs " << gFakeNVVMBuilder.getStructTypeCallCount << "; field pointers "
                  << gFakeNVVMBuilder.emitStructFieldPointerCallCount << "; sequential pointers "
                  << gFakeNVVMBuilder.emitSequentialElementPointerCallCount << "; loads "
                  << gFakeNVVMBuilder.emitLoadCallCount << "; extracts "
                  << gFakeNVVMBuilder.emitAggregateElementExtractCallCount;
            getTestReporter()->message(TestMessageType::TestFailure, trace.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.arrayElementType ==
            _getFakeNVVMBuilderResourceViewType(FakeNVVMBuilderScalarTypeKind::Integer));
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.sequentialElementPointerTypeKinds.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.sequentialElementPointerTypeKinds[0] ==
            FakeNVVMBuilderScalarTypeKind::ResourceView);

        bool sawResourceViewLoad = false;
        for (auto typeKind : gFakeNVVMBuilder.loadResultTypeKinds)
            sawResourceViewLoad |= typeKind == FakeNVVMBuilderScalarTypeKind::ResourceView;
        SLANG_CHECK(sawResourceViewLoad);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangLocalArraysCrossHelperReferenceBoundaries)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMLocalArrayHelperSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.arrayElementType ==
            _getFakeNVVMBuilderVectorType(3, FakeNVVMBuilderScalarTypeKind::Float));
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitLocalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.localStorageValueTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.localStorageValueTypes[0] == _getFakeNVVMBuilderArrayType());
        SLANG_CHECK(gFakeNVVMBuilder.localStorageAlignments[0] == 16);

        bool sawArrayPointerParameter = false;
        for (const auto parameterTypeKind : gFakeNVVMBuilder.functionParameterTypeKinds)
        {
            sawArrayPointerParameter |=
                parameterTypeKind == FakeNVVMBuilderParameterTypeKind::ArrayPointer;
        }
        SLANG_CHECK(sawArrayPointerParameter);

        bool passedLocalArray = false;
        for (const FakeNVVMBuilderValueRef argument : gFakeNVVMBuilder.callArgumentValueRefs)
        {
            passedLocalArray |=
                argument.kind == FakeNVVMBuilderValueKind::LocalStorage && argument.index == 0;
        }
        SLANG_CHECK(passedLocalArray);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 6);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 6);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCopyableStructLocalStoresToStructuredBuffer)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCopyableStructuredBufferAggregateSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 2);
        const Index parameterOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterOffset] ==
            FakeNVVMBuilderParameterTypeKind::ResourceView);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[parameterOffset] ==
            _getFakeNVVMBuilderResourceViewType(FakeNVVMBuilderScalarTypeKind::ScalarStruct));

        SLANG_CHECK(gFakeNVVMBuilder.emitLocalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.localStorageValueTypes.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.localStorageValueTypes[0] == _getFakeNVVMBuilderScalarStructType());
        SLANG_CHECK(gFakeNVVMBuilder.localStorageAlignments[0] == 8);
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes.getCount() == 3);
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes[1] == _getFakeNVVMBuilderFloatType());
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarStructFieldTypes[2] ==
            _getFakeNVVMBuilderVectorType(4, FakeNVVMBuilderScalarTypeKind::Half));

        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount >= 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::ScalarStruct);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 4);
        const uint32_t expectedStoreAlignments[] = {4, 4, 8, 4};
        SLANG_CHECK(
            gFakeNVVMBuilder.storeAlignments.getCount() == SLANG_COUNT_OF(expectedStoreAlignments));
        for (Index storeIndex = 0; storeIndex < SLANG_COUNT_OF(expectedStoreAlignments);
             ++storeIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.storeAlignments[storeIndex] ==
                expectedStoreAlignments[storeIndex]);
        }
        const FakeNVVMBuilderValueRef finalDestination =
            gFakeNVVMBuilder.storePointerValueRefs.getLast();
        const FakeNVVMBuilderValueRef finalValue = gFakeNVVMBuilder.storeValueRefs.getLast();
        SLANG_CHECK(finalDestination.kind == FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(finalValue.kind == FakeNVVMBuilderValueKind::AggregateConstruct);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCopyableStructLoadsAndLocalArraysUseGenericAggregates)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCopyableStructArraySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementType == _getFakeNVVMBuilderScalarStructType());
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLocalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.localStorageValueTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.localStorageValueTypes[0] == _getFakeNVVMBuilderArrayType());
        SLANG_CHECK(gFakeNVVMBuilder.localStorageAlignments[0] == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 3);

        bool extractedFromFirstClassLoad = false;
        for (const auto base : gFakeNVVMBuilder.aggregateElementBaseValueRefs)
            extractedFromFirstClassLoad |= base.kind == FakeNVVMBuilderValueKind::Load;
        SLANG_CHECK(extractedFromFirstClassLoad);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangMutableStructuredBufferAggregateFieldsUseGenericPointers)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMMutableStructuredBufferAggregateFieldSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.scalarStructFieldTypes.getCount() == 2);
        for (SlangNVVMTypeHandle fieldType : gFakeNVVMBuilder.scalarStructFieldTypes)
        {
            SLANG_CHECK(
                fieldType ==
                _getFakeNVVMBuilderVectorType(4, FakeNVVMBuilderScalarTypeKind::Integer));
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 2);
        const uint32_t expectedFieldIndices[] = {1, 0};
        for (Index i = 0; i < SLANG_COUNT_OF(expectedFieldIndices); ++i)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.structFieldPointerBaseValueRefs[i].kind ==
                FakeNVVMBuilderValueKind::PointerOffset);
            SLANG_CHECK(gFakeNVVMBuilder.structFieldPointerIndices[i] == expectedFieldIndices[i]);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.sequentialElementPointerBaseValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::StructFieldPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.sequentialElementPointerIndexValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignment == 16);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::SequentialElementPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::VectorElement);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangEmptyComputeUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

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
            StringBuilder state;
            state << "direct NVVM compile result " << int(compileResult) << "; builder modules "
                  << gFakeNVVMBuilder.createModuleCallCount << "; libNVVM programs "
                  << gFakeNVVM.createProgramCallCount << "; libNVVM modules "
                  << gFakeNVVM.addModuleCallCount;
            getTestReporter()->message(TestMessageType::Info, state.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.functionName == "computeMain");
        SLANG_CHECK(gFakeNVVMBuilder.blockName == "entry");
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getVoidTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.setInsertBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeQueryCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWriteCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.destroyModuleCallCount == 1);

        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addedModule.getLength() == sizeof(kFakeNVVMBuilderAssembly) - 1);
        SLANG_CHECK(
            ::memcmp(
                gFakeNVVM.addedModule.getBuffer(),
                kFakeNVVMBuilderAssembly,
                sizeof(kFakeNVVMBuilderAssembly) - 1) == 0);
        SLANG_CHECK(_hasOption(gFakeNVVM.verifyOptions, "-arch=compute_70"));
        SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-arch=compute_70"));
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVM.successfulLoadCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangTypeCacheIsModuleLocal)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        // Each compile creates a provider module. Within one module the result, pointer, and every
        // signed-i32 value share the centralized cache; the second module must reconstruct its own
        // handles because provider types cannot escape their module lifetime.
        for (int compileIndex = 0; compileIndex < 2; ++compileIndex)
        {
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
                globalSession,
                kDirectNVVMWriteScalarSource,
                code,
                diagnostics)));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        }

        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.destroyModuleCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.getVoidTypeCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionTypeCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 2);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangScalarMemoryAndConditionalUseDirectPipeline)
{
    struct ExpectedBuilderGraph
    {
        const char* source;
        const FakeNVVMBuilderParameterTypeKind* parameterTypeKinds;
        size_t parameterCount;
        int blockCount;
        int loadCount;
        int storeCount;
        int binaryCount;
        int comparisonCount;
        int branchCount;
        int conditionalBranchCount;
    };
    static const FakeNVVMBuilderParameterTypeKind kWriteParameterTypes[] = {
        FakeNVVMBuilderParameterTypeKind::Pointer,
        FakeNVVMBuilderParameterTypeKind::Integer,
    };
    static const FakeNVVMBuilderParameterTypeKind kCopyParameterTypes[] = {
        FakeNVVMBuilderParameterTypeKind::Pointer,
        FakeNVVMBuilderParameterTypeKind::Pointer,
    };
    static const FakeNVVMBuilderParameterTypeKind kChooseParameterTypes[] = {
        FakeNVVMBuilderParameterTypeKind::Pointer,
        FakeNVVMBuilderParameterTypeKind::Integer,
        FakeNVVMBuilderParameterTypeKind::Integer,
    };
    static const ExpectedBuilderGraph kCases[] = {
        {kDirectNVVMWriteScalarSource, kWriteParameterTypes, 2, 1, 0, 1, 0, 0, 0, 0},
        {kDirectNVVMCopyScalarSource, kCopyParameterTypes, 2, 1, 1, 1, 0, 0, 0, 0},
        {kDirectNVVMChooseScalarSource, kChooseParameterTypes, 3, 4, 0, 2, 2, 1, 2, 1},
    };

    for (const auto& expected : kCases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult compileResult =
                _compileSlangWithDirectNVVM(globalSession, expected.source, code, diagnostics);
            if (SLANG_FAILED(compileResult))
            {
                const String diagnosticText = _getBlobText(diagnostics);
                if (diagnosticText.getLength())
                    getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

            SLANG_CHECK(gFakeNVVMBuilder.functionName == "computeMain");
            SLANG_CHECK(gFakeNVVMBuilder.functionParameterCount == expected.parameterCount);
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds.getCount() ==
                Index(expected.parameterCount));
            for (Index i = 0; i < gFakeNVVMBuilder.functionParameterTypeKinds.getCount(); ++i)
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder.functionParameterTypeKinds[i] ==
                    expected.parameterTypeKinds[i]);
            }
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.getVoidTypeCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
            SLANG_CHECK(
                gFakeNVVMBuilder.getFunctionParameterCallCount == int(expected.parameterCount));
            SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == expected.blockCount);
            SLANG_CHECK(gFakeNVVMBuilder.setInsertBlockCallCount == expected.blockCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == expected.loadCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == expected.storeCount);
            SLANG_CHECK(
                gFakeNVVMBuilder
                    .scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)] ==
                expected.binaryCount);
            SLANG_CHECK(
                gFakeNVVMBuilder
                    .scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Compare)] ==
                expected.comparisonCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == expected.branchCount);
            SLANG_CHECK(
                gFakeNVVMBuilder.emitConditionalBranchCallCount == expected.conditionalBranchCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.destroyModuleCallCount == 1);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
            SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);

            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterIndices.getCount() ==
                Index(expected.parameterCount));
            for (Index i = 0; i < gFakeNVVMBuilder.functionParameterIndices.getCount(); ++i)
                SLANG_CHECK(gFakeNVVMBuilder.functionParameterIndices[i] == size_t(i));
            SLANG_CHECK(
                gFakeNVVMBuilder.storePointerParameterIndices.getCount() == expected.storeCount);
            for (size_t pointerIndex : gFakeNVVMBuilder.storePointerParameterIndices)
                SLANG_CHECK(pointerIndex == 0);

            if (expected.binaryCount)
            {
                const Index addIndex = _findFakeNVVMBuilderScalarOperation(
                    FakeNVVMBuilderScalarFamily::Binary,
                    SLANG_NVVM_VALUE_OP_ADD);
                const Index subIndex = _findFakeNVVMBuilderScalarOperation(
                    FakeNVVMBuilderScalarFamily::Binary,
                    SLANG_NVVM_VALUE_OP_SUBTRACT);
                const Index compareIndex = _findFakeNVVMBuilderScalarOperation(
                    FakeNVVMBuilderScalarFamily::Compare,
                    SLANG_NVVM_VALUE_OP_LESS_THAN);
                SLANG_CHECK_ABORT(addIndex >= 0);
                SLANG_CHECK_ABORT(subIndex >= 0);
                SLANG_CHECK_ABORT(compareIndex >= 0);
                const FakeNVVMBuilderScalarOperation& comparison =
                    gFakeNVVMBuilder.scalarOperations[compareIndex];
                SLANG_CHECK(comparison.operands[0].index == 1);
                SLANG_CHECK(comparison.operands[1].index == 2);
                for (Index binaryIndex : {addIndex, subIndex})
                {
                    const FakeNVVMBuilderScalarOperation& binary =
                        gFakeNVVMBuilder.scalarOperations[binaryIndex];
                    SLANG_CHECK(binary.operands[0].index == 1);
                    SLANG_CHECK(binary.operands[1].index == 2);
                }
                SLANG_CHECK(gFakeNVVMBuilder.conditionalTrueBlockIndex == 1);
                SLANG_CHECK(gFakeNVVMBuilder.conditionalFalseBlockIndex == 2);
                SLANG_CHECK(gFakeNVVMBuilder.branchTargetBlockIndices.getCount() == 2);
                SLANG_CHECK(gFakeNVVMBuilder.branchTargetBlockIndices[0] == 3);
                SLANG_CHECK(gFakeNVVMBuilder.branchTargetBlockIndices[1] == 3);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueKinds.getCount() == 2);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueKinds[0] ==
                    FakeNVVMBuilderValueKind::ScalarOperation);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueKinds[1] ==
                    FakeNVVMBuilderValueKind::ScalarOperation);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueBinaryIndices.getCount() == 2);
                SLANG_CHECK(
                    (gFakeNVVMBuilder.storeValueBinaryIndices[0] == addIndex &&
                     gFakeNVVMBuilder.storeValueBinaryIndices[1] == subIndex) ||
                    (gFakeNVVMBuilder.storeValueBinaryIndices[0] == subIndex &&
                     gFakeNVVMBuilder.storeValueBinaryIndices[1] == addIndex));
            }
            else if (expected.loadCount)
            {
                SLANG_CHECK(gFakeNVVMBuilder.loadPointerParameterIndices.getCount() == 1);
                SLANG_CHECK(gFakeNVVMBuilder.loadPointerParameterIndices[0] == 1);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueKinds.getCount() == 1);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueKinds[0] == FakeNVVMBuilderValueKind::Load);
            }
            else
            {
                SLANG_CHECK(gFakeNVVMBuilder.storeValueKinds.getCount() == 1);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueKinds[0] == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueParameterIndices[0] == 1);
            }
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangScalarSSAUsesDirectPipeline)
{
    enum class SSAShape
    {
        Constant,
        Merge,
        Loop,
    };
    struct ExpectedGraph
    {
        const char* source;
        SSAShape shape;
        int blockCount;
        int constantCount;
        int phiCount;
        int incomingCount;
        int binaryCount;
        int comparisonCount;
        int branchCount;
    };
    static const ExpectedGraph kCases[] = {
        {kDirectNVVMIntegerConstantSource, SSAShape::Constant, 1, 1, 0, 0, 1, 0, 0},
        {kDirectNVVMMergePhiSource, SSAShape::Merge, 4, 0, 1, 2, 0, 1, 2},
        {kDirectNVVMFiniteLoopSource, SSAShape::Loop, 6, 2, 2, 4, 2, 1, 4},
    };

    for (const auto& expected : kCases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult result =
                _compileSlangWithDirectNVVM(globalSession, expected.source, code, diagnostics);
            if (SLANG_FAILED(result))
            {
                const String diagnosticText = _getBlobText(diagnostics);
                if (diagnosticText.getLength())
                    getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

            SLANG_CHECK(gFakeNVVMBuilder.functionName == "computeMain");
            if (expected.shape == SSAShape::Constant || expected.shape == SSAShape::Loop)
            {
                SLANG_CHECK(gFakeNVVMBuilder.functionParameterCount == 2);
            }
            else
            {
                SLANG_CHECK(gFakeNVVMBuilder.functionParameterCount == 3);
            }
            SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == expected.blockCount);
            SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == expected.constantCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount == expected.phiCount);
            SLANG_CHECK(gFakeNVVMBuilder.addPhiIncomingCallCount == expected.incomingCount);
            SLANG_CHECK(
                gFakeNVVMBuilder
                    .scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)] ==
                expected.binaryCount);
            SLANG_CHECK(
                gFakeNVVMBuilder
                    .scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Compare)] ==
                expected.comparisonCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == expected.branchCount);
            SLANG_CHECK(
                gFakeNVVMBuilder.emitConditionalBranchCallCount == expected.comparisonCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);

            if (expected.shape == SSAShape::Constant)
            {
                SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues.getCount() == 1);
                SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == 1);
                const Index addIndex = _findFakeNVVMBuilderScalarOperation(
                    FakeNVVMBuilderScalarFamily::Binary,
                    SLANG_NVVM_VALUE_OP_ADD);
                SLANG_CHECK_ABORT(addIndex >= 0);
                const FakeNVVMBuilderScalarOperation& add =
                    gFakeNVVMBuilder.scalarOperations[addIndex];
                SLANG_CHECK(add.operands[0].kind == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(add.operands[0].index == 1);
                SLANG_CHECK(add.operands[1].kind == FakeNVVMBuilderValueKind::IntegerConstant);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::ScalarOperation);
                SLANG_CHECK(add.callerBlockIndex == 0);
                SLANG_CHECK(gFakeNVVMBuilder.storeBlockIndices[0] == 0);
            }
            if (expected.shape == SSAShape::Merge)
            {
                SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes.getCount() == 1);
                SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes[0] == _getFakeNVVMBuilderIntegerType());
                const Index mergeBlock = gFakeNVVMBuilder.scalarPhiTargetBlockIndices[0];
                const Index entryBlock = gFakeNVVMBuilder.conditionalSourceBlockIndex;
                const Index trueBlock = gFakeNVVMBuilder.conditionalTrueBlockIndex;
                const Index falseBlock = gFakeNVVMBuilder.conditionalFalseBlockIndex;
                SLANG_CHECK(entryBlock >= 0);
                SLANG_CHECK(trueBlock >= 0);
                SLANG_CHECK(falseBlock >= 0);
                SLANG_CHECK(mergeBlock >= 0);
                SLANG_CHECK(entryBlock != trueBlock);
                SLANG_CHECK(entryBlock != falseBlock);
                SLANG_CHECK(entryBlock != mergeBlock);
                SLANG_CHECK(trueBlock != falseBlock);
                SLANG_CHECK(trueBlock != mergeBlock);
                SLANG_CHECK(falseBlock != mergeBlock);
                SLANG_CHECK(gFakeNVVMBuilder.scalarPhiIncomingPhiIndices.getCount() == 2);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::ScalarPhi);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
                SLANG_CHECK(gFakeNVVMBuilder.storeBlockIndices[0] == mergeBlock);

                Index xPredecessor = -1;
                Index yPredecessor = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.scalarPhiIncomingPhiIndices.getCount(); ++i)
                {
                    SLANG_CHECK(gFakeNVVMBuilder.scalarPhiIncomingPhiIndices[i] == 0);
                    const FakeNVVMBuilderValueRef valueRef =
                        gFakeNVVMBuilder.scalarPhiIncomingValueRefs[i];
                    SLANG_CHECK(valueRef.kind == FakeNVVMBuilderValueKind::Parameter);
                    if (valueRef.index == 1)
                    {
                        xPredecessor = gFakeNVVMBuilder.scalarPhiIncomingPredecessorBlockIndices[i];
                    }
                    else if (valueRef.index == 2)
                    {
                        yPredecessor = gFakeNVVMBuilder.scalarPhiIncomingPredecessorBlockIndices[i];
                    }
                }
                SLANG_CHECK(xPredecessor == trueBlock);
                SLANG_CHECK(yPredecessor == falseBlock);
                SLANG_CHECK(xPredecessor != yPredecessor);
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(trueBlock, mergeBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(falseBlock, mergeBlock));
            }
            else if (expected.shape == SSAShape::Loop)
            {
                SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues.getCount() == 2);
                Index zeroIndex = -1;
                Index oneIndex = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.integerConstantValues.getCount(); ++i)
                {
                    if (gFakeNVVMBuilder.integerConstantValues[i] == 0)
                        zeroIndex = i;
                    else if (gFakeNVVMBuilder.integerConstantValues[i] == 1)
                        oneIndex = i;
                }
                SLANG_CHECK(zeroIndex >= 0);
                SLANG_CHECK(oneIndex >= 0);
                SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes.getCount() == 2);
                SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes[0] == _getFakeNVVMBuilderIntegerType());
                SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes[1] == _getFakeNVVMBuilderIntegerType());
                SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTargetBlockIndices.getCount() == 2);
                const Index headerBlock = gFakeNVVMBuilder.scalarPhiTargetBlockIndices[0];
                SLANG_CHECK(headerBlock != 0);
                SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTargetBlockIndices[1] == headerBlock);
                SLANG_CHECK(gFakeNVVMBuilder.scalarPhiIncomingPhiIndices.getCount() == 4);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::ScalarPhi);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 1);
                const Index compareIndex = _findFakeNVVMBuilderScalarOperation(
                    FakeNVVMBuilderScalarFamily::Compare,
                    SLANG_NVVM_VALUE_OP_LESS_THAN);
                SLANG_CHECK_ABORT(compareIndex >= 0);
                const FakeNVVMBuilderScalarOperation& comparison =
                    gFakeNVVMBuilder.scalarOperations[compareIndex];
                SLANG_CHECK(comparison.operands[0].kind == FakeNVVMBuilderValueKind::ScalarPhi);
                SLANG_CHECK(comparison.operands[0].index == 0);
                SLANG_CHECK(comparison.operands[1].kind == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(comparison.operands[1].index == 1);

                Index nextSumIndex = -1;
                Index nextIIndex = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.scalarOperations.getCount(); ++i)
                {
                    const FakeNVVMBuilderScalarOperation& scalarOperation =
                        gFakeNVVMBuilder.scalarOperations[i];
                    if (!_isFakeNVVMBuilderScalarOperation(
                            scalarOperation.key,
                            FakeNVVMBuilderScalarFamily::Binary,
                            SLANG_NVVM_VALUE_OP_ADD))
                        continue;
                    const FakeNVVMBuilderValueRef left = scalarOperation.operands[0];
                    const FakeNVVMBuilderValueRef right = scalarOperation.operands[1];
                    const bool leftIsI =
                        left.kind == FakeNVVMBuilderValueKind::ScalarPhi && left.index == 0;
                    const bool rightIsI =
                        right.kind == FakeNVVMBuilderValueKind::ScalarPhi && right.index == 0;
                    const bool leftIsSum =
                        left.kind == FakeNVVMBuilderValueKind::ScalarPhi && left.index == 1;
                    const bool rightIsSum =
                        right.kind == FakeNVVMBuilderValueKind::ScalarPhi && right.index == 1;
                    const bool leftIsOne = left.kind == FakeNVVMBuilderValueKind::IntegerConstant &&
                                           left.index == oneIndex;
                    const bool rightIsOne =
                        right.kind == FakeNVVMBuilderValueKind::IntegerConstant &&
                        right.index == oneIndex;
                    if ((leftIsSum && rightIsI) || (leftIsI && rightIsSum))
                    {
                        nextSumIndex = i;
                    }
                    if ((leftIsI && rightIsOne) || (leftIsOne && rightIsI))
                    {
                        nextIIndex = i;
                    }
                }
                SLANG_CHECK_ABORT(nextSumIndex >= 0);
                SLANG_CHECK_ABORT(nextIIndex >= 0);

                Index entryBlock = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.scalarPhiIncomingPhiIndices.getCount(); ++i)
                {
                    const FakeNVVMBuilderValueRef valueRef =
                        gFakeNVVMBuilder.scalarPhiIncomingValueRefs[i];
                    if (gFakeNVVMBuilder.scalarPhiIncomingPhiIndices[i] == 0 &&
                        valueRef.kind == FakeNVVMBuilderValueKind::IntegerConstant &&
                        valueRef.index == zeroIndex)
                    {
                        entryBlock = gFakeNVVMBuilder.scalarPhiIncomingPredecessorBlockIndices[i];
                        break;
                    }
                }
                SLANG_CHECK(entryBlock >= 0);
                SLANG_CHECK(_hasFakeNVVMBuilderPhiIncoming(
                    0,
                    FakeNVVMBuilderValueKind::IntegerConstant,
                    zeroIndex,
                    entryBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderPhiIncoming(
                    1,
                    FakeNVVMBuilderValueKind::IntegerConstant,
                    zeroIndex,
                    entryBlock));

                Index continueBlock = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.scalarPhiIncomingPhiIndices.getCount(); ++i)
                {
                    const FakeNVVMBuilderValueRef valueRef =
                        gFakeNVVMBuilder.scalarPhiIncomingValueRefs[i];
                    if (gFakeNVVMBuilder.scalarPhiIncomingPhiIndices[i] == 0 &&
                        valueRef.kind == FakeNVVMBuilderValueKind::ScalarOperation &&
                        valueRef.index == nextIIndex)
                    {
                        continueBlock =
                            gFakeNVVMBuilder.scalarPhiIncomingPredecessorBlockIndices[i];
                        break;
                    }
                }
                SLANG_CHECK(continueBlock >= 0);
                SLANG_CHECK(_hasFakeNVVMBuilderPhiIncoming(
                    0,
                    FakeNVVMBuilderValueKind::ScalarOperation,
                    nextIIndex,
                    continueBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderPhiIncoming(
                    1,
                    FakeNVVMBuilderValueKind::ScalarOperation,
                    nextSumIndex,
                    continueBlock));

                const Index bodyBlock = gFakeNVVMBuilder.conditionalTrueBlockIndex;
                const Index exitBlock = gFakeNVVMBuilder.conditionalFalseBlockIndex;
                const Index breakBlock = gFakeNVVMBuilder.storeBlockIndices[0];
                SLANG_CHECK(gFakeNVVMBuilder.conditionalSourceBlockIndex == headerBlock);
                SLANG_CHECK(bodyBlock >= 0);
                SLANG_CHECK(exitBlock >= 0);
                SLANG_CHECK(breakBlock >= 0);
                SLANG_CHECK(entryBlock != headerBlock);
                SLANG_CHECK(entryBlock != bodyBlock);
                SLANG_CHECK(entryBlock != continueBlock);
                SLANG_CHECK(entryBlock != exitBlock);
                SLANG_CHECK(entryBlock != breakBlock);
                SLANG_CHECK(headerBlock != bodyBlock);
                SLANG_CHECK(headerBlock != continueBlock);
                SLANG_CHECK(headerBlock != exitBlock);
                SLANG_CHECK(headerBlock != breakBlock);
                SLANG_CHECK(bodyBlock != continueBlock);
                SLANG_CHECK(bodyBlock != exitBlock);
                SLANG_CHECK(bodyBlock != breakBlock);
                SLANG_CHECK(continueBlock != exitBlock);
                SLANG_CHECK(continueBlock != breakBlock);
                SLANG_CHECK(exitBlock != breakBlock);
                SLANG_CHECK(
                    gFakeNVVMBuilder.scalarOperations[nextSumIndex].callerBlockIndex == bodyBlock);
                SLANG_CHECK(
                    gFakeNVVMBuilder.scalarOperations[nextIIndex].callerBlockIndex ==
                    continueBlock);
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(entryBlock, headerBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(bodyBlock, continueBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(continueBlock, headerBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(exitBlock, breakBlock));
            }
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangScalarFunctionsUseDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMScalarFunctionSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)] ==
            2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Compare)] ==
            0);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);

        const Index kernelFunction = gFakeNVVMBuilder.kernelFunctionIndices[0];
        SLANG_CHECK_ABORT(kernelFunction >= 0);
        SLANG_CHECK_ABORT(kernelFunction < gFakeNVVMBuilder.functionTypeIndices.getCount());
        for (Index functionIndex = 0; functionIndex < 3; ++functionIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.functionLinkages[functionIndex] ==
                (functionIndex == kernelFunction ? SLANG_NVVM_LINKAGE_EXTERNAL
                                                 : SLANG_NVVM_LINKAGE_INTERNAL));
            SLANG_CHECK(
                gFakeNVVMBuilder.functionFlags[functionIndex] == SLANG_NVVM_FUNCTION_FLAG_NONE);
        }
        const Index kernelType = gFakeNVVMBuilder.functionTypeIndices[kernelFunction];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[kernelType] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[kernelType] == 2);
        const Index kernelTypeOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[kernelType];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[kernelTypeOffset] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[kernelTypeOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        Index functionBlocks[3] = {-1, -1, -1};
        for (Index blockIndex = 0; blockIndex < gFakeNVVMBuilder.blockFunctionIndices.getCount();
             ++blockIndex)
        {
            const Index functionIndex = gFakeNVVMBuilder.blockFunctionIndices[blockIndex];
            SLANG_CHECK(functionIndex >= 0 && functionIndex < 3);
            SLANG_CHECK(functionBlocks[functionIndex] == -1);
            functionBlocks[functionIndex] = blockIndex;
        }
        SLANG_CHECK_ABORT(functionBlocks[kernelFunction] >= 0);

        Index incrementFunction = -1;
        Index incrementBinary = -1;
        Index kernelBinary = -1;
        for (Index binaryIndex = 0; binaryIndex < gFakeNVVMBuilder.scalarOperations.getCount();
             ++binaryIndex)
        {
            SLANG_CHECK(_isFakeNVVMBuilderScalarOperation(
                gFakeNVVMBuilder.scalarOperations[binaryIndex].key,
                FakeNVVMBuilderScalarFamily::Binary,
                SLANG_NVVM_VALUE_OP_ADD));
            const Index blockIndex =
                gFakeNVVMBuilder.scalarOperations[binaryIndex].callerBlockIndex;
            const Index functionIndex = gFakeNVVMBuilder.blockFunctionIndices[blockIndex];
            const FakeNVVMBuilderValueRef left =
                gFakeNVVMBuilder.scalarOperations[binaryIndex].operands[0];
            const FakeNVVMBuilderValueRef right =
                gFakeNVVMBuilder.scalarOperations[binaryIndex].operands[1];
            const bool isIncrement =
                ((left.kind == FakeNVVMBuilderValueKind::Parameter &&
                  left.functionIndex == functionIndex && left.index == 0 &&
                  right.kind == FakeNVVMBuilderValueKind::IntegerConstant && right.index == 0) ||
                 (right.kind == FakeNVVMBuilderValueKind::Parameter &&
                  right.functionIndex == functionIndex && right.index == 0 &&
                  left.kind == FakeNVVMBuilderValueKind::IntegerConstant && left.index == 0));
            if (isIncrement)
            {
                incrementFunction = functionIndex;
                incrementBinary = binaryIndex;
            }
            else
            {
                SLANG_CHECK(functionIndex == kernelFunction);
                kernelBinary = binaryIndex;
            }
        }
        SLANG_CHECK_ABORT(incrementFunction >= 0);
        SLANG_CHECK_ABORT(incrementFunction != kernelFunction);
        SLANG_CHECK_ABORT(incrementBinary >= 0);
        SLANG_CHECK_ABORT(kernelBinary >= 0);

        Index incrementTwiceFunction = -1;
        for (Index functionIndex = 0; functionIndex < 3; ++functionIndex)
        {
            if (functionIndex != kernelFunction && functionIndex != incrementFunction)
                incrementTwiceFunction = functionIndex;
        }
        SLANG_CHECK_ABORT(incrementTwiceFunction >= 0);
        const Index helperFunctions[] = {incrementFunction, incrementTwiceFunction};
        for (Index helperFunction : helperFunctions)
        {
            const Index helperType = gFakeNVVMBuilder.functionTypeIndices[helperFunction];
            SLANG_CHECK(
                gFakeNVVMBuilder.functionTypeResultKinds[helperType] ==
                FakeNVVMBuilderResultTypeKind::Integer);
            SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[helperType] == 1);
            const Index helperTypeOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[helperType];
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[helperTypeOffset] ==
                FakeNVVMBuilderParameterTypeKind::Integer);
        }

        Index incrementTwiceFirstCall = -1;
        Index incrementTwiceSecondCall = -1;
        Index kernelIncrementCall = -1;
        Index kernelIncrementTwiceCall = -1;
        for (Index callIndex = 0; callIndex < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount();
             ++callIndex)
        {
            const Index callerBlock = gFakeNVVMBuilder.callCallerBlockIndices[callIndex];
            const Index callerFunction = gFakeNVVMBuilder.blockFunctionIndices[callerBlock];
            const Index calleeFunction = gFakeNVVMBuilder.callCalleeFunctionIndices[callIndex];
            SLANG_CHECK(gFakeNVVMBuilder.callArgumentCounts[callIndex] == 1);
            const FakeNVVMBuilderValueRef argument =
                gFakeNVVMBuilder
                    .callArgumentValueRefs[gFakeNVVMBuilder.callArgumentOffsets[callIndex]];
            if (callerFunction == incrementTwiceFunction)
            {
                SLANG_CHECK(calleeFunction == incrementFunction);
                if (argument.kind == FakeNVVMBuilderValueKind::Parameter)
                {
                    SLANG_CHECK(argument.functionIndex == incrementTwiceFunction);
                    SLANG_CHECK(argument.index == 0);
                    incrementTwiceFirstCall = callIndex;
                }
                else
                {
                    SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::Call);
                    incrementTwiceSecondCall = callIndex;
                }
            }
            else
            {
                SLANG_CHECK(callerFunction == kernelFunction);
                SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(argument.functionIndex == kernelFunction);
                SLANG_CHECK(argument.index == 1);
                if (calleeFunction == incrementFunction)
                    kernelIncrementCall = callIndex;
                else if (calleeFunction == incrementTwiceFunction)
                    kernelIncrementTwiceCall = callIndex;
                else
                    SLANG_CHECK(false);
            }
        }
        SLANG_CHECK_ABORT(incrementTwiceFirstCall >= 0);
        SLANG_CHECK_ABORT(incrementTwiceSecondCall >= 0);
        SLANG_CHECK_ABORT(kernelIncrementCall >= 0);
        SLANG_CHECK_ABORT(kernelIncrementTwiceCall >= 0);
        const FakeNVVMBuilderValueRef secondCallArgument =
            gFakeNVVMBuilder.callArgumentValueRefs
                [gFakeNVVMBuilder.callArgumentOffsets[incrementTwiceSecondCall]];
        SLANG_CHECK(secondCallArgument.kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(secondCallArgument.index == incrementTwiceFirstCall);

        SLANG_CHECK(gFakeNVVMBuilder.scalarReturnValueRefs.getCount() == 2);
        bool sawIncrementReturn = false;
        bool sawIncrementTwiceReturn = false;
        for (Index returnIndex = 0; returnIndex < gFakeNVVMBuilder.scalarReturnValueRefs.getCount();
             ++returnIndex)
        {
            const Index returnBlock = gFakeNVVMBuilder.scalarReturnBlockIndices[returnIndex];
            const Index returnFunction = gFakeNVVMBuilder.blockFunctionIndices[returnBlock];
            const FakeNVVMBuilderValueRef returnValue =
                gFakeNVVMBuilder.scalarReturnValueRefs[returnIndex];
            if (returnFunction == incrementFunction)
            {
                SLANG_CHECK(returnValue.kind == FakeNVVMBuilderValueKind::ScalarOperation);
                SLANG_CHECK(returnValue.index == incrementBinary);
                sawIncrementReturn = true;
            }
            else if (returnFunction == incrementTwiceFunction)
            {
                SLANG_CHECK(returnValue.kind == FakeNVVMBuilderValueKind::Call);
                SLANG_CHECK(returnValue.index == incrementTwiceSecondCall);
                sawIncrementTwiceReturn = true;
            }
            else
            {
                SLANG_CHECK(false);
            }
        }
        SLANG_CHECK(sawIncrementReturn);
        SLANG_CHECK(sawIncrementTwiceReturn);

        const FakeNVVMBuilderValueRef kernelLeft =
            gFakeNVVMBuilder.scalarOperations[kernelBinary].operands[0];
        const FakeNVVMBuilderValueRef kernelRight =
            gFakeNVVMBuilder.scalarOperations[kernelBinary].operands[1];
        SLANG_CHECK(kernelLeft.kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(kernelRight.kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(
            (kernelLeft.index == kernelIncrementCall &&
             kernelRight.index == kernelIncrementTwiceCall) ||
            (kernelLeft.index == kernelIncrementTwiceCall &&
             kernelRight.index == kernelIncrementCall));
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::ScalarOperation);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == kernelBinary);
        SLANG_CHECK(gFakeNVVMBuilder.storeBlockIndices[0] == functionBlocks[kernelFunction]);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerFunctionIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerFunctionIndices[0] == kernelFunction);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerParameterIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerParameterIndices[0] == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // Linking must prune an unreachable helper with an otherwise unsupported body. The direct
    // emitter receives only the selected kernel and its one reachable helper.
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMPrunesUnreachableHelperSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)] ==
            1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangVectorFunctionsUseExactGenericTypes)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMVectorFunctionSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 4);
        Index chooseFunction = -1;
        Index floatFunction = -1;
        Index boolFunction = -1;
        for (Index functionIndex = 0; functionIndex < gFakeNVVMBuilder.functionNames.getCount();
             ++functionIndex)
        {
            const String& name = gFakeNVVMBuilder.functionNames[functionIndex];
            if (name.indexOf("chooseInt4") >= 0)
                chooseFunction = functionIndex;
            else if (name.indexOf("identityFloat3") >= 0)
                floatFunction = functionIndex;
            else if (name.indexOf("identityBool2") >= 0)
                boolFunction = functionIndex;
        }
        SLANG_CHECK_ABORT(chooseFunction >= 0);
        SLANG_CHECK_ABORT(floatFunction >= 0);
        SLANG_CHECK_ABORT(boolFunction >= 0);

        const SlangNVVMTypeHandle int4Type = _getFakeNVVMBuilderVectorType(4);
        const SlangNVVMTypeHandle float3Type =
            _getFakeNVVMBuilderVectorType(3, FakeNVVMBuilderScalarTypeKind::Float);
        const SlangNVVMTypeHandle bool2Type =
            _getFakeNVVMBuilderVectorType(2, FakeNVVMBuilderScalarTypeKind::Boolean);
        struct ExpectedHelper
        {
            Index functionIndex;
            SlangNVVMTypeHandle resultType;
            SlangNVVMTypeHandle parameterTypes[3];
            size_t parameterCount;
        };
        const ExpectedHelper expectedHelpers[] = {
            {chooseFunction, int4Type, {_getFakeNVVMBuilderBooleanType(), int4Type, int4Type}, 3},
            {floatFunction, float3Type, {float3Type}, 1},
            {boolFunction, bool2Type, {bool2Type}, 1},
        };
        for (const auto& helper : expectedHelpers)
        {
            const Index functionType = gFakeNVVMBuilder.functionTypeIndices[helper.functionIndex];
            SLANG_CHECK(
                gFakeNVVMBuilder.functionTypeResultKinds[functionType] ==
                FakeNVVMBuilderResultTypeKind::ValueVector);
            SLANG_CHECK(
                gFakeNVVMBuilder.functionTypeResultTypes[functionType] == helper.resultType);
            SLANG_CHECK(
                gFakeNVVMBuilder.functionTypeParameterCounts[functionType] ==
                helper.parameterCount);
            const Index parameterOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionType];
            for (Index parameterIndex = 0; parameterIndex < Index(helper.parameterCount);
                 ++parameterIndex)
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder.functionParameterTypes[parameterOffset + parameterIndex] ==
                    helper.parameterTypes[parameterIndex]);
            }
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.callResultTypes.getCount() == 3);
        bool sawInt4Call = false;
        bool sawFloat3Call = false;
        bool sawBool2Call = false;
        for (SlangNVVMTypeHandle callType : gFakeNVVMBuilder.callResultTypes)
        {
            sawInt4Call |= callType == int4Type;
            sawFloat3Call |= callType == float3Type;
            sawBool2Call |= callType == bool2Type;
        }
        SLANG_CHECK(sawInt4Call);
        SLANG_CHECK(sawFloat3Call);
        SLANG_CHECK(sawBool2Call);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes.getCount() >= 1);
        Index int4Phi = -1;
        for (Index phiIndex = 0; phiIndex < gFakeNVVMBuilder.scalarPhiTypes.getCount(); ++phiIndex)
        {
            if (gFakeNVVMBuilder.scalarPhiTypes[phiIndex] == int4Type)
                int4Phi = phiIndex;
        }
        SLANG_CHECK_ABORT(int4Phi >= 0);
        Index int4IncomingCount = 0;
        for (Index incomingIndex = 0;
             incomingIndex < gFakeNVVMBuilder.scalarPhiIncomingPhiIndices.getCount();
             ++incomingIndex)
        {
            if (gFakeNVVMBuilder.scalarPhiIncomingPhiIndices[incomingIndex] == int4Phi)
                ++int4IncomingCount;
        }
        SLANG_CHECK(int4IncomingCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.addPhiIncomingCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    const char* expectedDiagnostics[] = {"invalid vector element count"};
    Index unsupportedIndex = 0;
    for (const char* source : kDirectNVVMUnsupportedVectorFunctionSources)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics)));
            SLANG_CHECK(code == nullptr);
            SLANG_CHECK(
                _getBlobText(diagnostics).indexOf(expectedDiagnostics[unsupportedIndex]) >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
        ++unsupportedIndex;
    }
}

SLANG_UNIT_TEST(nvvmSlangPreservesFunctionContracts)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFunctionContractSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.functionNames.getCount() == 4);
        SLANG_CHECK(gFakeNVVMBuilder.functionLinkages.getCount() == 4);
        SLANG_CHECK(gFakeNVVMBuilder.functionFlags.getCount() == 4);

        Index entryIndex = -1;
        Index helperIndex = -1;
        Index plainIndex = -1;
        Index exportIndex = -1;
        for (Index functionIndex = 0; functionIndex < gFakeNVVMBuilder.functionNames.getCount();
             ++functionIndex)
        {
            const String& name = gFakeNVVMBuilder.functionNames[functionIndex];
            if (name == "computeMain")
                entryIndex = functionIndex;
            else if (name == "exportedFunc")
                exportIndex = functionIndex;
            else if (name.indexOf("helperFunc") >= 0)
                helperIndex = functionIndex;
            else if (name.indexOf("plainHelper") >= 0)
                plainIndex = functionIndex;
        }
        SLANG_CHECK_ABORT(entryIndex >= 0);
        SLANG_CHECK_ABORT(helperIndex >= 0);
        SLANG_CHECK_ABORT(plainIndex >= 0);
        SLANG_CHECK_ABORT(exportIndex >= 0);

        SLANG_CHECK(gFakeNVVMBuilder.functionLinkages[entryIndex] == SLANG_NVVM_LINKAGE_EXTERNAL);
        SLANG_CHECK(gFakeNVVMBuilder.functionFlags[entryIndex] == SLANG_NVVM_FUNCTION_FLAG_NONE);
        SLANG_CHECK(gFakeNVVMBuilder.functionLinkages[helperIndex] == SLANG_NVVM_LINKAGE_INTERNAL);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionFlags[helperIndex] == SLANG_NVVM_FUNCTION_FLAG_NO_INLINE);
        SLANG_CHECK(gFakeNVVMBuilder.functionLinkages[plainIndex] == SLANG_NVVM_LINKAGE_INTERNAL);
        SLANG_CHECK(gFakeNVVMBuilder.functionFlags[plainIndex] == SLANG_NVVM_FUNCTION_FLAG_NONE);
        SLANG_CHECK(gFakeNVVMBuilder.functionLinkages[exportIndex] == SLANG_NVVM_LINKAGE_EXTERNAL);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionFlags[exportIndex] == SLANG_NVVM_FUNCTION_FLAG_NO_INLINE);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices[0] == entryIndex);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangPointerOffsetUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMPointerOffsetSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 3);
        const Index parameterKindOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 2] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetElementValueRefs.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetCallerBlockIndices.getCount() == 2);
        Index destinationOffsetIndex = -1;
        Index sourceOffsetIndex = -1;
        for (Index offsetIndex = 0;
             offsetIndex < gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount();
             ++offsetIndex)
        {
            const FakeNVVMBuilderValueRef base =
                gFakeNVVMBuilder.pointerOffsetBaseValueRefs[offsetIndex];
            const FakeNVVMBuilderValueRef element =
                gFakeNVVMBuilder.pointerOffsetElementValueRefs[offsetIndex];
            SLANG_CHECK(base.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(base.functionIndex == 0);
            SLANG_CHECK(element.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(element.functionIndex == 0);
            SLANG_CHECK(element.index == 2);
            SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetCallerBlockIndices[offsetIndex] == 0);
            if (base.index == 0)
                destinationOffsetIndex = offsetIndex;
            else if (base.index == 1)
                sourceOffsetIndex = offsetIndex;
            else
                SLANG_CHECK(false);
        }
        SLANG_CHECK_ABORT(destinationOffsetIndex >= 0);
        SLANG_CHECK_ABORT(sourceOffsetIndex >= 0);
        SLANG_CHECK(destinationOffsetIndex != sourceOffsetIndex);

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags[0] == SLANG_NVVM_LOAD_FLAG_NONE);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs[0].index == sourceOffsetIndex);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == destinationOffsetIndex);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Load);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);

        SLANG_CHECK(
            gFakeNVVMBuilder.scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)] ==
            0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFixedDeviceArrayUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFixedDeviceArraySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementType == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes[0] == _getFakeNVVMBuilderArrayType());
        SLANG_CHECK(gFakeNVVMBuilder.pointerAddressSpaces.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerAddressSpaces[0] == SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 3);
        const Index parameterKindOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset] ==
            FakeNVVMBuilderParameterTypeKind::ArrayPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::ArrayPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 2] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.sequentialElementPointerBaseValueRefs.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.sequentialElementPointerIndexValueRefs.getCount() == 2);
        for (Index elementIndex = 0; elementIndex < 2; ++elementIndex)
        {
            const FakeNVVMBuilderValueRef base =
                gFakeNVVMBuilder.sequentialElementPointerBaseValueRefs[elementIndex];
            const FakeNVVMBuilderValueRef index =
                gFakeNVVMBuilder.sequentialElementPointerIndexValueRefs[elementIndex];
            SLANG_CHECK(base.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(base.functionIndex == 0);
            SLANG_CHECK(base.index == elementIndex);
            SLANG_CHECK(index.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(index.functionIndex == 0);
            SLANG_CHECK(index.index == 2);
            SLANG_CHECK(
                gFakeNVVMBuilder.sequentialElementPointerCallerBlockIndices[elementIndex] == 0);
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::SequentialElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs[0].index == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::SequentialElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Load);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangUnsignedConstantPointerIndicesUseDirectPipeline)
{
    struct IndexCase
    {
        const char* source;
        bool isArray;
    };
    const IndexCase cases[] = {
        {kDirectNVVMUnsignedPointerOffsetSource, false},
        {kDirectNVVMUnsignedFixedArrayIndexSource, true},
    };

    for (const IndexCase& indexCase : cases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                _compileSlangWithDirectNVVM(globalSession, indexCase.source, code, diagnostics)));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

            const List<FakeNVVMBuilderValueRef>& indices =
                indexCase.isArray ? gFakeNVVMBuilder.sequentialElementPointerIndexValueRefs
                                  : gFakeNVVMBuilder.pointerOffsetElementValueRefs;
            SLANG_CHECK(indices.getCount() == 2);
            for (const FakeNVVMBuilderValueRef index : indices)
            {
                SLANG_CHECK(index.kind == FakeNVVMBuilderValueKind::IntegerConstant);
                SLANG_CHECK(
                    index.index < size_t(gFakeNVVMBuilder.integerConstantValues.getCount()));
                SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[index.index] == 1);
                SLANG_CHECK(gFakeNVVMBuilder.integerConstantBitWidths[index.index] == 32);
            }
            SLANG_CHECK(
                gFakeNVVMBuilder.emitSequentialElementPointerCallCount ==
                (indexCase.isArray ? 2 : 0));
            SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == (indexCase.isArray ? 0 : 2));
            SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangRawRWStructuredBufferI32StoreUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMRawRWStructuredBufferI32StoreSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getStructTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 2);
        const Index parameterKindOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset] ==
            FakeNVVMBuilderParameterTypeKind::ResourceView);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.aggregateElementBaseValueRefs.getCount() == 1);
        const FakeNVVMBuilderValueRef buffer = gFakeNVVMBuilder.aggregateElementBaseValueRefs[0];
        SLANG_CHECK(buffer.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(buffer.functionIndex == 0);
        SLANG_CHECK(buffer.index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.aggregateElementIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::AggregateElement);
        const FakeNVVMBuilderValueRef index = gFakeNVVMBuilder.pointerOffsetElementValueRefs[0];
        SLANG_CHECK(index.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(index.functionIndex == 0);
        SLANG_CHECK(index.index == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetCallerBlockIndices[0] == 0);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == 42);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRawRWStructuredBufferU32AtomicAddUsesGenericInterface)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMRawRWStructuredBufferU32AtomicAddSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.atomicOperations.getCount() == 1);
        const SlangNVVMAtomicOperationDesc& operation = gFakeNVVMBuilder.atomicOperations[0];
        SLANG_CHECK(operation.operation == SLANG_NVVM_ATOMIC_OP_ADD);
        SLANG_CHECK(NVVMSemantics::areSameType(operation.valueType, NVVMSemantics::kUnsignedI32));
        SLANG_CHECK(operation.addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
        SLANG_CHECK(operation.memoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED);
        SLANG_CHECK(gFakeNVVMBuilder.atomicOperationPointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.atomicOperationPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(gFakeNVVMBuilder.atomicOperationValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.atomicOperationValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangMixedWidthByteAddressAtomicsUseTypedViews)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMMixedWidthByteAddressAtomicSource,
            code,
            diagnostics,
            "cuda_sm_9_0");
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.atomicOperations.getCount() == 2);
        const SlangNVVMAtomicOperationDesc* maxOperation = nullptr;
        const SlangNVVMAtomicOperationDesc* addOperation = nullptr;
        for (const auto& operation : gFakeNVVMBuilder.atomicOperations)
        {
            if (operation.operation == SLANG_NVVM_ATOMIC_OP_MAX)
                maxOperation = &operation;
            if (operation.operation == SLANG_NVVM_ATOMIC_OP_ADD)
                addOperation = &operation;
        }
        SLANG_CHECK_ABORT(maxOperation != nullptr);
        SLANG_CHECK_ABORT(addOperation != nullptr);
        SLANG_CHECK(
            NVVMSemantics::areSameType(maxOperation->valueType, NVVMSemantics::kUnsignedI64));
        SLANG_CHECK(maxOperation->addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
        SLANG_CHECK(maxOperation->memoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED);
        SLANG_CHECK(
            NVVMSemantics::areSameType(addOperation->valueType, NVVMSemantics::kUnsignedI32));
        SLANG_CHECK(addOperation->addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
        SLANG_CHECK(addOperation->memoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED);

        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateConstructCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.aggregateConstructElementCounts.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.aggregateConstructElementCounts[0] == 2);
        SLANG_CHECK(gFakeNVVMBuilder.aggregateConstructElementValueRefs.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.aggregateConstructElementValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ByteOffsetPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.aggregateConstructElementValueRefs[1].kind ==
            FakeNVVMBuilderValueKind::AggregateElement);
        SLANG_CHECK(gFakeNVVMBuilder.emitByteOffsetPointerCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.byteOffsetPointerPointeeTypes.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.byteOffsetPointerPointeeTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.atomicOperationPointerValueRefs.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.atomicOperationPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(
            gFakeNVVMBuilder.atomicOperationPointerValueRefs[1].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(gFakeNVVMBuilder.atomicOperationValueRefs.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.atomicOperationValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(
            gFakeNVVMBuilder.atomicOperationValueRefs[1].kind ==
            FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRawBufferViewsCrossHelperParametersByValue)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMRawBufferHelperSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.callCalleeFunctionIndices.getCount() == 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices.getCount() == 1);
        const Index kernelFunction = gFakeNVVMBuilder.kernelFunctionIndices[0];
        SLANG_CHECK_ABORT(kernelFunction >= 0);
        SLANG_CHECK_ABORT(kernelFunction < gFakeNVVMBuilder.functionTypeIndices.getCount());

        for (Index functionIndex = 0;
             functionIndex < gFakeNVVMBuilder.functionTypeIndices.getCount();
             ++functionIndex)
        {
            if (functionIndex == kernelFunction)
                continue;
            const Index functionType = gFakeNVVMBuilder.functionTypeIndices[functionIndex];
            SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionType] >= 1);
            const Index parameterOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionType];
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[parameterOffset] ==
                FakeNVVMBuilderParameterTypeKind::ResourceView);
        }

        bool sawResourceArguments[4] = {};
        for (Index callIndex = 0; callIndex < gFakeNVVMBuilder.callArgumentOffsets.getCount();
             ++callIndex)
        {
            const Index callerBlock = gFakeNVVMBuilder.callCallerBlockIndices[callIndex];
            SLANG_CHECK(gFakeNVVMBuilder.blockFunctionIndices[callerBlock] == kernelFunction);
            const FakeNVVMBuilderValueRef resourceArgument =
                gFakeNVVMBuilder
                    .callArgumentValueRefs[gFakeNVVMBuilder.callArgumentOffsets[callIndex]];
            SLANG_CHECK(resourceArgument.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(resourceArgument.functionIndex == kernelFunction);
            SLANG_CHECK(resourceArgument.index >= 0 && resourceArgument.index < 4);
            if (resourceArgument.index >= 0 && resourceArgument.index < 4)
                sawResourceArguments[resourceArgument.index] = true;
        }
        for (bool sawResourceArgument : sawResourceArguments)
            SLANG_CHECK(sawResourceArgument);

        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateConstructCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.atomicOperations.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.atomicOperationPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 2);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangResourceViewsCrossHelperResultsByValue)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMResourceResultHelperSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        Index rawBufferHelper = -1;
        Index textureHelper = -1;
        for (Index functionIndex = 0; functionIndex < gFakeNVVMBuilder.functionNames.getCount();
             ++functionIndex)
        {
            const String& name = gFakeNVVMBuilder.functionNames[functionIndex];
            if (name.indexOf("preserveDestination") >= 0)
                rawBufferHelper = functionIndex;
            else if (name.indexOf("preserveTexture") >= 0)
                textureHelper = functionIndex;
        }
        SLANG_CHECK_ABORT(rawBufferHelper >= 0);
        SLANG_CHECK_ABORT(textureHelper >= 0);

        const Index rawBufferFunctionType = gFakeNVVMBuilder.functionTypeIndices[rawBufferHelper];
        const Index textureFunctionType = gFakeNVVMBuilder.functionTypeIndices[textureHelper];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[rawBufferFunctionType] ==
            FakeNVVMBuilderResultTypeKind::ResourceView);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultTypes[rawBufferFunctionType] ==
            _getFakeNVVMBuilderResourceViewType(FakeNVVMBuilderScalarTypeKind::Float));
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[textureFunctionType] ==
            FakeNVVMBuilderResultTypeKind::Integer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionFlags[rawBufferHelper] == SLANG_NVVM_FUNCTION_FLAG_NO_INLINE);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionFlags[textureHelper] == SLANG_NVVM_FUNCTION_FLAG_NO_INLINE);

        bool sawRawBufferCall = false;
        bool sawTextureCall = false;
        Index rawBufferCall = -1;
        for (Index callIndex = 0; callIndex < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount();
             ++callIndex)
        {
            if (gFakeNVVMBuilder.callCalleeFunctionIndices[callIndex] == rawBufferHelper)
            {
                sawRawBufferCall = true;
                rawBufferCall = callIndex;
                SLANG_CHECK(
                    gFakeNVVMBuilder.callResultKinds[callIndex] ==
                    FakeNVVMBuilderResultTypeKind::ResourceView);
            }
            if (gFakeNVVMBuilder.callCalleeFunctionIndices[callIndex] == textureHelper)
            {
                sawTextureCall = true;
                SLANG_CHECK(
                    gFakeNVVMBuilder.callResultKinds[callIndex] ==
                    FakeNVVMBuilderResultTypeKind::Integer);
            }
        }
        SLANG_CHECK(sawRawBufferCall);
        SLANG_CHECK(sawTextureCall);

        bool sawRawBufferCallExtraction = false;
        for (const auto& base : gFakeNVVMBuilder.aggregateElementBaseValueRefs)
        {
            sawRawBufferCallExtraction |=
                base.kind == FakeNVVMBuilderValueKind::Call && base.index == rawBufferCall;
        }
        SLANG_CHECK(sawRawBufferCallExtraction);
        SLANG_CHECK(gFakeNVVMBuilder.textureOperations.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRawBufferDataPointersUseGenericPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMRawBufferDataPointerSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 4);
        const Index parameterOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        for (Index parameterIndex = 0; parameterIndex < 3; ++parameterIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[parameterOffset + parameterIndex] ==
                FakeNVVMBuilderParameterTypeKind::ResourceView);
        }
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterOffset + 3] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 3);
        bool sawBufferParameters[3] = {};
        for (Index fieldValueIndex = 0; fieldValueIndex < 3; ++fieldValueIndex)
        {
            const FakeNVVMBuilderValueRef base =
                gFakeNVVMBuilder.aggregateElementBaseValueRefs[fieldValueIndex];
            SLANG_CHECK(base.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(base.functionIndex == 0);
            SLANG_CHECK(base.index >= 0 && base.index < 3);
            sawBufferParameters[base.index] = true;
            SLANG_CHECK(gFakeNVVMBuilder.aggregateElementIndices[fieldValueIndex] == 0);
        }
        for (bool sawParameter : sawBufferParameters)
            SLANG_CHECK(sawParameter);

        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 3);
        for (Index pointerIndex = 0; pointerIndex < 3; ++pointerIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.pointerOffsetBaseValueRefs[pointerIndex].kind ==
                FakeNVVMBuilderValueKind::AggregateElement);
            const FakeNVVMBuilderValueRef index =
                gFakeNVVMBuilder.pointerOffsetElementValueRefs[pointerIndex];
            SLANG_CHECK(index.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(index.functionIndex == 0);
            SLANG_CHECK(index.index == 3);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 0);

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 2);
        for (Index loadIndex = 0; loadIndex < 2; ++loadIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.loadPointerValueRefs[loadIndex].kind ==
                FakeNVVMBuilderValueKind::PointerOffset);
            SLANG_CHECK(gFakeNVVMBuilder.loadFlags[loadIndex] == SLANG_NVVM_LOAD_FLAG_NONE);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);

        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangReadOnlyByteAddressDataPointerIsInvariant)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMReadOnlyByteAddressDataPointerSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags[0] == SLANG_NVVM_LOAD_FLAG_INVARIANT);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCoreByteAddressAccessUsesGenericByteOffsets)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCoreByteAddressAccessSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.emitByteOffsetPointerCallCount == 3);
        SLANG_CHECK(
            gFakeNVVMBuilder.byteOffsetPointerPointeeTypes[0] == _getFakeNVVMBuilderVectorType(4));
        SLANG_CHECK(
            gFakeNVVMBuilder.byteOffsetPointerPointeeTypes[1] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(
            gFakeNVVMBuilder.byteOffsetPointerPointeeTypes[2] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags[0] == SLANG_NVVM_LOAD_FLAG_INVARIANT);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags[1] == SLANG_NVVM_LOAD_FLAG_NONE);
        // Generic byte-address legalization currently canonicalizes this source overload to the
        // two-operand load form, whose remaining contract is four-byte alignment.
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[0] == 4);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[1] == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignments[0] == 4);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignments[1] == 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFloatVectorByteAddressAccessUsesGenericOperations)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloatVectorByteAddressAccessSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getVectorTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementType == _getFakeNVVMBuilderFloatType());
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitVectorConstructCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.emitByteOffsetPointerCallCount == 12);
        Index integerPointerCount = 0;
        Index floatPointerCount = 0;
        Index float4PointerCount = 0;
        for (auto typeKind : gFakeNVVMBuilder.byteOffsetPointerTypeKinds)
        {
            if (typeKind == FakeNVVMBuilderScalarTypeKind::Integer)
                ++integerPointerCount;
            else if (typeKind == FakeNVVMBuilderScalarTypeKind::Float)
                ++floatPointerCount;
            else if (typeKind == FakeNVVMBuilderScalarTypeKind::Float4)
                ++float4PointerCount;
        }
        SLANG_CHECK(integerPointerCount == 2);
        SLANG_CHECK(floatPointerCount == 8);
        SLANG_CHECK(float4PointerCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 6);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 7);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangWideIntegerByteAddressAccessUsesGenericOperations)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMWideIntegerByteAddressAccessSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.emitByteOffsetPointerCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags[0] == SLANG_NVVM_LOAD_FLAG_INVARIANT);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags[1] == SLANG_NVVM_LOAD_FLAG_NONE);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[0] == 8);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[1] == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignments[0] == 8);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignments[1] == 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNumericArrayByteAddressAccessUsesGenericOperations)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMNumericArrayByteAddressAccessSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.arrayElementType ==
            _getFakeNVVMBuilderVectorType(4, FakeNVVMBuilderScalarTypeKind::Float));
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitByteOffsetPointerCallCount == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.byteOffsetPointerTypeKinds[0] ==
            FakeNVVMBuilderScalarTypeKind::NumericArray);
        SLANG_CHECK(
            gFakeNVVMBuilder.byteOffsetPointerTypeKinds[1] ==
            FakeNVVMBuilderScalarTypeKind::NumericArray);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags[0] == SLANG_NVVM_LOAD_FLAG_INVARIANT);
        // Generic aggregate legalization canonicalizes the retained wide load to the ordinary
        // two-operand byte-load form, whose remaining alignment contract is four bytes.
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[0] == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignments[0] == 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRejectsNestedArrayByteAddressAccessBeforeProviderMutation)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMUnsupportedNestedArrayByteAddressAccessSource,
            code,
            diagnostics);
        SLANG_CHECK(SLANG_FAILED(result));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("core byte-address buffer access") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRejectsReadOnlyByteAddressDataPointerStoreBeforeProviderMutation)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMReadOnlyByteAddressStoreSource,
            code,
            diagnostics);
        SLANG_CHECK(SLANG_FAILED(result));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("store to immutable location") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangAggregateAndReadOnlyResourceUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMAggregateAndReadOnlyResourceSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 4);
        const Index parameterKindOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        const FakeNVVMBuilderParameterTypeKind expectedParameterKinds[] = {
            FakeNVVMBuilderParameterTypeKind::ScalarStructPointer,
            FakeNVVMBuilderParameterTypeKind::ResourceView,
            FakeNVVMBuilderParameterTypeKind::ResourceView,
            FakeNVVMBuilderParameterTypeKind::Integer,
        };
        for (Index parameterIndex = 0; parameterIndex < SLANG_COUNT_OF(expectedParameterKinds);
             ++parameterIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + parameterIndex] ==
                expectedParameterKinds[parameterIndex]);
        }

        SLANG_CHECK(gFakeNVVMBuilder.setFunctionParameterAttributesCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.parameterAttributeFunctionIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.parameterAttributeFunctionIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.parameterAttributeIndices[0] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.parameterAttributeFlags[0] == SLANG_NVVM_PARAMETER_FLAG_BY_VALUE);
        SLANG_CHECK(
            gFakeNVVMBuilder.parameterAttributePointeeTypes[0] ==
            _getFakeNVVMBuilderScalarStructType());
        SLANG_CHECK(gFakeNVVMBuilder.parameterAttributeAlignments[0] == 8);

        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 2);
        const uint32_t expectedAggregateFieldIndices[] = {0, 1};
        for (Index fieldIndex = 0; fieldIndex < SLANG_COUNT_OF(expectedAggregateFieldIndices);
             ++fieldIndex)
        {
            const FakeNVVMBuilderValueRef base =
                gFakeNVVMBuilder.structFieldPointerBaseValueRefs[fieldIndex];
            SLANG_CHECK(base.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(base.functionIndex == 0);
            SLANG_CHECK(base.index == 0);
            SLANG_CHECK(
                gFakeNVVMBuilder.structFieldPointerIndices[fieldIndex] ==
                expectedAggregateFieldIndices[fieldIndex]);
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 2);
        bool sawDestinationView = false;
        bool sawSourceView = false;
        for (const FakeNVVMBuilderValueRef base : gFakeNVVMBuilder.aggregateElementBaseValueRefs)
        {
            SLANG_CHECK(base.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(base.functionIndex == 0);
            sawDestinationView = sawDestinationView || base.index == 1;
            sawSourceView = sawSourceView || base.index == 2;
        }
        SLANG_CHECK(sawDestinationView);
        SLANG_CHECK(sawSourceView);

        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.loadFlags.getCount() == 3);
        for (SlangNVVMLoadFlags flags : gFakeNVVMBuilder.loadFlags)
            SLANG_CHECK(flags == SLANG_NVVM_LOAD_FLAG_INVARIANT);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangVectorStructuredBuffersUseGenericTransport)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMVectorStructuredBufferSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 3);
        const Index parameterOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        for (Index parameterIndex = 0; parameterIndex < 3; ++parameterIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[parameterOffset + parameterIndex] ==
                FakeNVVMBuilderParameterTypeKind::ResourceView);
        }
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[parameterOffset] ==
            _getFakeNVVMBuilderResourceViewType(FakeNVVMBuilderScalarTypeKind::UInt4));
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[parameterOffset + 1] ==
            _getFakeNVVMBuilderResourceViewType(FakeNVVMBuilderScalarTypeKind::Float4));
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[parameterOffset + 2] ==
            _getFakeNVVMBuilderResourceViewType());

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::UInt4);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[1] == FakeNVVMBuilderScalarTypeKind::Float4);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[0] == 16);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[1] == 16);

        SLANG_CHECK(gFakeNVVMBuilder.emitByteOffsetPointerCallCount == 4);
        const int64_t expectedByteOffsets[] = {12, 8, 4, 0};
        for (Index offsetIndex = 0; offsetIndex < SLANG_COUNT_OF(expectedByteOffsets);
             ++offsetIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.byteOffsetPointerTypeKinds[offsetIndex] ==
                FakeNVVMBuilderScalarTypeKind::Float);
            const FakeNVVMBuilderValueRef offset =
                gFakeNVVMBuilder.byteOffsetPointerOffsetValueRefs[offsetIndex];
            SLANG_CHECK(offset.kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK(
                gFakeNVVMBuilder.integerConstantValues[offset.index] ==
                expectedByteOffsets[offsetIndex]);
        }

        bool sawResourceVectorLanePointer = false;
        for (auto resultTypeKind : gFakeNVVMBuilder.sequentialElementPointerTypeKinds)
        {
            sawResourceVectorLanePointer |= resultTypeKind == FakeNVVMBuilderScalarTypeKind::Float;
        }
        SLANG_CHECK(sawResourceVectorLanePointer);

        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 6);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 6);
        for (uint32_t alignment : gFakeNVVMBuilder.storeAlignments)
            SLANG_CHECK(alignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangSelectedNumericStructuredBuffersUseGenericTransport)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMSelectedNumericStructuredBufferSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 2);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 5);
        const Index parameterOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        const FakeNVVMBuilderScalarTypeKind elementTypes[] = {
            FakeNVVMBuilderScalarTypeKind::Half,
            FakeNVVMBuilderScalarTypeKind::Double,
            FakeNVVMBuilderScalarTypeKind::Half2,
            FakeNVVMBuilderScalarTypeKind::Double2,
        };
        for (Index parameterIndex = 0; parameterIndex < SLANG_COUNT_OF(elementTypes);
             ++parameterIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[parameterOffset + parameterIndex] ==
                FakeNVVMBuilderParameterTypeKind::ResourceView);
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypes[parameterOffset + parameterIndex] ==
                _getFakeNVVMBuilderResourceViewType(elementTypes[parameterIndex]));
        }
        SLANG_CHECK(
            gFakeNVVMBuilder
                .functionParameterTypeKinds[parameterOffset + SLANG_COUNT_OF(elementTypes)] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        const Index helperTypeIndex = gFakeNVVMBuilder.functionTypeIndices[1];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[helperTypeIndex] ==
            FakeNVVMBuilderResultTypeKind::Integer);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[helperTypeIndex] == 1);
        const Index helperParameterOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[helperTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[helperParameterOffset] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        bool sawBooleanToHalf = false;
        bool sawBooleanToDouble = false;
        Index halfToPhysicalCount = 0;
        Index physicalToHalfCount = 0;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            if (operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET &&
                operation.operandCount == 1)
            {
                halfToPhysicalCount +=
                    NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kUnsignedI16) &&
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat16);
                physicalToHalfCount +=
                    NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat16) &&
                    NVVMSemantics::areSameType(
                        operation.operandTypes[0],
                        NVVMSemantics::kUnsignedI16);
            }
            if (operation.key.operation != SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT ||
                operation.operandCount != 1 ||
                !NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kBool))
            {
                continue;
            }
            sawBooleanToHalf =
                sawBooleanToHalf ||
                (operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                 operation.resultType.bitWidth == 16 && operation.resultType.laneCount == 1);
            sawBooleanToDouble =
                sawBooleanToDouble ||
                (operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                 operation.resultType.bitWidth == 64 && operation.resultType.laneCount == 1);
        }
        SLANG_CHECK(sawBooleanToHalf);
        SLANG_CHECK(sawBooleanToDouble);
        SLANG_CHECK(halfToPhysicalCount == 2);
        SLANG_CHECK(physicalToHalfCount == 2);

        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 4);
        const uint32_t expectedAlignments[] = {2, 8, 4, 16};
        SLANG_CHECK(
            gFakeNVVMBuilder.storeAlignments.getCount() == SLANG_COUNT_OF(expectedAlignments));
        for (Index storeIndex = 0; storeIndex < SLANG_COUNT_OF(expectedAlignments); ++storeIndex)
            SLANG_CHECK(
                gFakeNVVMBuilder.storeAlignments[storeIndex] == expectedAlignments[storeIndex]);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

static void _runNVVMScalarDirectPipeline(NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    const bool isUnary = testCase.key.family == FakeNVVMBuilderScalarFamily::Unary;
    const bool isCompare = testCase.key.family == FakeNVVMBuilderScalarFamily::Compare;
    const Index parameterCount = isUnary ? 2 : 3;

    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result =
            _compileSlangWithDirectNVVM(globalSession, testCase.source, code, diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == (isCompare ? 4 : 1));
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == parameterCount);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == parameterCount);
        const Index parameterKindOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        for (Index i = 1; i < parameterCount; ++i)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + i] ==
                FakeNVVMBuilderParameterTypeKind::Integer);
        }

        SLANG_CHECK(
            _getFakeNVVMBuilderScalarOperationCallCount(
                testCase.key.family,
                testCase.key.operation) == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 1);
        const FakeNVVMBuilderScalarOperation& recorded = gFakeNVVMBuilder.scalarOperations[0];
        SLANG_CHECK(_isFakeNVVMBuilderScalarOperation(
            recorded.key,
            testCase.key.family,
            testCase.key.operation));
        SLANG_CHECK(
            recorded.callerBlockIndex ==
            (isCompare ? gFakeNVVMBuilder.conditionalSourceBlockIndex : 0));
        SLANG_CHECK(recorded.operandCount == uint32_t(parameterCount - 1));
        for (Index i = 0; i < parameterCount - 1; ++i)
        {
            SLANG_CHECK(recorded.operands[i].kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(recorded.operands[i].functionIndex == 0);
            SLANG_CHECK(recorded.operands[i].index == i + 1);
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind ==
            (isCompare ? FakeNVVMBuilderValueKind::ScalarPhi
                       : FakeNVVMBuilderValueKind::ScalarOperation));
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == (isCompare ? 1 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == (isCompare ? 2 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount == (isCompare ? 1 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.addPhiIncomingCallCount == (isCompare ? 2 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == (isCompare ? 2 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

#define NVVM_SCALAR_DIRECT_TEST(NAME, OPERATION)                          \
    SLANG_UNIT_TEST(NAME)                                                 \
    {                                                                     \
        _runNVVMScalarDirectPipeline(NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerMultiplyUsesDirectPipeline, Multiply)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerBitAndUsesDirectPipeline, BitAnd)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerBitOrUsesDirectPipeline, BitOr)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerBitXorUsesDirectPipeline, BitXor)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerBitNotUsesDirectPipeline, BitNot)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerNegateUsesDirectPipeline, Negate)
SLANG_UNIT_TEST(nvvmSlangRelaxedGlobalI32AtomicAddUsesDirectPipeline)
{
    struct DirectCase
    {
        const char* source;
        Index parameterCount;
        bool consumesOldValue;
        SlangNVVMValueTypeDesc valueType;
    };
    static const DirectCase kCases[] = {
        {kDirectNVVMRelaxedGlobalI32AtomicAddSource, 1, false, NVVMSemantics::kSignedI32},
        {kDirectNVVMRelaxedGlobalI32AtomicAddOldValueSource, 2, true, NVVMSemantics::kSignedI32},
        {kDirectNVVMUnsignedAtomicAddSource, 1, false, NVVMSemantics::kUnsignedI32},
    };

    for (const auto& directCase : kCases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult result =
                _compileSlangWithDirectNVVM(globalSession, directCase.source, code, diagnostics);
            if (SLANG_FAILED(result))
            {
                const String diagnosticText = _getBlobText(diagnostics);
                if (diagnosticText.getLength())
                    getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

            SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
            SLANG_CHECK(
                gFakeNVVMBuilder.getFunctionParameterCallCount == directCase.parameterCount);
            SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
            const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
            SLANG_CHECK(
                gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
                FakeNVVMBuilderResultTypeKind::Void);
            SLANG_CHECK(
                gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] ==
                size_t(directCase.parameterCount));
            const Index parameterKindOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
            for (Index parameterIndex = 0; parameterIndex < directCase.parameterCount;
                 ++parameterIndex)
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder
                        .functionParameterTypeKinds[parameterKindOffset + parameterIndex] ==
                    FakeNVVMBuilderParameterTypeKind::Pointer);
            }

            SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues.getCount() == 1);
            SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == 1);
            SLANG_CHECK(gFakeNVVMBuilder.emitAtomicOperationCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.atomicOperations.getCount() == 1);
            SLANG_CHECK(gFakeNVVMBuilder.atomicOperations[0].operation == SLANG_NVVM_ATOMIC_OP_ADD);
            SLANG_CHECK(NVVMSemantics::areSameType(
                gFakeNVVMBuilder.atomicOperations[0].valueType,
                directCase.valueType));
            SLANG_CHECK(
                gFakeNVVMBuilder.atomicOperations[0].addressSpace ==
                SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
            SLANG_CHECK(
                gFakeNVVMBuilder.atomicOperations[0].memoryOrder ==
                SLANG_NVVM_MEMORY_ORDER_RELAXED);
            SLANG_CHECK(gFakeNVVMBuilder.atomicOperationCallerBlockIndices.getCount() == 1);
            SLANG_CHECK(gFakeNVVMBuilder.atomicOperationCallerBlockIndices[0] == 0);
            SLANG_CHECK(gFakeNVVMBuilder.atomicOperationPointerValueRefs.getCount() == 1);
            const FakeNVVMBuilderValueRef pointer =
                gFakeNVVMBuilder.atomicOperationPointerValueRefs[0];
            SLANG_CHECK(pointer.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(pointer.functionIndex == 0);
            SLANG_CHECK(pointer.index == 0);
            SLANG_CHECK(gFakeNVVMBuilder.atomicOperationValueRefs.getCount() == 1);
            const FakeNVVMBuilderValueRef value = gFakeNVVMBuilder.atomicOperationValueRefs[0];
            SLANG_CHECK(value.kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK(value.index == 0);

            SLANG_CHECK(
                gFakeNVVMBuilder.emitStoreCallCount == (directCase.consumesOldValue ? 1 : 0));
            if (directCase.consumesOldValue)
            {
                SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].functionIndex == 0);
                SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 1);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::AtomicOperation);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
                SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
            }

            SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
            SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangRelaxedSharedI32AtomicAddUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMGroupSharedI32AtomicAddSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder trace;
            trace << "shared atomic fake trace: result " << result << "; modules "
                  << gFakeNVVMBuilder.createModuleCallCount << "; globals "
                  << gFakeNVVMBuilder.declareGlobalStorageCallCount << "; atomics "
                  << gFakeNVVMBuilder.emitAtomicOperationCallCount << "; serializations "
                  << gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount;
            getTestReporter()->message(TestMessageType::TestFailure, trace.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageValueType == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageLinkage == SLANG_NVVM_LINKAGE_INTERNAL);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageAddressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageAlignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageNames.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageNames[0].indexOf("atomicCounter") >= 0);

        SLANG_CHECK(gFakeNVVMBuilder.atomicOperations.getCount() == 1);
        const SlangNVVMAtomicOperationDesc& operation = gFakeNVVMBuilder.atomicOperations[0];
        SLANG_CHECK(operation.operation == SLANG_NVVM_ATOMIC_OP_ADD);
        SLANG_CHECK(NVVMSemantics::areSameType(operation.valueType, NVVMSemantics::kSignedI32));
        SLANG_CHECK(operation.addressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED);
        SLANG_CHECK(operation.memoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED);
        SLANG_CHECK(gFakeNVVMBuilder.atomicOperationPointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.atomicOperationPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::GlobalStorage);
        SLANG_CHECK(gFakeNVVMBuilder.atomicOperationValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.atomicOperationValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementPointerCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCommonSharedAtomicAlgebraUsesOneTypedInterface)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCommonSharedAtomicAlgebraSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        size_t operationCounts[SLANG_NVVM_ATOMIC_OPERATION_COUNT] = {};
        SLANG_CHECK(gFakeNVVMBuilder.atomicOperations.getCount() == 13);
        for (Index i = 0; i < gFakeNVVMBuilder.atomicOperations.getCount(); ++i)
        {
            const SlangNVVMAtomicOperationDesc& operation = gFakeNVVMBuilder.atomicOperations[i];
            SLANG_CHECK(operation.operation < SLANG_NVVM_ATOMIC_OPERATION_COUNT);
            ++operationCounts[operation.operation];
            SLANG_CHECK(NVVMSemantics::areSameType(operation.valueType, NVVMSemantics::kSignedI32));
            SLANG_CHECK(operation.addressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED);
            SLANG_CHECK(operation.memoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED);
            SLANG_CHECK(operation.failureMemoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED);
            SLANG_CHECK(
                gFakeNVVMBuilder.atomicOperationPointerValueRefs[i].kind ==
                FakeNVVMBuilderValueKind::GlobalStorage);

            const size_t expectedValueCount =
                operation.operation == SLANG_NVVM_ATOMIC_OP_LOAD               ? 0
                : operation.operation == SLANG_NVVM_ATOMIC_OP_COMPARE_EXCHANGE ? 2
                                                                               : 1;
            SLANG_CHECK(gFakeNVVMBuilder.atomicOperationValueCounts[i] == expectedValueCount);
        }
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_LOAD] == 1);
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_STORE] == 1);
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_EXCHANGE] == 1);
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_COMPARE_EXCHANGE] == 1);
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_ADD] == 4);
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_MIN] == 1);
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_MAX] == 1);
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_BIT_AND] == 1);
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_BIT_OR] == 1);
        SLANG_CHECK(operationCounts[SLANG_NVVM_ATOMIC_OP_BIT_XOR] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitAtomicOperationCallCount == 13);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangAtomicReductionsUseCanonicalOperations)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMAtomicReductionSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        bool sawUnsignedXor = false;
        bool sawFloat32Add = false;
        bool sawFloat64Add = false;
        bool sawHalf2Add = false;
        for (const SlangNVVMAtomicOperationDesc& operation : gFakeNVVMBuilder.atomicOperations)
        {
            SLANG_CHECK(operation.addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
            SLANG_CHECK(operation.memoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED);
            sawUnsignedXor |=
                operation.operation == SLANG_NVVM_ATOMIC_OP_BIT_XOR &&
                NVVMSemantics::areSameType(operation.valueType, NVVMSemantics::kUnsignedI32);
            sawFloat32Add |=
                operation.operation == SLANG_NVVM_ATOMIC_OP_ADD &&
                NVVMSemantics::areSameType(operation.valueType, NVVMSemantics::kFloat32);
            sawFloat64Add |=
                operation.operation == SLANG_NVVM_ATOMIC_OP_ADD &&
                NVVMSemantics::areSameType(operation.valueType, NVVMSemantics::kFloat64);
            sawHalf2Add |= operation.operation == SLANG_NVVM_ATOMIC_OP_ADD &&
                           operation.valueType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                           operation.valueType.bitWidth == 16 && operation.valueType.laneCount == 2;
        }
        SLANG_CHECK(gFakeNVVMBuilder.atomicOperations.getCount() == 4);
        SLANG_CHECK(sawUnsignedXor);
        SLANG_CHECK(sawFloat32Add);
        SLANG_CHECK(sawFloat64Add);
        SLANG_CHECK(sawHalf2Add);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerAddressSpaceCastCallCount == 0);
        for (const FakeNVVMBuilderValueRef& pointer :
             gFakeNVVMBuilder.atomicOperationPointerValueRefs)
        {
            SLANG_CHECK(pointer.kind == FakeNVVMBuilderValueKind::PointerOffset);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 0);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFiniteGroupsharedValuesCrossHelperPointers)
{
    static const char* kSources[] = {
        kDirectNVVMSharedHelperPointerSource,
        kDirectNVVMSharedFloatArraySource,
    };
    for (const char* source : kSources)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult result =
                _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics);
            if (SLANG_FAILED(result))
            {
                const String diagnosticText = _getBlobText(diagnostics);
                if (diagnosticText.getLength())
                    getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(gFakeNVVMBuilder.declareGlobalStorageCallCount == 1);
            SLANG_CHECK(
                gFakeNVVMBuilder.globalStorageAddressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED);
            SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangSelectedWideAndFloatingAtomicAddsUseTypedOperations)
{
    struct DirectCase
    {
        const char* source;
        SlangNVVMValueTypeDesc valueType;
    };
    static const DirectCase kCases[] = {
        {kDirectNVVMWideAtomicAddSource, {SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER, 64, 1}},
        {kDirectNVVMFloatingAtomicAddSource, NVVMSemantics::kFloat32},
    };
    for (const DirectCase& directCase : kCases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult result =
                _compileSlangWithDirectNVVM(globalSession, directCase.source, code, diagnostics);
            if (SLANG_FAILED(result))
            {
                const String diagnosticText = _getBlobText(diagnostics);
                if (diagnosticText.getLength())
                    getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(gFakeNVVMBuilder.atomicOperations.getCount() == 1);
            const SlangNVVMAtomicOperationDesc& operation = gFakeNVVMBuilder.atomicOperations[0];
            SLANG_CHECK(operation.operation == SLANG_NVVM_ATOMIC_OP_ADD);
            SLANG_CHECK(NVVMSemantics::areSameType(operation.valueType, directCase.valueType));
            SLANG_CHECK(operation.addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
            SLANG_CHECK(operation.memoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerEqualUsesDirectPipeline, Equal)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerNotEqualUsesDirectPipeline, NotEqual)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerSignedGreaterThanUsesDirectPipeline, SignedGreaterThan)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerSignedLessEqualUsesDirectPipeline, SignedLessEqual)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerSignedGreaterEqualUsesDirectPipeline, SignedGreaterEqual)

#undef NVVM_SCALAR_DIRECT_TEST
SLANG_UNIT_TEST(nvvmSlangRejectsAdjacentStructuredBufferShapesBeforeProviderMutation)
{
    static const char* kUnsupportedSources[] = {
        kDirectNVVMIncompatibleStructuredBufferAggregateLayoutSource,
        kDirectNVVMUnsupportedStructuredMatrixWriteSource,
    };
    for (const char* source : kUnsupportedSources)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics)));
            SLANG_CHECK(code == nullptr);
            SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52017") >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.getStructTypeCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangRetainsOnlySelectedCUDAKernel)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMSelectedKernelSource,
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
        SLANG_CHECK(gFakeNVVMBuilder.functionName == "computeMain");
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangOrdinaryComputeAcceptsRawKernelParameters)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMConventionalParameterizedComputeSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangBuilderIdentityAffectsHashAndIsSessionCached)
{
    ComPtr<slang::IBlob> hashWithBuilder;
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::ISession> session;
        ComPtr<slang::IComponentType> program;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createDirectNVVMLinkedProgram(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            session,
            program,
            diagnostics)));
        program->getEntryPointHash(0, 0, hashWithBuilder.writeRef());
        SLANG_CHECK_ABORT(hashWithBuilder != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        const String expectedSearchPath = _getExpectedNVVMBuilderSearchPath();
        if (expectedSearchPath.getLength())
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.loadedPath.getUnownedSlice().indexOf(
                    expectedSearchPath.getUnownedSlice()) == 0);
        }

        ComPtr<slang::IBlob> code;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    ComPtr<slang::IBlob> hashWithoutBuilder;
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.libraryUnavailable = true;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::ISession> session;
        ComPtr<slang::IComponentType> program;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createDirectNVVMLinkedProgram(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            session,
            program,
            diagnostics)));
        program->getEntryPointHash(0, 0, hashWithoutBuilder.writeRef());
        SLANG_CHECK_ABORT(hashWithoutBuilder != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 0);

        ComPtr<slang::IBlob> code;
        SLANG_CHECK(SLANG_FAILED(
            program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
    }

    SLANG_CHECK_ABORT(hashWithBuilder->getBufferSize() == hashWithoutBuilder->getBufferSize());
    SLANG_CHECK(
        ::memcmp(
            hashWithBuilder->getBufferPointer(),
            hashWithoutBuilder->getBufferPointer(),
            hashWithBuilder->getBufferSize()) != 0);
}

SLANG_UNIT_TEST(nvvmSlangBuilderDiagnosticsStopBeforeLibNVVM)
{
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_INVALID;
    gFakeNVVMBuilder.verificationDiagnostic = "fake direct NVVM verifier failure";
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        const String diagnosticText = _getBlobText(diagnostics);
        SLANG_CHECK(diagnosticText.indexOf("E52018") >= 0);
        SLANG_CHECK(diagnosticText.indexOf(gFakeNVVMBuilder.verificationDiagnostic) >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.destroyModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangPreflightsExactValueOperationCapabilities)
{
    const SlangNVVMValueTypeDesc signedI8 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        8,
        1,
    };
    const SlangNVVMValueTypeDesc signedI32Operands[] = {
        NVVMSemantics::kSignedI32,
        NVVMSemantics::kSignedI32,
    };
    const SlangNVVMValueTypeDesc float32Operands[] = {
        NVVMSemantics::kFloat32,
        NVVMSemantics::kFloat32,
    };
    const SlangNVVMValueTypeDesc float32ToFloat16Operands[] = {NVVMSemantics::kFloat32};
    const SlangNVVMValueTypeDesc signedI8Operands[] = {signedI8, signedI8};

    struct CapabilityCase
    {
        const char* source;
        SlangNVVMValueOperationDesc rejectedOperation;
        const char* diagnosticName;
    };
    const CapabilityCase cases[] = {
        {
            kDirectNVVMIntegerMultiplySource,
            {
                SLANG_NVVM_VALUE_OP_MULTIPLY,
                NVVMSemantics::kSignedI32,
                signedI32Operands,
                2,
            },
            "signed i32 multiplication",
        },
        {
            kDirectNVVMFloat32AddSource,
            {
                SLANG_NVVM_VALUE_OP_ADD,
                NVVMSemantics::kFloat32,
                float32Operands,
                2,
            },
            "float32 addition",
        },
        {
            kDirectNVVMFloat16ValueSource,
            {
                SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
                NVVMSemantics::kFloat16,
                float32ToFloat16Operands,
                1,
            },
            "floating-point width conversion",
        },
        {
            "[CUDAKernel] void computeMain(uniform Ptr<uint, Access::ReadWrite, "
            "AddressSpace::Device> destination) { *destination = WaveGetConvergedMask(); }",
            {
                SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK,
                NVVMSemantics::kUnsignedI32,
                nullptr,
                0,
            },
            "hardware wave active mask",
        },
        {
            kDirectNVVMMixedNumericSource,
            {
                SLANG_NVVM_VALUE_OP_ADD,
                signedI8,
                signedI8Operands,
                2,
            },
            "parameterized integer binary operation",
        },
    };

    // Rejecting only one complete descriptor proves that validation preserved the exact overload,
    // rather than collapsing it to a broad feature or operation code. Each query happens before
    // module creation, so an unsupported overload cannot leave partial provider state behind.
    for (const auto& capability : cases)
    {
        _resetDirectNVVMFakes();
        _rejectFakeNVVMBuilderValueOperation(capability.rejectedOperation);
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(globalSession, capability.source, code, diagnostics)));
            SLANG_CHECK(code == nullptr);
            const String diagnosticText = _getBlobText(diagnostics);
            SLANG_CHECK(diagnosticText.indexOf("E52018") >= 0);
            SLANG_CHECK(diagnosticText.indexOf(capability.diagnosticName) >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.isOperationSupportedCallCount > 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangLibdeviceAndMinMaxOperationsRequestTypedOperations)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    static const uint8_t kLibdevice[] = {0x42, 0x43, 0xc0, 0xde, 0x7e, 0x12};
    TempDirectory toolkit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(toolkit)));
    String candidatePath;
    String libdevicePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createFakeNVVMToolkit(
        toolkit.path,
        kLibdevice,
        sizeof(kLibdevice),
        candidatePath,
        libdevicePath)));
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        globalSession->setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVVM, toolkit.path.getBuffer());

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMExactLibdeviceUnarySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        for (const char* name : {"__nv_sinf", "__nv_cosf", "__nv_sin", "__nv_cos", "__nv_truncf"})
            SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains(name));
        for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
            SLANG_CHECK(
                operation.key.operation != 40 && operation.key.operation != 41 &&
                operation.key.operation != 42);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 2);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds[0] == FakeModuleAddKind::Normal);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds[1] == FakeModuleAddKind::Lazy);
        SLANG_CHECK(gFakeNVVM.addedLibraryModuleName == "libdevice.10.bc");
        SLANG_CHECK(gFakeNVVM.addedLibraryModule.getLength() == sizeof(kLibdevice));
        SLANG_CHECK(
            ::memcmp(gFakeNVVM.addedLibraryModule.getBuffer(), kLibdevice, sizeof(kLibdevice)) ==
            0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        globalSession->setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVVM, toolkit.path.getBuffer());

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result =
            _compileSlangWithDirectNVVM(globalSession, kDirectNVVMMinMaxSource, code, diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        for (const char* name : {"__nv_fminf", "__nv_fmaxf", "__nv_fmin", "__nv_fmax"})
            SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains(name));
        bool sawIntegerCompare = false;
        for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
        {
            SLANG_CHECK(operation.key.operation != SlangNVVMValueOperation(43));
            SLANG_CHECK(operation.key.operation != SlangNVVMValueOperation(44));
            sawIntegerCompare |=
                operation.key.family == FakeNVVMBuilderScalarFamily::Compare &&
                NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kSignedI32);
        }
        SLANG_CHECK(sawIntegerCompare);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 2);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds[1] == FakeModuleAddKind::Lazy);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // The same selected toolkit must not be read for a module whose accepted semantic set does not
    // contain a device-library operation.
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        globalSession->setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVVM, toolkit.path.getBuffer());

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 1);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds[0] == FakeModuleAddKind::Normal);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangIntegerBitHelpersRequestTypedOperations)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMIntegerBitOperationsSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        // The public formulas use ordinary arithmetic around four explicitly named primitives.
        for (const char* name : {"llvm.ctpop", "llvm.bitreverse", "llvm.ctlz", "llvm.cttz"})
        {
            SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains(name));
        }
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 4);
        for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
        {
            SLANG_CHECK(operation.key.operation < 45 || operation.key.operation > 48);
        }
        Index scans = 0;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations[i] == UINT32_MAX);
            SLANG_CHECK(gFakeNVVMBuilder.intrinsicResultTypes[i].bitWidth == 32);
            SLANG_CHECK(gFakeNVVMBuilder.intrinsicResultTypes[i].laneCount == 1);
            const Index count = gFakeNVVMBuilder.intrinsicArgumentCounts[i];
            SLANG_CHECK(count == 1 || count == 2);
            if (count == 2)
            {
                ++scans;
                const Index offset = gFakeNVVMBuilder.intrinsicArgumentOffsets[i];
                const auto flag = gFakeNVVMBuilder.intrinsicArgumentValueRefs[offset + 1];
                SLANG_CHECK_ABORT(flag.kind == FakeNVVMBuilderValueKind::IntegerConstant);
                SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[flag.index] == 0);
            }
        }
        SLANG_CHECK(scans == 2);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangIntegerTruthinessAndBitfieldsUseTypedRecipes)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMIntegerTruthinessBitfieldSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        bool sawIntegerTruthiness = false;
        bool sawVectorBitNot = false;
        bool sawSignedReinterpretation = false;
        bool sawUnsignedReinterpretation = false;
        bool sawVectorShift = false;
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            sawIntegerTruthiness |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_NOT_EQUAL &&
                operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_BOOL &&
                operation.operandCount == 2 &&
                operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER;
            sawVectorBitNot |= operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_NOT &&
                               operation.resultType.laneCount == 2;
            sawSignedReinterpretation |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET &&
                operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER;
            sawUnsignedReinterpretation |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET &&
                operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER &&
                operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER;
            sawVectorShift |= (operation.key.operation == SLANG_NVVM_VALUE_OP_SHIFT_LEFT ||
                               operation.key.operation == SLANG_NVVM_VALUE_OP_SHIFT_RIGHT) &&
                              operation.resultType.laneCount == 2;
        }
        SLANG_CHECK(sawIntegerTruthiness);
        SLANG_CHECK(sawVectorBitNot);
        SLANG_CHECK(sawSignedReinterpretation);
        SLANG_CHECK(sawUnsignedReinterpretation);
        SLANG_CHECK(sawVectorShift);
        SLANG_CHECK(gFakeNVVMBuilder.emitVectorConstructCallCount >= 6);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangFloatingTruthinessUsesTypedComparisons)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloatingTruthinessSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        bool sawComparison[3] = {};
        for (const FakeNVVMBuilderScalarOperation& operation : gFakeNVVMBuilder.scalarOperations)
        {
            if (operation.key.operation != SLANG_NVVM_VALUE_OP_NOT_EQUAL ||
                operation.resultType.kind != SLANG_NVVM_VALUE_TYPE_BOOL ||
                operation.operandCount != 2 ||
                operation.operandTypes[0].kind != SLANG_NVVM_VALUE_TYPE_FLOATING_POINT)
            {
                continue;
            }
            switch (operation.operandTypes[0].bitWidth)
            {
            case 16:
                sawComparison[0] = true;
                break;
            case 32:
                sawComparison[1] = true;
                break;
            case 64:
                sawComparison[2] = true;
                break;
            }
        }
        SLANG_CHECK(sawComparison[0]);
        SLANG_CHECK(sawComparison[1]);
        SLANG_CHECK(sawComparison[2]);

        bool sawZero[3] = {};
        for (Index i = 0; i < gFakeNVVMBuilder.floatingPointConstantBitPatterns.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.floatingPointConstantBitPatterns[i] != 0)
                continue;
            switch (gFakeNVVMBuilder.floatingPointConstantBitWidths[i])
            {
            case 16:
                sawZero[0] = true;
                break;
            case 32:
                sawZero[1] = true;
                break;
            case 64:
                sawZero[2] = true;
                break;
            }
        }
        SLANG_CHECK(sawZero[0]);
        SLANG_CHECK(sawZero[1]);
        SLANG_CHECK(sawZero[2]);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangScalarMathHelpersRequestTypedOperations)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    static const uint8_t kLibdevice[] = {0x42, 0x43, 0xc0, 0xde, 0x7e, 0x12};
    TempDirectory toolkit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(toolkit)));
    String candidatePath;
    String libdevicePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createFakeNVVMToolkit(
        toolkit.path,
        kLibdevice,
        sizeof(kLibdevice),
        candidatePath,
        libdevicePath)));

    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        globalSession->setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVVM, toolkit.path.getBuffer());

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMScalarMathOperationsSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        uint32_t operationCounts[SLANG_NVVM_VALUE_OPERATION_COUNT] = {};
        for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
        {
            if (operation.key.operation < SLANG_NVVM_VALUE_OPERATION_COUNT)
                ++operationCounts[operation.key.operation];
        }
        SLANG_CHECK(operationCounts[49] == 0);
        SLANG_CHECK(!gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_fabsf"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_tan"));
        SLANG_CHECK(operationCounts[66] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_pow"));
        SLANG_CHECK(operationCounts[63] == 0);
        SLANG_CHECK(operationCounts[67] == 0);
        SLANG_CHECK(operationCounts[68] == 0);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 2);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds[1] == FakeModuleAddKind::Lazy);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangScalarIntrinsicHelpersUseTypedRecipes)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    static const uint8_t kLibdevice[] = {0x42, 0x43, 0xc0, 0xde, 0x7e, 0x12};
    TempDirectory toolkit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(toolkit)));
    String candidatePath;
    String libdevicePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createFakeNVVMToolkit(
        toolkit.path,
        kLibdevice,
        sizeof(kLibdevice),
        candidatePath,
        libdevicePath)));

    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        globalSession->setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVVM, toolkit.path.getBuffer());

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMScalarIntrinsicRecipeSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        uint32_t operationCounts[SLANG_NVVM_VALUE_OPERATION_COUNT] = {};
        bool hasSignedI16ToHalf = false;
        bool hasHalfToUnsignedI16 = false;
        bool hasUnsignedI64ToDouble = false;
        bool hasDoubleToUnsignedI64 = false;
        for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
        {
            if (operation.key.operation < SLANG_NVVM_VALUE_OPERATION_COUNT)
                ++operationCounts[operation.key.operation];
            if (operation.key.operation != SLANG_NVVM_VALUE_OP_BIT_REINTERPRET ||
                operation.operandCount != 1)
            {
                continue;
            }
            hasSignedI16ToHalf |=
                NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat16) &&
                NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kSignedI16);
            hasHalfToUnsignedI16 |=
                NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kUnsignedI16) &&
                NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat16);
            hasUnsignedI64ToDouble |=
                NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat64) &&
                NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kUnsignedI64);
            hasDoubleToUnsignedI64 |=
                NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kUnsignedI64) &&
                NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat64);
        }
        SLANG_CHECK(hasSignedI16ToHalf);
        SLANG_CHECK(hasHalfToUnsignedI16);
        SLANG_CHECK(hasUnsignedI64ToDouble);
        SLANG_CHECK(hasDoubleToUnsignedI64);
        SLANG_CHECK(operationCounts[SLANG_NVVM_VALUE_OP_SHIFT_LEFT] >= 1);
        SLANG_CHECK(operationCounts[SLANG_NVVM_VALUE_OP_SHIFT_RIGHT] >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_sinf"));
        SLANG_CHECK(operationCounts[40] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_cosf"));
        SLANG_CHECK(operationCounts[41] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_frexpf"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_frexp"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_modff"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_modf"));
        for (uint32_t retired : {69u, 70u, 77u, 78u})
            SLANG_CHECK(operationCounts[retired] == 0);
        SLANG_CHECK(operationCounts[SlangNVVMValueOperation(43)] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_fminf"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_fmaxf"));
        SLANG_CHECK(operationCounts[SlangNVVMValueOperation(44)] == 0);
        SLANG_CHECK(operationCounts[68] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_sinhf"));
        SLANG_CHECK(operationCounts[73] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_coshf"));
        SLANG_CHECK(operationCounts[74] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_tanhf"));
        SLANG_CHECK(operationCounts[75] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_fmaf"));
        SLANG_CHECK(operationCounts[76] == 0);
        SLANG_CHECK(operationCounts[SLANG_NVVM_VALUE_OP_FLOAT_CONVERT] >= 20);
        // The selected libdevice call writes its output pointer internally, without a
        // separate provider store. Check that each named path carries actual storage.
        for (const char* name : {"__nv_frexpf", "__nv_frexp", "__nv_modff", "__nv_modf"})
        {
            bool sawOutputPointer = false;
            for (Index i = 0; i < gFakeNVVMBuilder.intrinsicNames.getCount(); ++i)
            {
                if (gFakeNVVMBuilder.intrinsicNames[i] != name)
                    continue;
                SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[i] == 2);
                const Index offset = gFakeNVVMBuilder.intrinsicArgumentOffsets[i];
                FakeNVVMBuilderScalarTypeKind pointee;
                sawOutputPointer |= _getFakeNVVMBuilderPointerScalarTypeKind(
                    gFakeNVVMBuilder.intrinsicArgumentValueRefs[offset + 1],
                    pointee);
            }
            SLANG_CHECK(sawOutputPointer);
        }
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 2);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds[1] == FakeModuleAddKind::Lazy);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangCompoundWaveHelpersUseScalarRecipes)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCompoundWaveOperationsSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.shfl.sync.idx.f32"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.match.any.sync.i32"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.vote.ballot.sync"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.ctpop"));
        bool intersectsComponentMasks = false;
        for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
            intersectsComponentMasks |=
                operation.key.operation == SLANG_NVVM_VALUE_OP_BIT_AND &&
                NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kUnsignedI32);
        SLANG_CHECK(intersectsComponentMasks);
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount >= 4);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangMaskedWaveScalarHelpersUseGenericLoops)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMMaskedWaveScalarOperationsSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.shfl.sync.idx.i32"));
        SLANG_CHECK(
            gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.read.ptx.sreg.laneid"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.cttz"));
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTargetBlockIndices.getCount() >= 4);
        SLANG_CHECK(gFakeNVVMBuilder.addPhiIncomingCallCount >= 8);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount >= 4);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangSingletonReductionsPreserveTypedOperands)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    _resetDirectNVVMFakes();
    TempDirectory toolkit;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(_configureFakeDirectNVVMLibdevice(globalSession, toolkit)));

        const char source[] = R"(
            [CUDAKernel]
            void computeMain(
                uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
                uniform uint mask)
            {
                uint lane = WaveGetLaneIndex();
                float f = float(lane);
                double d = double(lane);
                uint4 members = uint4(mask, 0, 0, 0);
                destination[0] = int(WaveMultiMin(f, members));
                destination[1] = int(WaveMultiMax(f, members));
                destination[2] = int(WaveMultiSum(d, members));
                destination[3] = int(WaveMultiProduct(d, members));
                destination[4] = int(WaveMultiMin(d, members));
                destination[5] = int(WaveMultiMax(d, members));
            }
        )";
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result =
            _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        uint32_t orderedFloat32 = 0, orderedFloat64 = 0;
        for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
        {
            if ((operation.key.operation == SLANG_NVVM_VALUE_OP_LESS_THAN ||
                 operation.key.operation == SLANG_NVVM_VALUE_OP_GREATER_THAN) &&
                operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT)
            {
                orderedFloat32 += operation.operandTypes[0].bitWidth == 32;
                orderedFloat64 += operation.operandTypes[0].bitWidth == 64;
            }
        }
        SLANG_CHECK(orderedFloat32 >= 2);
        SLANG_CHECK(orderedFloat64 >= 2);
        SLANG_CHECK(!gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_fminf"));
        SLANG_CHECK(!gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_fmax"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.shfl.sync.idx.i32"));
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTargetBlockIndices.getCount() >= 4);
        // The core builds the Double seed through asdouble's integer-word bitcast. The exact
        // fp64 wave runtime fixtures own its bits and singleton preservation independently of
        // whether constant folding creates a floating constant or retains that construction.
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangAggregateWaveHelpersUseRecursiveScalarRecipes)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    static const uint8_t kLibdevice[] = {0x42, 0x43, 0xc0, 0xde, 0x7e, 0x12};
    TempDirectory toolkit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(toolkit)));
    String candidatePath;
    String libdevicePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createFakeNVVMToolkit(
        toolkit.path,
        kLibdevice,
        sizeof(kLibdevice),
        candidatePath,
        libdevicePath)));

    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        globalSession->setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVVM, toolkit.path.getBuffer());

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMAggregateWaveOperationsSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.shfl.sync.idx.i32"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.vote.ballot.sync"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.cttz"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("__nv_fminf"));
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount >= 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitVectorConstructCallCount >= 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount >= 3);
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTargetBlockIndices.getCount() >= 4);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 2);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds[1] == FakeModuleAddKind::Lazy);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangFloat64ImplicitAggregateShuffleUsesTypedMaskChain)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    static const uint8_t kLibdevice[] = {0x42, 0x43, 0xc0, 0xde, 0x7e, 0x12};
    TempDirectory toolkit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(toolkit)));
    String candidatePath;
    String libdevicePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createFakeNVVMToolkit(
        toolkit.path,
        kLibdevice,
        sizeof(kLibdevice),
        candidatePath,
        libdevicePath)));

    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        globalSession->setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVVM, toolkit.path.getBuffer());

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFloat64ImplicitAggregateShuffleSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        uint32_t activeMaskCount = 0;
        for (auto operation : gFakeNVVMBuilder.intrinsicOperations)
            activeMaskCount += operation == SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK;
        SLANG_CHECK(activeMaskCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.shfl.sync.idx.i32"));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.vote.ballot.sync"));
        Index ballotFunction = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.functionNames.getCount(); ++i)
            if (gFakeNVVMBuilder.namedIntrinsicFunctionNames[i] == "llvm.nvvm.vote.ballot.sync")
                ballotFunction = i;
        SLANG_CHECK_ABORT(ballotFunction >= 0);
        bool capturedMaskFeedsBallot = false;
        for (Index i = 0; i < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.callCalleeFunctionIndices[i] != ballotFunction)
                continue;
            const auto mask =
                gFakeNVVMBuilder.callArgumentValueRefs[gFakeNVVMBuilder.callArgumentOffsets[i]];
            capturedMaskFeedsBallot |= mask.kind == FakeNVVMBuilderValueKind::Intrinsic &&
                                       gFakeNVVMBuilder.intrinsicOperations[mask.index] ==
                                           SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK;
        }
        SLANG_CHECK(capturedMaskFeedsBallot);
        bool ballotFeedsTransport = false;
        for (Index i = 0; i < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount(); ++i)
        {
            if (!gFakeNVVMBuilder.callArgumentCounts[i])
                continue;
            const auto mask =
                gFakeNVVMBuilder.callArgumentValueRefs[gFakeNVVMBuilder.callArgumentOffsets[i]];
            ballotFeedsTransport |=
                mask.kind == FakeNVVMBuilderValueKind::Call &&
                gFakeNVVMBuilder.callCalleeFunctionIndices[mask.index] == ballotFunction;
        }
        SLANG_CHECK(ballotFeedsTransport);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangCanonicalEphemeralValuesUseDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMChosenUndefinedAndDebugMarkerSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool foundChosenFloatZero = false;
        for (Index index = 0; index < gFakeNVVMBuilder.floatingPointConstantBitPatterns.getCount();
             ++index)
        {
            foundChosenFloatZero |= gFakeNVVMBuilder.floatingPointConstantBitWidths[index] == 32 &&
                                    gFakeNVVMBuilder.floatingPointConstantBitPatterns[index] == 0;
        }
        SLANG_CHECK(foundChosenFloatZero);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMStableStringHashSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        bool foundStableStringHash = false;
        for (Index index = 0; index < gFakeNVVMBuilder.integerConstantValues.getCount(); ++index)
        {
            foundStableStringHash |= gFakeNVVMBuilder.integerConstantBitWidths[index] == 32 &&
                                     gFakeNVVMBuilder.integerConstantValues[index] == 1840786589;
        }
        SLANG_CHECK(foundStableStringHash);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// The same canonical vector must retain distinct local-memory and by-value helper roles.
SLANG_UNIT_TEST(nvvmSlangBFloat16LocalVectorsUseQualifiedStorage)
{
    for (uint32_t width = 2; width <= 4; ++width)
    {
        _resetDirectNVVMFakes();
        {
            StringBuilder source;
            source << "typealias V = vector<BFloat16," << width << ">;" << R"SLANG(
                [noinline] V replace(inout V x, V y) { let old = x; x = y; return old; }
                [noinline] void initialize(out V x, V y) { x = y; }
                RWStructuredBuffer<uint> outputBuffer;
                [numthreads(32, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
                {
                    let v = V(BFloat16(1.0f));
                    V local = v;
                    let old = replace(local, v);
                    V copied;
                    initialize(copied, old);
                    outputBuffer[tid.x] = uint(bit_cast<uint16_t>(local.x)) +
                                          uint(bit_cast<uint16_t>(copied.x));
                }
            )SLANG";
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult result =
                _compileSlangWithDirectNVVM(globalSession, source.getBuffer(), code, diagnostics);
            if (SLANG_FAILED(result))
                getTestReporter()->message(
                    TestMessageType::Info,
                    _getBlobText(diagnostics).getBuffer());
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
            SLANG_CHECK(gFakeNVVMBuilder.localStorageValueTypes.getCount() == 2);
            for (Index i = 0; i < gFakeNVVMBuilder.localStorageValueTypes.getCount(); ++i)
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder.localStorageValueTypes[i] ==
                    (width == 2 ? _getFakeNVVMBuilderVectorType(2)
                                : _getFakeNVVMBuilderArrayType()));
                SLANG_CHECK(gFakeNVVMBuilder.localStorageAlignments[i] == (width == 2 ? 4u : 2u));
            }
            bool sawStorageLoad = false;
            for (Index i = 0; i < gFakeNVVMBuilder.loadResultTypeKinds.getCount(); ++i)
            {
                const auto expectedKind = width == 2 ? FakeNVVMBuilderScalarTypeKind::UInt2
                                                     : FakeNVVMBuilderScalarTypeKind::NumericArray;
                if (gFakeNVVMBuilder.loadResultTypeKinds[i] != expectedKind)
                    continue;
                sawStorageLoad = true;
                SLANG_CHECK(gFakeNVVMBuilder.loadAlignments[i] == (width == 2 ? 4u : 2u));
                SLANG_CHECK(gFakeNVVMBuilder.loadFlags[i] == SLANG_NVVM_LOAD_FLAG_NONE);
            }
            SLANG_CHECK(sawStorageLoad);
            SLANG_CHECK(_countFakeNVVMNoInlineHelperCalls("initialize", 2) == 1);
            SLANG_CHECK(_countFakeNVVMNoInlineHelperCalls("replace", 2) == 1);
            if (width > 2)
            {
                SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == width);
                SLANG_CHECK(gFakeNVVMBuilder.emitAggregateConstructCallCount >= 2);
                SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount >= width);
                SLANG_CHECK(gFakeNVVMBuilder.emitVectorConstructCallCount >= 2);
            }
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

// A local BF16 record keeps its qualified field storage without becoming a by-value record.
SLANG_UNIT_TEST(nvvmSlangBFloat16LocalRecordsUseQualifiedFields)
{
    for (uint32_t width = 2; width <= 4; ++width)
    {
        _resetDirectNVVMFakes();
        {
            StringBuilder source;
            source << "typealias V = vector<BFloat16," << width << ">;" << R"SLANG(
                struct Record { uint16_t prefix; V value; uint16_t suffix; }
                [noinline] void initialize(out Record x, V y)
                { x.prefix = uint16_t(17); x.value = y; x.suffix = uint16_t(19); }
                [noinline] V replace(inout Record x, V y)
                { let old = x.value; x.value = y; return old; }
                RWStructuredBuffer<uint> outputBuffer;
                [numthreads(32, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
                {
                    let v = V(BFloat16(1.0f));
                    Record local;
                    initialize(local, v);
                    let old = replace(local, v);
                    outputBuffer[tid.x] = uint(bit_cast<uint16_t>(old[0])) +
                        uint(bit_cast<uint16_t>(local.value.x)) +
                        uint(local.prefix) + uint(local.suffix);
                }
            )SLANG";
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult result =
                _compileSlangWithDirectNVVM(globalSession, source.getBuffer(), code, diagnostics);
            if (SLANG_FAILED(result))
                getTestReporter()->message(
                    TestMessageType::Info,
                    _getBlobText(diagnostics).getBuffer());
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.localStorageValueTypes.getCount() == 1);
            SLANG_CHECK(
                gFakeNVVMBuilder.localStorageValueTypes[0] ==
                _getFakeNVVMBuilderScalarStructType());
            SLANG_CHECK(gFakeNVVMBuilder.localStorageAlignments[0] == (width == 2 ? 4u : 2u));
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.scalarStructFieldTypes.getCount() == 3);
            SLANG_CHECK(
                gFakeNVVMBuilder.scalarStructFieldTypes[0] == _getFakeNVVMBuilderIntegerType());
            SLANG_CHECK(
                gFakeNVVMBuilder.scalarStructFieldTypes[1] ==
                (width == 2 ? _getFakeNVVMBuilderVectorType(2) : _getFakeNVVMBuilderArrayType()));
            SLANG_CHECK(
                gFakeNVVMBuilder.scalarStructFieldTypes[2] == _getFakeNVVMBuilderIntegerType());
            SLANG_CHECK(_countFakeNVVMNoInlineHelperCalls("initialize", 2) == 1);
            SLANG_CHECK(_countFakeNVVMNoInlineHelperCalls("replace", 2) == 1);
            SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount >= 7);
            if (width > 2)
            {
                SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == width);
                SLANG_CHECK(gFakeNVVMBuilder.emitAggregateConstructCallCount >= 2);
                SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount >= width);
                SLANG_CHECK(gFakeNVVMBuilder.emitVectorConstructCallCount >= 2);
            }
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangSubstandardRecordsUseInternalValues)
{
    const char* types[] = {"FloatE4M3", "FloatE5M2", "BFloat16"};
    for (const char* type : types)
    {
        _resetDirectNVVMFakes();
        StringBuilder source;
        source << "typealias F = " << type
               << "; typealias Bits = " << (String(type) == "BFloat16" ? "uint16_t" : "uint8_t")
               << ";" << R"SLANG(
            struct Payload { F value; }
            [noinline] Payload copy(Payload x) { return x; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p = {F(1.25f)};
                outputBuffer[0] = uint(bit_cast<Bits>(copy(p).value));
            }
        )SLANG";
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        auto result =
            _compileSlangWithDirectNVVM(globalSession, source.getBuffer(), code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount >= 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount >= 1);
    }
}

// Uses LLVM's real aggregate types while retaining the deterministic fake libNVVM compiler.
// Consider `struct Payload { FloatE4M3 value; }; struct Outer { Payload inner; };`.
// The scalar fake provider has only one struct handle and one field list, so it cannot represent
// distinct parent and child types. Forward
// only the existing builder-library request to the platform loader instead of inventing another
// recursive type model in the fixture. The real provider verifies and serializes the module;
// separate GPU tests qualify libNVVM optimization and execution.
class RealBuilderFakeNVVMLoader : public FakeDirectNVVMLoader
{
public:
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        if (path && UnownedStringSlice(path).indexOf(toSlice("slang-llvm-nvvm")) >= 0)
            return DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(path, outLibrary);
        return FakeDirectNVVMLoader::loadSharedLibrary(path, outLibrary);
    }
};

// Nested field/index selection preserves native storage and read permission independently.
SLANG_UNIT_TEST(nvvmSlangNestedBorrowedVectorAddressesPreserveStorageRoles)
{
    NVVMIRBuilder realBuilder;
    _requireRealNVVMBuilder(unitTestContext, realBuilder);
    _resetDirectNVVMFakes();
    {
        const char* source = R"SLANG(
            struct Inner { float3 value; float sentinel; }
            struct Payload { Inner inner; uint guard; }
            [noinline] float read(__constref Payload p, uint lane)
            {
                return p.inner.value[lane];
            }
            [noinline] void replace(inout Payload p, uint lane, float value)
            {
                p.inner.value[lane] = value;
            }
            RWStructuredBuffer<float> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p;
                p.inner.value = float3(1.0f, 2.0f, 3.0f);
                p.inner.sentinel = 9.0f;
                p.guard = 7;
                let before = read(p, tid.x % 3);
                replace(p, tid.x % 3, 5.0f);
                let after = read(p, tid.x % 3);
                outputBuffer[0] = before + after + p.inner.sentinel + float(p.guard);
            }
        )SLANG";
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new RealBuilderFakeNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(session, source, code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);

        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        const String& assembly = gFakeNVVM.addedModule;
        SLANG_CHECK(assembly.indexOf("alloca { { <3 x float>, float }, i32 }, align 16") >= 0);

        // These are the only scalar-returning and void helpers in this fixture. Scope checks
        // to their bodies because the kernel's resource-descriptor loads may be invariant.
        const char* helperPrefixes[] = {"define internal float ", "define internal void "};
        for (Index helper = 0; helper < 2; ++helper)
        {
            const Index start = assembly.indexOf(helperPrefixes[helper]);
            SLANG_CHECK_ABORT(start >= 0);
            const Index end = assembly.indexOf("\n}", start);
            SLANG_CHECK_ABORT(end > start);
            SLANG_CHECK(assembly.indexOf(helperPrefixes[helper], end) < 0);
            const String body = assembly.subString(start, end - start);
            SLANG_CHECK(
                body.indexOf("getelementptr inbounds { { <3 x float>, float }, i32 }") >= 0);
            SLANG_CHECK(body.indexOf("getelementptr inbounds { <3 x float>, float }") >= 0);
            SLANG_CHECK(body.indexOf("[3 x float]") < 0);

            const Index laneAddress = body.indexOf("getelementptr <3 x float>");
            SLANG_CHECK_ABORT(laneAddress >= 0);
            const Index laneAddressEnd = body.indexOf('\n', laneAddress);
            SLANG_CHECK_ABORT(laneAddressEnd > laneAddress);
            const String laneAddressLine =
                body.subString(laneAddress, laneAddressEnd - laneAddress);
            // The provider assigns stable names to parameters for LLVM 7/14 textual compatibility.
            // Both helpers receive the runtime lane as their second parameter.
            SLANG_CHECK(laneAddressLine.indexOf("i32 0, i32 %slangParameter1") >= 0);

            const Index memoryOperation =
                body.indexOf(helper == 0 ? "load float," : "store float ");
            SLANG_CHECK_ABORT(memoryOperation >= 0);
            const Index memoryOperationEnd = body.indexOf('\n', memoryOperation);
            SLANG_CHECK_ABORT(memoryOperationEnd > memoryOperation);
            const String memoryLine =
                body.subString(memoryOperation, memoryOperationEnd - memoryOperation);
            SLANG_CHECK(memoryLine.indexOf("align 4") >= 0);
            SLANG_CHECK(body.indexOf("!invariant.load") < 0);
        }
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// This is the former nested FP8 negative, extended to scalar BF16. The canonical inner
// record remains observable across a noinline outer value parameter and result.
SLANG_UNIT_TEST(nvvmSlangNestedSubstandardRecordsUseInternalValues)
{
    NVVMIRBuilder realBuilder;
    _requireRealNVVMBuilder(unitTestContext, realBuilder);
    for (const char* format : {"FloatE4M3", "FloatE5M2", "BFloat16"})
    {
        _resetDirectNVVMFakes();
        {
            StringBuilder source;
            source << "typealias F = " << format << "; typealias Bits = "
                   << (String(format) == "BFloat16" ? "uint16_t" : "uint8_t") << ";" << R"SLANG(
                struct Payload { F value; }
                struct Outer { Payload value; }
                [noinline] Outer copy(Outer x) { return x; }
                RWStructuredBuffer<uint> outputBuffer;
                [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
                {
                    Payload p = {bit_cast<F>(Bits(tid.x))};
                    Outer o = {p};
                    outputBuffer[0] = uint(bit_cast<Bits>(copy(o).value.value));
                }
            )SLANG";
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new RealBuilderFakeNVVMLoader);
            session->setSharedLibraryLoader(loader);
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const auto result =
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
            if (SLANG_FAILED(result))
                getTestReporter()->message(
                    TestMessageType::Info,
                    _getBlobText(diagnostics).getBuffer());
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
            const char* recordType = String(format) == "BFloat16" ? "{ { i16 } }" : "{ { i8 } }";
            StringBuilder signature;
            signature << "define internal " << recordType;
            SLANG_CHECK(gFakeNVVM.addedModule.indexOf(signature.getUnownedSlice()) >= 0);
            StringBuilder extraction;
            extraction << "extractvalue " << recordType;
            SLANG_CHECK(gFakeNVVM.addedModule.indexOf(extraction.getUnownedSlice()) >= 0);
            StringBuilder returnedValue;
            returnedValue << "ret " << recordType;
            SLANG_CHECK(gFakeNVVM.addedModule.indexOf(returnedValue.getUnownedSlice()) >= 0);
            SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
            SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 1);
            SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

// Three struct levels retain a local root through whole-inner assignment and BF2 lane addressing.
// The integer-only child checks that substandard admission is a property of the complete record.
SLANG_UNIT_TEST(nvvmSlangNestedSubstandardRecordsUseLocalFields)
{
    NVVMIRBuilder realBuilder;
    _requireRealNVVMBuilder(unitTestContext, realBuilder);
    const char* source = R"SLANG(
        struct Inner { uint16_t prefix; vector<BFloat16, 2> pair; uint16_t suffix; }
        struct Guards { uint16_t first; uint last; }
        struct Middle { Inner payload; Guards guards; }
        struct Outer { uint8_t tag; Middle middle; uint64_t tail; }
        [noinline] void initialize(out Outer p, uint bits)
        {
            p.tag = uint8_t(17);
            p.middle.payload.prefix = uint16_t(19);
            p.middle.payload.pair = vector<BFloat16, 2>(
                bit_cast<BFloat16>(uint16_t(bits)), bit_cast<BFloat16>(uint16_t(bits ^ 65535)));
            p.middle.payload.suffix = uint16_t(23);
            p.middle.guards.first = uint16_t(29);
            p.middle.guards.last = 31;
            p.tail = uint64_t(37);
        }
        [noinline] Inner replace(inout Outer p, Inner q, uint lane, uint bits)
        {
            let old = p.middle.payload;
            p.middle.payload = q;
            p.middle.payload.pair[lane] = bit_cast<BFloat16>(uint16_t(bits));
            p.middle.guards.last += 1;
            return old;
        }
        RWStructuredBuffer<uint> outputBuffer;
        [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            Outer a;
            Outer b;
            initialize(a, tid.x);
            initialize(b, tid.x ^ 65535);
            let old = replace(a, b.middle.payload, tid.x & 1, tid.x ^ 0x5555);
            outputBuffer[0] = uint(bit_cast<uint16_t>(old.pair.x));
            outputBuffer[1] = uint(bit_cast<uint16_t>(a.middle.payload.pair.y));
            outputBuffer[2] = a.middle.guards.last;
            outputBuffer[3] = uint(a.tail) + uint(a.tag);
        }
    )SLANG";
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new RealBuilderFakeNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const auto result = _compileSlangWithDirectNVVM(session, source, code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        const String& assembly = gFakeNVVM.addedModule;
        SLANG_CHECK(
            assembly.indexOf(
                "alloca { i8, { { i16, <2 x i16>, i16 }, { i16, i32 } }, i64 }, align 8") >= 0);
        SLANG_CHECK(
            assembly.indexOf(
                "getelementptr inbounds { i8, { { i16, <2 x i16>, i16 }, { i16, i32 } }, i64 }") >=
            0);
        SLANG_CHECK(assembly.indexOf("getelementptr <2 x i16>") >= 0);
        SLANG_CHECK(assembly.indexOf("load { i16, <2 x i16>, i16 }") >= 0);
        SLANG_CHECK(assembly.indexOf("store { i16, <2 x i16>, i16 }") >= 0);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// CUDA padding is checked at each nested boundary, including the integer-only child. BF2
// aligns to four bytes, so the inner payload is sixteen bytes rather than its Natural size twelve.
SLANG_UNIT_TEST(nvvmSlangNestedSubstandardRecordLayoutQueries)
{
    const char* source = R"SLANG(
        struct Inner
        {
            uint8_t tag; FloatE4M3 a; FloatE5M2 b; BFloat16 scalar;
            vector<BFloat16, 2> pair; uint16_t suffix;
        }
        struct Guards { uint16_t first; uint last; }
        struct Middle { uint8_t tag; Inner payload; Guards guards; }
        struct Outer { uint16_t prefix; Middle middle; uint64_t tail; }
        [CUDAKernel] void computeMain(
            uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
        {
            Inner inner; Guards guards; Middle middle; Outer outer;
            destination[0] = __sizeOf<Inner>();
            destination[1] = __alignOf<Inner>();
            destination[2] = __offsetOf(inner, inner.scalar);
            destination[3] = __offsetOf(inner, inner.pair);
            destination[4] = __offsetOf(inner, inner.suffix);
            destination[5] = __sizeOf<Guards>();
            destination[6] = __alignOf<Guards>();
            destination[7] = __offsetOf(guards, guards.last);
            destination[8] = __sizeOf<Middle>();
            destination[9] = __alignOf<Middle>();
            destination[10] = __offsetOf(middle, middle.payload);
            destination[11] = __offsetOf(middle, middle.guards);
            destination[12] = __sizeOf<Outer>();
            destination[13] = __alignOf<Outer>();
            destination[14] = __offsetOf(outer, outer.middle);
            destination[15] = __offsetOf(outer, outer.tail);
        }
    )SLANG";
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const auto result = _compileSlangWithDirectNVVM(session, source, code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        const int64_t expected[] = {16, 4, 4, 8, 12, 8, 4, 4, 28, 4, 4, 20, 40, 8, 4, 32};
        SLANG_CHECK_ABORT(gFakeNVVMBuilder.storeValueRefs.getCount() == SLANG_COUNT_OF(expected));
        for (Index i = 0; i < SLANG_COUNT_OF(expected); ++i)
        {
            const auto value = gFakeNVVMBuilder.storeValueRefs[i];
            SLANG_CHECK_ABORT(value.kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK_ABORT(value.index >= 0);
            SLANG_CHECK_ABORT(value.index < gFakeNVVMBuilder.integerConstantValues.getCount());
            SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[value.index] == expected[i]);
        }
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// This exact source previously guarded the array-parameter exclusion. It now crosses an internal
// value boundary and returns an already-qualified nested record, so use the real aggregate builder.
SLANG_UNIT_TEST(nvvmSlangNestedRecordArrayParameterReturnsRecord)
{
    NVVMIRBuilder realBuilder;
    _requireRealNVVMBuilder(unitTestContext, realBuilder);
    _resetDirectNVVMFakes();
    {
        const char* source = R"SLANG(
            struct Payload { FloatE4M3 value; };
            struct Outer { Payload inner; };

            [noinline] Outer copy(Outer x[2]) { return x[1]; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Outer p = {{bit_cast<FloatE4M3>(uint8_t(tid.x))}};
                Outer values[2] = {p, p};
                outputBuffer[0] = uint(bit_cast<uint8_t>(copy(values).inner.value));
            }

        )SLANG";
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new RealBuilderFakeNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const auto result = _compileSlangWithDirectNVVM(session, source, code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        const String& assembly = gFakeNVVM.addedModule;
        SLANG_CHECK(assembly.indexOf("define internal { { i8 } }") >= 0);
        SLANG_CHECK(assembly.indexOf("([2 x { { i8 } }] %slangParameter0)") >= 0);
        SLANG_CHECK(assembly.indexOf("call { { i8 } }") >= 0);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// Nesting preserves the existing local/internal domain; it cannot grant an external memory
// role or a pointer-return ABI. Each rejected source must stop before loading either provider.
SLANG_UNIT_TEST(nvvmSlangNestedSubstandardRecordsRejectOtherRoles)
{
    struct Case
    {
        const char* source;
        const char* construct;
    };
    const Case cases[] = {
        {R"SLANG(
            [noinline] uint read(__constref Outer x)
            { return uint(bit_cast<uint8_t>(x.inner.value)); }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Outer p = {{bit_cast<FloatE4M3>(uint8_t(tid.x))}};
                outputBuffer[0] = read(p);
            }
        )SLANG",
         "helper function parameter"},
        // Shared storage never acquires the local record role. Module preflight rejects this
        // unsupported global declaration before an individual field access needs diagnosis.
        {R"SLANG(
            groupshared Outer sharedValue;
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                sharedValue.inner.value = bit_cast<FloatE4M3>(uint8_t(tid.x));
                GroupMemoryBarrierWithGroupSync();
                outputBuffer[0] = uint(bit_cast<uint8_t>(sharedValue.inner.value));
            }
        )SLANG",
         "'global_var'"},
        {R"SLANG(
            RWStructuredBuffer<Outer> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            { outputBuffer[0].inner.value = bit_cast<FloatE4M3>(uint8_t(tid.x)); }
        )SLANG",
         "struct field address result"},
        {R"SLANG(
            [CudaDeviceExport] [noinline] Outer copy(Outer x) { return x; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Outer p = {{bit_cast<FloatE4M3>(uint8_t(tid.x))}};
                outputBuffer[0] = uint(bit_cast<uint8_t>(copy(p).inner.value));
            }
        )SLANG",
         "exported substandard record helper result"},
        {R"SLANG(
            [CudaDeviceExport] [noinline] uint read(Outer x)
            { return uint(bit_cast<uint8_t>(x.inner.value)); }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Outer p = {{bit_cast<FloatE4M3>(uint8_t(tid.x))}};
                outputBuffer[0] = read(p);
            }
        )SLANG",
         "exported substandard record helper parameter"},
        {R"SLANG(
            [CudaDeviceExport] [noinline] void replace(inout Outer x, FloatE4M3 y)
            { x.inner.value = y; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Outer p = {{bit_cast<FloatE4M3>(uint8_t(tid.x))}};
                replace(p, p.inner.value);
                outputBuffer[0] = uint(bit_cast<uint8_t>(p.inner.value));
            }
        )SLANG",
         "exported substandard record helper reference"},
        // Public pointer syntax produces a UserPointer result. The separate exact Generic
        // local-pointer result exclusion remains a source audit, not a claim about this case.
        {R"SLANG(
            [noinline] Ptr<Outer> address(inout Outer x) { return &x; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Outer p = {{bit_cast<FloatE4M3>(uint8_t(tid.x))}};
                outputBuffer[0] = uint(bit_cast<uint8_t>((*address(p)).inner.value));
            }
        )SLANG",
         "helper function result type"},
        {R"SLANG(
            struct ArrayChild { Payload values[2]; }
            [noinline] ArrayChild copy(ArrayChild x) { return x; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p = {bit_cast<FloatE4M3>(uint8_t(tid.x))};
                ArrayChild x = {{p, p}};
                outputBuffer[0] = uint(bit_cast<uint8_t>(copy(x).values[1].value));
            }
        )SLANG",
         "helper function result type"},
    };
    for (const auto& test : cases)
    {
        _resetDirectNVVMFakes();
        {
            StringBuilder source;
            source << "struct Payload { FloatE4M3 value; }; struct Outer { Payload inner; };\n"
                   << test.source;
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
            const String text = _getBlobText(diagnostics);
            if (text.indexOf(test.construct) < 0)
                getTestReporter()->message(TestMessageType::TestFailure, text.getBuffer());
            SLANG_CHECK(text.indexOf("E52017") >= 0);
            SLANG_CHECK(text.indexOf(test.construct) >= 0);
            SLANG_CHECK(code == nullptr);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }

    // BF3/BF4 retain their flat local-only representation; nesting does not make them values.
    for (uint32_t width : {3u, 4u})
    {
        _resetDirectNVVMFakes();
        {
            StringBuilder source;
            source << "struct Payload { vector<BFloat16," << width << "> value; }" << R"SLANG(
                struct Outer { Payload inner; }
                [noinline] void initialize(out Outer x, BFloat16 v) { x.inner.value = v; }
                RWStructuredBuffer<uint> outputBuffer;
                [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
                {
                    Outer p;
                    initialize(p, bit_cast<BFloat16>(uint16_t(tid.x)));
                    outputBuffer[0] = uint(bit_cast<uint16_t>(p.inner.value.x));
                }
            )SLANG";
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
            const String text = _getBlobText(diagnostics);
            SLANG_CHECK(text.indexOf("E52017") >= 0);
            SLANG_CHECK(text.indexOf("helper function parameter") >= 0);
            SLANG_CHECK(code == nullptr);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangUnsupportedIRStopsBeforeEmission)
{
    struct UnsupportedCase
    {
        const char* source;
        const char* expectedConstruct;
    };
    static const UnsupportedCase kCases[] = {
        // Local BF3/BF4 record fields do not admit readonly, nested, by-value or external roles.
        {R"SLANG(
            struct Payload { vector<BFloat16,3> value; }
            [CudaDeviceExport] [noinline]
            void initialize(out Payload x) { x.value = vector<BFloat16,3>(BFloat16(1.0f)); }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            { Payload p; initialize(p); outputBuffer[tid.x] = uint(bit_cast<uint16_t>(p.value.x)); }
        )SLANG",
         "exported BF16 record helper reference"},
        {R"SLANG(
            struct Payload { vector<BFloat16,3> value; }
            [noinline] void initialize(out Payload x)
            { x.value = vector<BFloat16,3>(BFloat16(1.0f)); }
            [noinline] BFloat16 read(__constref Payload x) { return x.value.x; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            { Payload p; initialize(p); outputBuffer[tid.x] = uint(bit_cast<uint16_t>(read(p))); }
        )SLANG",
         "helper function parameter"},
        {R"SLANG(
            struct Payload { vector<BFloat16,3> value; }
            struct Outer { Payload inner; }
            [noinline] void initialize(out Outer x)
            { x.inner.value = vector<BFloat16,3>(BFloat16(1.0f)); }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            { Outer p; initialize(p); outputBuffer[tid.x] = uint(bit_cast<uint16_t>(p.inner.value.x)); }
        )SLANG",
         "helper function parameter"},
        {R"SLANG(
            struct Payload { vector<BFloat16,3> value; }
            RWStructuredBuffer<Payload> outputBuffer;
            [numthreads(32,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            { outputBuffer[tid.x].value = vector<BFloat16,3>(BFloat16(1.0f)); }
        )SLANG",
         "struct field address result"},
        // Bare local BF16 references do not qualify external helpers or numeric casts.

        {R"SLANG(
            [CudaDeviceExport]
            [noinline]
            vector<BFloat16,3> exportedResult(BFloat16 x)
            {
                return vector<BFloat16,3>(x);
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32,1,1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[tid.x] = uint(bit_cast<uint16_t>(exportedResult(bit_cast<BFloat16>(uint16_t(tid.x))).x));
            }
        )SLANG",
         "exported BF16 vector helper result"},
        {R"SLANG(
            [CudaDeviceExport]
            [noinline]
            BFloat16 exportedParameter(vector<BFloat16,3> x)
            {
                return x.y;
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32,1,1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[tid.x] = uint(bit_cast<uint16_t>(exportedParameter(vector<BFloat16,3>(bit_cast<BFloat16>(uint16_t(tid.x))))));
            }
        )SLANG",
         "exported BF16 vector helper parameter"},
        {R"SLANG(
            [CudaDeviceExport] [noinline] void replace(inout vector<BFloat16,2> x, vector<BFloat16,2> y) { x = y; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            { let v = vector<BFloat16,2>(bit_cast<BFloat16>(uint16_t(tid.x))); vector<BFloat16,2> local = v; replace(local, v); outputBuffer[tid.x] = bit_cast<uint>(local); }
        )SLANG",
         "exported BF16 vector helper reference"},
        {R"SLANG(
            struct Payload { vector<BFloat16,3> value; }
            [noinline] Payload copy(Payload x) { return x; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            { Payload p = {vector<BFloat16,3>(bit_cast<BFloat16>(uint16_t(tid.x)))}; outputBuffer[tid.x] = uint(bit_cast<uint16_t>(copy(p).value.x)); }
        )SLANG",
         "helper function result type"},
        {R"SLANG(
            struct Payload { vector<BFloat16,3> value; }
            [noinline] vector<BFloat16,3> read(__constref Payload x) { return x.value; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p;
                p.value = vector<BFloat16,3>(bit_cast<BFloat16>(uint16_t(tid.x)));
                outputBuffer[0] = uint(bit_cast<uint16_t>(read(p).x));
            }
        )SLANG",
         "helper function parameter"},
        {R"SLANG(
            RWStructuredBuffer<vector<BFloat16,4>> outputBuffer;
            [numthreads(32, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            { outputBuffer[tid.x] = vector<BFloat16,4>(bit_cast<BFloat16>(uint16_t(tid.x))); }
        )SLANG",
         "struct field address result"},
        {R"SLANG(
            RWStructuredBuffer<BFloat16> outputBuffer;
            [numthreads(32, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            { outputBuffer[tid.x] = bit_cast<BFloat16>(uint16_t(tid.x)); }
        )SLANG",
         "struct field address result"},

        {R"SLANG(
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[tid.x] = uint(bit_cast<uint16_t>(BFloat16(int(tid.x))));
            }
        )SLANG",
         "castIntToFloat"},
        {R"SLANG(
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[tid.x] = uint(bit_cast<uint16_t>(BFloat16(double(tid.x))));
            }
        )SLANG",
         "floatCast"},
        {R"SLANG(
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(32, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[tid.x] = uint(bit_cast<uint16_t>(BFloat16(half(tid.x))));
            }
        )SLANG",
         "floatCast"},
        // Additional scalar widths retain exact mask, lane and result/payload contracts.
        {R"SLANG(
            uint64_t malformedShuffle(uint64_t mask, uint64_t value, int lane)
            {
                __intrinsic_asm "__shfl_sync($0, $1, $2)";
            }
            RWStructuredBuffer<uint64_t> outputBuffer;
            [numthreads(32, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[tid.x] = malformedShuffle(uint64_t(0xFFFFFFFFu), uint64_t(tid.x), 0);
            }
        )SLANG",
         "GenericAsm assembly=__shfl_sync($0, $1, $2), signature=uint64_t(uint64_t, uint64_t, "
         "int)"},
        {R"SLANG(
            uint64_t malformedShuffle(uint mask, uint value, int lane)
            {
                __intrinsic_asm "__shfl_sync($0, $1, $2)";
            }
            RWStructuredBuffer<uint64_t> outputBuffer;
            [numthreads(32, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[tid.x] = malformedShuffle(0xFFFFFFFFu, tid.x, 0);
            }
        )SLANG",
         "GenericAsm assembly=__shfl_sync($0, $1, $2), signature=uint64_t(uint, uint, int)"},
        {R"SLANG(
            uint64_t malformedShuffle(uint mask, uint64_t value, float lane)
            {
                __intrinsic_asm "__shfl_sync($0, $1, $2)";
            }
            RWStructuredBuffer<uint64_t> outputBuffer;
            [numthreads(32, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[tid.x] = malformedShuffle(0xFFFFFFFFu, uint64_t(tid.x), 0.0f);
            }
        )SLANG",
         "GenericAsm assembly=__shfl_sync($0, $1, $2), signature=uint64_t(uint, uint64_t, float)"},
        // Local packed-vector legalization must not turn shared or external lane writes into
        // whole-vector read/modify/write operations: independent lanes may have different writers.
        {R"(
            groupshared bool4 flags;
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(4, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                flags[tid.x] = (tid.x % 2) == 0;
                GroupMemoryBarrierWithGroupSync();
                outputBuffer[tid.x] = uint(flags[tid.x]);
            }
        )",
         "'array element pointer relation'"},
        {R"(
            RWStructuredBuffer<bool4> flags;
            [numthreads(4, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                flags[0][tid.x] = (tid.x % 2) == 0;
            }
        )",
         "'sequential element pointer: Ptr<bool,"},
        {kDirectNVVMUnsupportedPointerHelperParameterSource, "'helper function parameter:"},
        {kDirectNVVMUnsupportedPointerHelperResultSource, "'helper function result type:"},
        {kDirectNVVMUnsupportedFloatArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedHalfAddSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedDoubleAddSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedNestedArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedStructPointerSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedArrayPointerHelperSource, "'helper function parameter:"},
        {kDirectNVVMNonCanonicalCUDAOffsetSource, "'CUDA layout query'"},
        // Clocks admit only their exact zero-operand scalar contracts and canonical bodies.
        {R"SLANG(
            int probe()
            {
                __intrinsic_asm "clock";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe());
            }
        )SLANG",
         "GenericAsm assembly=clock, signature="},
        {R"SLANG(
            uint64_t probe()
            {
                __intrinsic_asm "clock";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe());
            }
        )SLANG",
         "GenericAsm assembly=clock, signature="},
        {R"SLANG(
            float probe()
            {
                __intrinsic_asm "clock";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe());
            }
        )SLANG",
         "GenericAsm assembly=clock, signature="},
        {R"SLANG(
            uint2 probe()
            {
                __intrinsic_asm "clock";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe().x);
            }
        )SLANG",
         "GenericAsm assembly=clock, signature="},
        {R"SLANG(
            uint probe(uint p)
            {
                __intrinsic_asm "clock";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe(1));
            }
        )SLANG",
         "GenericAsm assembly=clock, signature="},
        {R"SLANG(
            uint64_t probe()
            {
                __intrinsic_asm "clock64";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe());
            }
        )SLANG",
         "GenericAsm assembly=clock64, signature="},
        {R"SLANG(
            int probe()
            {
                __intrinsic_asm "clock64";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe());
            }
        )SLANG",
         "GenericAsm assembly=clock64, signature="},
        {R"SLANG(
            int64_t probe(uint p)
            {
                __intrinsic_asm "clock64";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe(1));
            }
        )SLANG",
         "GenericAsm assembly=clock64, signature="},
        {R"SLANG(
            uint probe()
            {
                GroupMemoryBarrierWithGroupSync();
                __intrinsic_asm "clock";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe());
            }
        )SLANG",
         "GenericAsm assembly=clock, signature="},
        {R"SLANG(
            int64_t probe()
            {
                GroupMemoryBarrierWithGroupSync();
                __intrinsic_asm "clock64";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                data[cudaThreadIdx().x] = uint(probe());
            }
        )SLANG",
         "GenericAsm assembly=clock64, signature="},
        // Quad helper ownership must not suppress standalone or noncanonical requirements.
        {R"SLANG(
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                uint lane = cudaThreadIdx().x;
                __requireMaximallyReconverges();
                data[lane] = lane;
            }
        )SLANG",
         "RequireMaximallyReconverges"},
        {R"SLANG(
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                uint lane = cudaThreadIdx().x;
                __requireQuadDerivatives();
                data[lane] = lane;
            }
        )SLANG",
         "RequireQuadDerivatives"},
        {R"SLANG(
            bool probe(bool p)
            {
                __requireMaximallyReconverges();
                __intrinsic_asm "_slang_quadAny";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                uint lane = cudaThreadIdx().x;
                data[lane] = uint(probe(data[lane] != 0));
            }
        )SLANG",
         "RequireMaximallyReconverges"},
        {R"SLANG(
            bool probe(bool p)
            {
                __requireQuadDerivatives();
                __intrinsic_asm "_slang_quadAll";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                uint lane = cudaThreadIdx().x;
                data[lane] = uint(probe(data[lane] != 0));
            }
        )SLANG",
         "RequireQuadDerivatives"},
        {R"SLANG(
            bool probe(bool p)
            {
                __requireMaximallyReconverges();
                __requireQuadDerivatives();
                __intrinsic_asm "_slang_quadAny_other";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                uint lane = cudaThreadIdx().x;
                data[lane] = uint(probe(data[lane] != 0));
            }
        )SLANG",
         "RequireMaximallyReconverges"},
        {R"SLANG(
            uint probe(bool p)
            {
                __intrinsic_asm "_slang_quadAny";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                uint lane = cudaThreadIdx().x;
                data[lane] = probe(data[lane] != 0);
            }
        )SLANG",
         "GenericAsm assembly=_slang_quadAny, signature=uint(bool)"},
        {R"SLANG(
            bool probe(uint p)
            {
                __intrinsic_asm "_slang_quadAll";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                uint lane = cudaThreadIdx().x;
                data[lane] = uint(probe(data[lane]));
            }
        )SLANG",
         "GenericAsm assembly=_slang_quadAll, signature=bool(uint)"},
        {R"SLANG(
            bool probe(bool p)
            {
                __requireMaximallyReconverges();
                __requireQuadDerivatives();
                GroupMemoryBarrierWithGroupSync();
                __intrinsic_asm "_slang_quadAny";
            }
            [CUDAKernel]
            void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
            {
                uint lane = cudaThreadIdx().x;
                data[lane] = uint(probe(data[lane] != 0));
            }
        )SLANG",
         "RequireMaximallyReconverges"},
        {kDirectNVVMUnsupportedScalarTruthinessSignatureSource, "'GenericAsm assembly="},
        {kDirectNVVMUnsupportedMinMaxSignatureSource, "assembly=$P_min($0, $1)"},
        {kDirectNVVMUnsupportedIntegerBitSignatureSource, "assembly=$P_countbits($0)"},
        {kDirectNVVMUnsupportedVectorIntegerBitSource, "assembly=$P_reversebits($0)"},
        {kDirectNVVMUnsupportedVectorScalarMathSource, "assembly=$P_tan($0)"},
        {kDirectNVVMUnsupportedScalarIntrinsicRecipeSignatureSource,
         "assembly=$P_asuint($0, $1, $2)"},
        {kDirectNVVMUnsupportedCompoundWaveSignatureSource,
         "assembly=_waveShuffleMultiple($0, $1, $2)"},
        {kDirectNVVMUnsupportedAggregateWaveSignatureSource,
         "assembly=_waveShuffleMultiple($0, $1, $2)"},
        {kDirectNVVMUnsupportedMaskedWaveScalarSignatureSource, "assembly=_waveSum($1.x, $0)"},
        {R"SLANG(
            double unsupportedWave(double value, uint mask)
            {
                __target_switch
                {
                case cuda: __intrinsic_asm "_waveSum($1.x, $0)";
                default: return value;
                }
            }
            [CUDAKernel]
            void computeMain(
                uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
                uniform uint mask)
            {
                *destination = int(unsupportedWave(double(1), uint(mask)));
            }
        )SLANG",
         "assembly=_waveSum($1.x, $0)"},
        {R"SLANG(
            double2 unsupportedWave(double2 value, uint2 mask)
            {
                __target_switch
                {
                case cuda: __intrinsic_asm "_waveSumMultiple($1.x, $0)";
                default: return value;
                }
            }
            [CUDAKernel]
            void computeMain(
                uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
                uniform uint mask)
            {
                *destination = int(unsupportedWave(double2(1), uint2(mask)).x);
            }
        )SLANG",
         "assembly=_waveSumMultiple($1.x, $0)"},
        {R"SLANG(
            double unsupportedWave(double value, uint4 mask)
            {
                __target_switch
                {
                case cuda: __intrinsic_asm "_waveAnd($1.x, $0)";
                default: return value;
                }
            }
            [CUDAKernel]
            void computeMain(
                uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
                uniform uint mask)
            {
                *destination = int(unsupportedWave(double(1), uint4(mask)));
            }
        )SLANG",
         "assembly=_waveAnd($1.x, $0)"},
        {R"SLANG(
            double unsupportedWave(double value, uint4 mask)
            {
                __target_switch
                {
                case cuda: __intrinsic_asm "_wavePrefixMin($1.x, $0)";
                default: return value;
                }
            }
            [CUDAKernel]
            void computeMain(
                uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
                uniform uint mask)
            {
                *destination = int(unsupportedWave(double(1), uint4(mask)));
            }
        )SLANG",
         "assembly=_wavePrefixMin($1.x, $0)"},
        {R"SLANG(
            double2 unsupportedWave(double2 value, uint4 mask)
            {
                __target_switch
                {
                case cuda: __intrinsic_asm "_wavePrefixMinMultiple($1.x, $0)";
                default: return value;
                }
            }
            [CUDAKernel]
            void computeMain(
                uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
                uniform uint mask)
            {
                *destination = int(unsupportedWave(double2(1), uint4(mask)).x);
            }
        )SLANG",
         "assembly=_wavePrefixMinMultiple($1.x, $0)"},
        {R"SLANG(
            double2 unsupportedWave(double2 value, uint4 mask)
            {
                __target_switch
                {
                case cuda: __intrinsic_asm "_waveMin($1.x, $0)";
                default: return value;
                }
            }
            [CUDAKernel]
            void computeMain(
                uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
                uniform uint mask)
            {
                *destination = int(unsupportedWave(double2(1), uint4(mask)).x);
            }
        )SLANG",
         "assembly=_waveMin($1.x, $0)"},
        {kDirectNVVMUnsupportedOpaqueHalfConversionSignatureSource, "'GenericAsm assembly="},
        {kDirectNVVMUnsupportedSurfaceSignatureSource, "'GenericAsm assembly="},
        {kDirectNVVMLogicalNotSource, "'entry-point parameter'"},
        {kDirectNVVMAcquireGlobalI32AtomicAddSource, "'atomicAdd'"},
        {kDirectNVVMPointerEqualSource, "'cmpEQ'"},
        {kDirectNVVMPointerNotEqualSource, "'cmpNE'"},
        {kDirectNVVMPointerGreaterThanSource, "'cmpGT'"},
        {kDirectNVVMPointerLessEqualSource, "'cmpLE'"},
        {kDirectNVVMPointerGreaterEqualSource, "'cmpGE'"},
    };

    // Noncanonical layout, unsupported shared storage, logical NOT,
    // malformed-signature opaque-Half, scalar-math, and surface helpers, atomic-add ABI variants,
    // non-relaxed atomic-add order, adjacent atomic operations, non-integer shared arrays, pointer
    // comparisons, and helper-array-pointer shapes remain deterministic
    // before builder discovery.
    for (const auto& unsupported : kCases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(globalSession, unsupported.source, code, diagnostics)));
            SLANG_CHECK(code == nullptr);
            const String diagnosticText = _getBlobText(diagnostics);
            SLANG_CHECK(diagnosticText.indexOf("E52017") >= 0);
            if (diagnosticText.indexOf(unsupported.expectedConstruct) < 0)
            {
                StringBuilder message;
                message << "Expected unsupported construct " << unsupported.expectedConstruct
                        << ", but received: " << diagnosticText;
                getTestReporter()->message(TestMessageType::TestFailure, message.getBuffer());
            }
            SLANG_CHECK(diagnosticText.indexOf(unsupported.expectedConstruct) >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.successfulLoadCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

// Both FP8-only record arrays now use the same internal value-parameter contract as mixed records.
// Preserve the former rejection source, but validate its aggregate call with the real builder.
SLANG_UNIT_TEST(nvvmSlangFloat8RecordArrayParametersReturnRecords)
{
    NVVMIRBuilder realBuilder;
    _requireRealNVVMBuilder(unitTestContext, realBuilder);
    for (const char* format : {"FloatE4M3", "FloatE5M2"})
    {
        _resetDirectNVVMFakes();
        {
            StringBuilder source;
            source << "typealias F = " << format << ";\n"
                   << R"SLANG(
            struct Payload { F value; }
            [noinline] Payload copy(Payload x[2]) { return x[1]; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p = {bit_cast<F>(uint8_t(tid.x))};
                Payload a[2] = {p, p}; outputBuffer[0] = uint(bit_cast<uint8_t>(copy(a).value));
            }

            )SLANG";
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new RealBuilderFakeNVVMLoader);
            session->setSharedLibraryLoader(loader);
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const auto result =
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
            if (SLANG_FAILED(result))
                getTestReporter()->message(
                    TestMessageType::Info,
                    _getBlobText(diagnostics).getBuffer());
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
            const String& assembly = gFakeNVVM.addedModule;
            SLANG_CHECK(assembly.indexOf("define internal { i8 }") >= 0);
            SLANG_CHECK(assembly.indexOf("([2 x { i8 }] %slangParameter0)") >= 0);
            SLANG_CHECK(assembly.indexOf("call { i8 }") >= 0);
            SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
            SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

// Scalar FP8 transport/widening does not establish other casts, storage or an external ABI.
SLANG_UNIT_TEST(nvvmSlangFloat8UnsupportedRolesStopBeforeEmission)
{
    struct UnsupportedCase
    {
        const char* source;
        const char* expectedConstruct;
    };
    static const UnsupportedCase cases[] = {
        {R"SLANG(
            struct Payload { F value; }
            [CudaDeviceExport] [noinline] Payload exported(Payload x) { return x; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p = {bit_cast<F>(uint8_t(tid.x))};
                outputBuffer[0] = uint(bit_cast<uint8_t>(exported(p).value));
            }
        )SLANG",
         "exported substandard record helper result"},
        {R"SLANG(
            struct Payload { F value; }
            [CudaDeviceExport] [noinline] uint exported(Payload x) { return uint(bit_cast<uint8_t>(x.value)); }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p = {bit_cast<F>(uint8_t(tid.x))};
                outputBuffer[0] = exported(p);
            }
        )SLANG",
         "exported substandard record helper parameter"},
        {R"SLANG(
            struct Payload { F value; }
            [CudaDeviceExport] [noinline] void exported(inout Payload x, F y) { x.value = y; }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p = {bit_cast<F>(uint8_t(tid.x))};
                exported(p, p.value); outputBuffer[0] = uint(bit_cast<uint8_t>(p.value));
            }
        )SLANG",
         "exported substandard record helper reference"},
        {R"SLANG(
            struct Payload { F value; }
            [noinline] uint read(__constref Payload x) { return uint(bit_cast<uint8_t>(x.value)); }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p = {bit_cast<F>(uint8_t(tid.x))};
                outputBuffer[0] = read(p);
            }
        )SLANG",
         "helper function parameter"},
        {R"SLANG(
            [noinline]
            F copy(F v)
            {
                return v;
            }

            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain()
            {
                outputBuffer[0] = uint(bit_cast<uint8_t>(copy(F(1.0e30f))));
            }
        )SLANG",
         "nonfinite FP8 literal"},
        {R"SLANG(
            [noinline]
            void replace(inout F v, F w)
            {
                v = w;
            }

            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                F v = bit_cast<F>(uint8_t(tid.x));
                replace(v, v);
                outputBuffer[0] = uint(bit_cast<uint8_t>(v));
            }
        )SLANG",
         "helper function parameter"},
        {R"SLANG(
            [noinline]
            F copy(F v[2])
            {
                return v[1];
            }

            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                F v = bit_cast<F>(uint8_t(tid.x));
                F a[2] = {v, v};
                outputBuffer[0] = uint(bit_cast<uint8_t>(copy(a)));
            }
        )SLANG",
         "helper function parameter"},
        {R"SLANG(
            RWStructuredBuffer<F> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[0] = bit_cast<F>(uint8_t(tid.x));
            }
        )SLANG",
         "struct field address result"},
        {R"SLANG(
            [noinline]
            vector<F, 2> copy(vector<F, 2> v)
            {
                return v;
            }

            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                let value = vector<F, 2>(bit_cast<F>(uint8_t(tid.x)));
                outputBuffer[0] = uint(bit_cast<uint8_t>(copy(value).x));
            }
        )SLANG",
         "helper function result type"},
        {R"SLANG(
            [CudaDeviceExport]
            [noinline]
            F copy(uint8_t v)
            {
                return bit_cast<F>(v);
            }

            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[0] = uint(bit_cast<uint8_t>(copy(uint8_t(tid.x))));
            }
        )SLANG",
         "exported FP8 helper result"},
        {R"SLANG(
            [CudaDeviceExport]
            [noinline]
            uint8_t copy(F v)
            {
                return bit_cast<uint8_t>(v);
            }

            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[0] = uint(copy(bit_cast<F>(uint8_t(tid.x))));
            }
        )SLANG",
         "exported FP8 helper parameter"},
        {R"SLANG(
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[0] = uint(bit_cast<uint8_t>(F(asfloat(tid.x))));
            }
        )SLANG",
         "floatCast"},
        {R"SLANG(
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[0] = uint(bit_cast<uint8_t>(F(tid.x)));
            }
        )SLANG",
         "castIntToFloat"},
    };
    for (const char* format : {"FloatE4M3", "FloatE5M2"})
    {
        for (const auto& unsupported : cases)
        {
            _resetDirectNVVMFakes();
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            StringBuilder source;
            source << "typealias F = " << format << ";\n" << unsupported.source;
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.indexOf(unsupported.expectedConstruct) < 0)
                getTestReporter()->message(
                    TestMessageType::TestFailure,
                    diagnosticText.getBuffer());
            SLANG_CHECK(diagnosticText.indexOf("E52017") >= 0);
            SLANG_CHECK(diagnosticText.indexOf(unsupported.expectedConstruct) >= 0);
            SLANG_CHECK(code == nullptr);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangMissingBuilderDoesNotFallback)
{
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.libraryUnavailable = true;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        const String firstDiagnostic = _getBlobText(diagnostics);
        SLANG_CHECK(firstDiagnostic.indexOf("E52016") >= 0);
        const String expectedSearchPath = _getExpectedNVVMBuilderSearchPath();
        if (expectedSearchPath.getLength())
        {
            SLANG_CHECK(firstDiagnostic.indexOf(expectedSearchPath.getUnownedSlice()) >= 0);
            SLANG_CHECK(
                gFakeNVVMBuilder.loadedPath.getUnownedSlice().indexOf(
                    expectedSearchPath.getUnownedSlice()) == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);

        // A failed provider load is a session result too. Recompiling must report the same
        // resolved location without another load attempt or a fallback to NVRTC.
        code.setNull();
        diagnostics.setNull();
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// Local array values and storage must retain one LLVM element representation, including BF2
// padding. Exercise aggregate construction and field-first initialization before a whole copy.
SLANG_UNIT_TEST(nvvmSlangLocalSubstandardRecordArraysPreserveValuesAndAddresses)
{
    NVVMIRBuilder realBuilder;
    _requireRealNVVMBuilder(unitTestContext, realBuilder);
    const char* initializations[] = {
        R"SLANG(
            Payload values[2] = {make(tid.x), make(tid.x ^ 65535)};
        )SLANG",
        R"SLANG(
            Payload values[2];
            for (uint k = 0; k < 2; ++k)
            {
                values[k].before = uint16_t(k + 17);
                values[k].pair = vector<BFloat16, 2>(
                    bit_cast<BFloat16>(uint16_t(tid.x + k)),
                    bit_cast<BFloat16>(uint16_t(tid.x ^ k ^ 65535)));
                values[k].after = uint16_t(k + 23);
            }
        )SLANG",
    };
    for (Index initializationIndex = 0; initializationIndex < SLANG_COUNT_OF(initializations);
         ++initializationIndex)
    {
        _resetDirectNVVMFakes();
        StringBuilder source;
        source << R"SLANG(
            struct Payload { uint16_t before; vector<BFloat16, 2> pair; uint16_t after; }
            struct Outer { uint8_t tag; Payload inner; uint tail; }
            [noinline] Payload make(uint bits)
            {
                Payload value = {uint16_t(17), vector<BFloat16, 2>(
                    bit_cast<BFloat16>(uint16_t(bits)),
                    bit_cast<BFloat16>(uint16_t(bits ^ 65535))), uint16_t(23)};
                return value;
            }
            [noinline] uint read(Payload value)
            { return uint(bit_cast<uint16_t>(value.pair.x)) + uint(value.after); }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
        )SLANG" << initializations[initializationIndex]
               << R"SLANG(
                let saved = values;
                values[tid.x & 1].pair[tid.y & 1] = bit_cast<BFloat16>(uint16_t(tid.z));
                Outer nested[2];
                for (uint k = 0; k < 2; ++k)
                {
                    nested[k].tag = uint8_t(k);
                    nested[k].inner = values[k];
                    nested[k].tail = k + 37;
                }
                let savedNested = nested;
                nested[tid.y & 1].inner.pair[tid.z & 1] = bit_cast<BFloat16>(uint16_t(99));
                outputBuffer[0] = read(saved[0]);
                outputBuffer[1] = read(saved[tid.x & 1]);
                outputBuffer[2] = read(values[tid.x & 1]);
                outputBuffer[3] = read(savedNested[tid.y & 1].inner);
                outputBuffer[4] = read(nested[tid.y & 1].inner);
            }
        )SLANG";
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new RealBuilderFakeNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const auto result =
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);
        const String& assembly = gFakeNVVM.addedModule;
        SLANG_CHECK(assembly.indexOf("alloca [2 x { i16, <2 x i16>, i16 }], align 4") >= 0);
        // Aggregate initialization preserves the snapshot as an SSA value and stores the
        // constructed array. Field-first initialization instead loads its whole snapshot.
        // Neither path needs the other memory operation after frontend value propagation.
        SLANG_CHECK(
            assembly.indexOf(
                initializationIndex == 0 ? "store [2 x { i16, <2 x i16>, i16 }]"
                                         : "load [2 x { i16, <2 x i16>, i16 }]") >= 0);
        SLANG_CHECK(assembly.indexOf("getelementptr [2 x { i16, <2 x i16>, i16 }]") >= 0);
        SLANG_CHECK(
            assembly.indexOf("getelementptr inbounds { i8, { i16, <2 x i16>, i16 }, i32 }") >= 0);
        SLANG_CHECK(assembly.indexOf("getelementptr <2 x i16>") >= 0);
        SLANG_CHECK(assembly.indexOf("extractvalue [2 x { i16, <2 x i16>, i16 }]") >= 0);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// CUDA's aligned element size owns array stride; Natural layout would incorrectly use eight
// bytes for this padded BF2 record instead of twelve, and sixteen for Outer instead of twenty.
SLANG_UNIT_TEST(nvvmSlangLocalSubstandardRecordArrayLayoutQueries)
{
    const char* source = R"SLANG(
        struct Payload { uint16_t before; vector<BFloat16, 2> pair; uint16_t after; }
        struct Outer { uint8_t tag; Payload inner; uint tail; }
        typedef Payload Pair[2];
        typedef Outer NestedPair[2];
        RWStructuredBuffer<int> outputBuffer;
        [numthreads(1, 1, 1)] void computeMain()
        {
            Payload payload;
            Outer outer;
            outputBuffer[0] = __sizeOf<Payload>();
            outputBuffer[1] = __alignOf<Payload>();
            outputBuffer[2] = __offsetOf(payload, payload.pair);
            outputBuffer[3] = __offsetOf(payload, payload.after);
            outputBuffer[4] = __sizeOf<Pair>();
            outputBuffer[5] = __alignOf<Pair>();
            outputBuffer[6] = __sizeOf<Outer>();
            outputBuffer[7] = __alignOf<Outer>();
            outputBuffer[8] = __offsetOf(outer, outer.inner);
            outputBuffer[9] = __offsetOf(outer, outer.tail);
            outputBuffer[10] = __sizeOf<NestedPair>();
        }
    )SLANG";
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const auto result = _compileSlangWithDirectNVVM(session, source, code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        const int64_t expected[] = {12, 4, 4, 8, 24, 4, 20, 4, 4, 16, 40};
        SLANG_CHECK_ABORT(gFakeNVVMBuilder.storeValueRefs.getCount() == SLANG_COUNT_OF(expected));
        for (Index i = 0; i < SLANG_COUNT_OF(expected); ++i)
        {
            const auto value = gFakeNVVMBuilder.storeValueRefs[i];
            SLANG_CHECK_ABORT(value.kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK_ABORT(value.index >= 0);
            SLANG_CHECK_ABORT(value.index < gFakeNVVMBuilder.integerConstantValues.getCount());
            SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[value.index] == expected[i]);
        }
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// The real builder preserves exact aggregate handles; the fake libNVVM compiler records LLVM IR.
SLANG_UNIT_TEST(nvvmSlangLocalRecordArrayParameterUsesCanonicalValueABI)
{
    NVVMIRBuilder realBuilder;
    _requireRealNVVMBuilder(unitTestContext, realBuilder);
    _resetDirectNVVMFakes();
    {
        const char* source = R"SLANG(
            struct Payload
            {
                uint16_t before;
                vector<BFloat16, 2> pair;
                uint16_t after;
            }
            typedef Payload Pair[2];
            [noinline] uint inspect(Pair values, uint slot, uint lane, uint bits)
            {
                values[slot].pair[lane] = bit_cast<BFloat16>(uint16_t(bits));
                return uint(bit_cast<uint16_t>(values[0].pair.x))
                    + uint(bit_cast<uint16_t>(values[1].pair.y))
                    + uint(values[slot].before) + uint(values[1-slot].after);
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Pair values;
                for (uint k = 0; k < 2; ++k)
                {
                    values[k].before = uint16_t(tid.x + k + 17);
                    values[k].pair = vector<BFloat16, 2>(
                        bit_cast<BFloat16>(uint16_t(tid.x + k)),
                        bit_cast<BFloat16>(uint16_t(tid.x ^ k ^ 65535)));
                    values[k].after = uint16_t(tid.x + k + 23);
                }
                outputBuffer[0] = inspect(values, tid.y & 1, tid.z & 1, tid.x ^ 0x3333);
                outputBuffer[1] = uint(bit_cast<uint16_t>(values[0].pair.x));
            }
        )SLANG";
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new RealBuilderFakeNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(session, source, code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        const String& assembly = gFakeNVVM.addedModule;
        // Select the source inspect helper by its symbol, not by being the first internal
        // i32 function: execution-register helpers also have an i32 result and no parameters.
        String signature;
        Index cursor = 0;
        while ((cursor = assembly.indexOf("define internal i32 ", cursor)) >= 0)
        {
            const Index end = assembly.indexOf('\n', cursor);
            SLANG_CHECK_ABORT(end > cursor);
            const String candidate = assembly.subString(cursor, end - cursor);
            if (candidate.indexOf("inspect") >= 0)
            {
                SLANG_CHECK_ABORT(signature.getLength() == 0);
                signature = candidate;
            }
            cursor = end;
        }
        SLANG_CHECK_ABORT(signature.getLength() > 0);
        const char* arrayType = "[2 x { i16, <2 x i16>, i16 }]";
        SLANG_CHECK(
            signature.indexOf(
                "([2 x { i16, <2 x i16>, i16 }] %slangParameter0, i32 %slangParameter1, "
                "i32 %slangParameter2, i32 %slangParameter3)") >= 0);
        SLANG_CHECK(signature.indexOf("[2 x { i16, <2 x i16>, i16 }]*") < 0);

        // Match the actual declared symbol, so an unrelated intrinsic call cannot satisfy this.
        const Index nameStart = signature.indexOf('@');
        const Index nameEnd = signature.indexOf('(', nameStart);
        SLANG_CHECK_ABORT(nameStart >= 0 && nameEnd > nameStart);
        const String symbol = signature.subString(nameStart, nameEnd - nameStart);
        StringBuilder callPrefix;
        callPrefix << "call i32 " << symbol << "(" << arrayType << " ";
        SLANG_CHECK(assembly.indexOf(callPrefix.getUnownedSlice()) >= 0);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// Internal array value parameters do not grant references, results or external storage a new role.
// Keep each read observable so preflight sees the intended surviving role.
SLANG_UNIT_TEST(nvvmSlangLocalSubstandardRecordArraysRejectOtherRoles)
{
    struct Case
    {
        const char* name;
        const char* construct;
        const char* source;
    };
    // Array returns are lowered to an OutParam, so their rejection is a parameter contract.
    const Case cases[] = {
        {"exported array parameter", "exported substandard record array helper parameter", R"SLANG(
            struct Payload { BFloat16 value; }
            typedef Payload Pair[2];
            [CudaDeviceExport] [noinline] uint inspect(Pair values, uint slot)
            {
                return uint(bit_cast<uint16_t>(values[slot].value));
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Pair values;
                values[0].value = bit_cast<BFloat16>(uint16_t(tid.x));
                values[1].value = bit_cast<BFloat16>(uint16_t(tid.x ^ 65535));
                outputBuffer[0] = inspect(values, tid.y & 1);
            }
        )SLANG"},
        {"array wrapper parameter", "helper function parameter: Wrapper", R"SLANG(
            struct Payload { BFloat16 value; }
            struct Wrapper { Payload values[2]; }
            [noinline] uint inspect(Wrapper wrapper, uint slot)
            {
                return uint(bit_cast<uint16_t>(wrapper.values[slot].value));
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Wrapper wrapper;
                wrapper.values[0].value = bit_cast<BFloat16>(uint16_t(tid.x));
                wrapper.values[1].value = bit_cast<BFloat16>(uint16_t(tid.x ^ 65535));
                outputBuffer[0] = inspect(wrapper, tid.y & 1);
            }
        )SLANG"},
        {"multidimensional array parameter",
         "helper function parameter: Array<Array<Payload, 2>, 2>",
         R"SLANG(
            struct Payload { BFloat16 value; }
            typedef Payload Grid[2][2];
            [noinline] uint inspect(Grid values, uint row, uint column)
            {
                return uint(bit_cast<uint16_t>(values[row][column].value));
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Grid values;
                for (uint row = 0; row < 2; ++row)
                    for (uint column = 0; column < 2; ++column)
                        values[row][column].value = bit_cast<BFloat16>(
                            uint16_t(tid.x ^ (row * 0x3333) ^ (column * 0x5555)));
                outputBuffer[0] = inspect(values, tid.y & 1, tid.z & 1);
            }
        )SLANG"},
        {"array result", "helper function parameter: OutParam<Array<Payload, 2>>", R"SLANG(
            struct Payload
            {
                BFloat16 value;
            }
            typedef Payload Pair[2];
            [noinline] Pair make(uint v)
            {
                Pair p;
                p[0].value = p[1].value = bit_cast<BFloat16>(uint16_t(v));
                return p;
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                let p = make(tid.x);
                outputBuffer[0] = uint(bit_cast<uint16_t>(p[tid.y & 1].value));
            }
        )SLANG"},
        {"inout array", "helper function parameter: BorrowInOutParam<Array<Payload, 2>>", R"SLANG(
            struct Payload
            {
                BFloat16 value;
            }
            [noinline] void mutate(inout Payload p[2], uint v)
            {
                p[v & 1].value = bit_cast<BFloat16>(uint16_t(v));
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p[2];
                p[0].value = p[1].value = bit_cast<BFloat16>(uint16_t(1));
                mutate(p, tid.x);
                outputBuffer[0] = uint(bit_cast<uint16_t>(p[tid.y & 1].value));
            }
        )SLANG"},
        {"out array", "helper function parameter: OutParam<Array<Payload, 2>>", R"SLANG(
            struct Payload
            {
                BFloat16 value;
            }
            [noinline] void initialize(out Payload p[2], uint v)
            {
                p[0].value = p[1].value = bit_cast<BFloat16>(uint16_t(v));
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p[2];
                initialize(p, tid.x);
                outputBuffer[0] = uint(bit_cast<uint16_t>(p[tid.y & 1].value));
            }
        )SLANG"},
        {"readonly array", "helper function parameter: BorrowInParam<Array<Payload, 2>", R"SLANG(
            struct Payload
            {
                BFloat16 value;
            }
            [noinline] uint read(__constref Payload p[2], uint v)
            {
                return uint(bit_cast<uint16_t>(p[v & 1].value));
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p[2];
                p[0].value = p[1].value = bit_cast<BFloat16>(uint16_t(tid.x));
                outputBuffer[0] = read(p, tid.y);
            }
        )SLANG"},
        {"local wrapper", "'var'", R"SLANG(
            struct Payload
            {
                BFloat16 value;
            }
            struct Wrapper
            {
                Payload values[2];
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Wrapper p;
                p.values[0].value = p.values[1].value = bit_cast<BFloat16>(uint16_t(tid.x));
                outputBuffer[0] = uint(bit_cast<uint16_t>(p.values[tid.y & 1].value));
            }
        )SLANG"},
        {"multidimensional array", "'var'", R"SLANG(
            struct Payload
            {
                BFloat16 value;
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p[2][2];
                for (uint i = 0; i < 2; ++i)
                    for (uint j = 0; j < 2; ++j)
                        p[i][j].value = bit_cast<BFloat16>(uint16_t(tid.x + i + j));
                outputBuffer[0] = uint(bit_cast<uint16_t>(p[tid.y & 1][tid.z & 1].value));
            }
        )SLANG"},
        {"shared array", "sequential element pointer: Ptr<Payload", R"SLANG(
            struct Payload
            {
                BFloat16 value;
            }
            groupshared Payload p[2];
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                p[0].value = p[1].value = bit_cast<BFloat16>(uint16_t(tid.x));
                GroupMemoryBarrierWithGroupSync();
                outputBuffer[0] = uint(bit_cast<uint16_t>(p[tid.y & 1].value));
            }
        )SLANG"},
        {"resource array", "struct field address result: Ptr<StructuredBuffer<Wrapper>", R"SLANG(
            struct Payload
            {
                BFloat16 value;
            }
            struct Wrapper
            {
                Payload values[2];
            };
            StructuredBuffer<Wrapper> inputBuffer;
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[0] = uint(bit_cast<uint16_t>(inputBuffer[tid.x].values[tid.y & 1].value));
            }
        )SLANG"},
        {"uniform array", "struct field address result: Ptr<cbuffer<", R"SLANG(
            struct Payload
            {
                BFloat16 value;
            }
            cbuffer Params
            {
                Payload values[2];
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                outputBuffer[0] = uint(bit_cast<uint16_t>(values[tid.y & 1].value));
            }
        )SLANG"},
        {"BF3 array", "'var'", R"SLANG(
            struct Payload
            {
                vector<BFloat16, 3> value;
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p[2];
                for (uint k = 0; k < 2; ++k)
                    p[k].value = bit_cast<BFloat16>(uint16_t(tid.x + k));
                outputBuffer[0] = uint(bit_cast<uint16_t>(p[tid.y & 1].value[tid.z % 3]));
            }
        )SLANG"},
        {"BF4 array", "'var'", R"SLANG(
            struct Payload
            {
                vector<BFloat16, 4> value;
            }
            RWStructuredBuffer<uint> outputBuffer;
            [numthreads(1, 1, 1)]
            void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                Payload p[2];
                for (uint k = 0; k < 2; ++k)
                    p[k].value = bit_cast<BFloat16>(uint16_t(tid.x + k));
                outputBuffer[0] = uint(bit_cast<uint16_t>(p[tid.y & 1].value[tid.z % 4]));
            }
        )SLANG"},
    };
    for (const auto& test : cases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const auto result =
                _compileSlangWithDirectNVVM(session, test.source, code, diagnostics);
            const auto text = _getBlobText(diagnostics);
            if (SLANG_SUCCEEDED(result) || text.indexOf("E52017") < 0 ||
                text.indexOf(test.construct) < 0)
            {
                getTestReporter()->message(TestMessageType::TestFailure, test.name);
                getTestReporter()->message(TestMessageType::TestFailure, text.getBuffer());
            }
            SLANG_CHECK(SLANG_FAILED(result));
            SLANG_CHECK(text.indexOf("E52017") >= 0);
            SLANG_CHECK(text.indexOf(test.construct) >= 0);
            SLANG_CHECK(code == nullptr);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

// Keep the shader-visible decoded value observable through an ordinary buffer as well as the
// physical native store. The recording provider models resource-bearing global parameter blocks.
static const char kSurfaceFormatLegalizationSource[] = R"SLANG(
    RWStructuredBuffer<float> output;
    RWTexture1D<float> nativeSurface;
    [format("r16f")] RWTexture1D<float> halfSurface;
    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain(uint3 tid : SV_DispatchThreadID)
    {
        float value = nativeSurface.Load(int(tid.x));
        halfSurface[tid.x] = value;
        float restored = halfSurface.Load(int(tid.x));
        nativeSurface[tid.x] = restored;
        output[tid.x] = restored;
    }
)SLANG";

SLANG_UNIT_TEST(nvvmSurfaceLegalizationSeparatesPhysicalAccessAndConversion)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kSurfaceFormatLegalizationSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(gFakeNVVMBuilder.surfaceOperations.getCount() == 4);
        bool sawLoad[2] = {};
        bool sawStore[2] = {};
        for (Index i = 0; i < gFakeNVVMBuilder.surfaceOperations.getCount(); ++i)
        {
            const auto& operation = gFakeNVVMBuilder.surfaceOperations[i];
            SLANG_CHECK(operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT);
            SLANG_CHECK(operation.elementType.laneCount == 1);
            SLANG_CHECK(operation.shape == SLANG_NVVM_TEXTURE_SHAPE_1D);
            const bool isHalf = operation.elementType.bitWidth == 16;
            SLANG_CHECK(isHalf || operation.elementType.bitWidth == 32);
            const auto& operands = gFakeNVVMBuilder.surfaceOperationOperands[i];
            SLANG_CHECK_ABORT(operands[1].kind == FakeNVVMBuilderValueKind::ScalarOperation);
            const auto& coordinate = gFakeNVVMBuilder.scalarOperations[operands[1].index];
            SLANG_CHECK(coordinate.key.operation == SLANG_NVVM_VALUE_OP_MULTIPLY);
            SLANG_CHECK_ABORT(
                coordinate.operands[1].kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK(
                gFakeNVVMBuilder.integerConstantValues[coordinate.operands[1].index] ==
                (isHalf ? 2 : 4));
            if (operation.operation == SLANG_NVVM_SURFACE_OP_LOAD)
                sawLoad[isHalf] = true;
            else
            {
                sawStore[isHalf] = true;
                SLANG_CHECK_ABORT(operands[2].kind == FakeNVVMBuilderValueKind::ScalarOperation);
                const auto& conversion = gFakeNVVMBuilder.scalarOperations[operands[2].index];
                SLANG_CHECK(conversion.key.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT);
                SLANG_CHECK(conversion.resultType.bitWidth == operation.elementType.bitWidth);
                SLANG_CHECK(conversion.operandTypes[0].bitWidth == (isHalf ? 32 : 16));
                SLANG_CHECK_ABORT(
                    conversion.operands[0].kind == FakeNVVMBuilderValueKind::SurfaceOperation);
                const auto& load = gFakeNVVMBuilder.surfaceOperations[conversion.operands[0].index];
                SLANG_CHECK(load.operation == SLANG_NVVM_SURFACE_OP_LOAD);
                SLANG_CHECK(load.elementType.bitWidth == (isHalf ? 32 : 16));
            }
        }
        SLANG_CHECK(sawLoad[0] && sawLoad[1] && sawStore[0] && sawStore[1]);
        Index conversions = 0;
        for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
        {
            conversions += operation.key.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT;
            SLANG_CHECK(operation.key.operation != SLANG_NVVM_VALUE_OP_BIT_REINTERPRET);
        }
        SLANG_CHECK(conversions == 2);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSurfaceLegalizationChecksPhysicalCapabilitiesBeforeModuleCreation)
{
    _resetDirectNVVMFakes();
    {
        gFakeNVVMBuilder.rejectHalfSurfaceOperation = true;
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kSurfaceFormatLegalizationSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).contains("52018"));
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangResourceLegacyRoutesRejectBeforeOutput)
{
    struct Case
    {
        const char* declaration;
        const char* invocation;
        bool isTag;
    };
    const Case cases[] = {
        {R"SLANG(float4 legacy(Texture2D<float4> t, SamplerState s, float2 uv)
            { __intrinsic_asm(nvvmTextureSample) "unused"; })SLANG",
         "output[0] = legacy(texture, sampler, float2(0));",
         true},
        {R"SLANG(float legacy(RWTexture2D<float> t, int2 p)
            { __intrinsic_asm(nvvmSurfaceLoad) "unused"; })SLANG",
         "output[0] = legacy(image, int2(0));",
         true},
        {R"SLANG(void legacy(RWTexture2D<float> t, uint2 p, float value)
            { __intrinsic_asm(nvvmSurfaceStore) "unused"; })SLANG",
         "legacy(image, uint2(0), 1);",
         true},
        {R"SLANG(float4 legacy(Texture2D<float4> t, SamplerState s, float2 uv, float lod)
            { __intrinsic_asm "tex2DLod<$T0>($0, ($2).x, ($2).y, ($3))"; })SLANG",
         "output[0] = legacy(texture, sampler, float2(0), 0);",
         false},
        {R"SLANG(float4 legacy(Texture2D<float4> t, int3 p)
            { __intrinsic_asm "tex2Dfetch_int<$T0>($0, ($1).x, ($1).y, ($1).z)"; })SLANG",
         "output[0] = legacy(texture, int3(0));",
         false},
        {R"SLANG(float4 legacy(Texture2D<float4> t, SamplerState s, float2 uv)
            { __intrinsic_asm "tex2Dgather<$TR>($0, ($2).x, ($2).y, 1)"; })SLANG",
         "output[0] = legacy(texture, sampler, float2(0));",
         false},
        {R"SLANG(void legacy(Texture2D<float4> t, out uint w, out uint h)
            { __intrinsic_asm "{uint32_t w, h; asm(\"txq.width.b32 %0, [%2]; txq.height.b32 %1, [%2];\" : \"=r\"(w), \"=r\"(h) : \"l\"($0)); *($1) = w;*($2) = h;}"; })SLANG",
         "uint w, h; legacy(texture, w, h); output[0] = float4(w, h, 0, 0);",
         false},
    };
    for (const auto& test : cases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);
            StringBuilder source;
            source << test.declaration << R"SLANG(
                Texture2D<float4> texture;
                SamplerState sampler;
                RWTexture2D<float> image;
                RWStructuredBuffer<float4> output;
                [numthreads(1,1,1)] void computeMain() {
            )SLANG" << test.invocation
                   << "}";
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(globalSession, source.getBuffer(), code, diagnostics)));
            SLANG_CHECK(code == nullptr);
            const String text = _getBlobText(diagnostics);
            const char* expected = test.isTag ? "E36121" : "E52017";
            if (!text.contains(expected))
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
            SLANG_CHECK(text.contains(expected));
            if (!test.isTag)
                SLANG_CHECK(text.contains("GenericAsm assembly="));
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangResourceOperationsUseTypedInstructions)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        const char* source = R"SLANG(
            Texture2D<float4> texture;
            SamplerState sampler;
            RWTexture2DArray<uint> arrayImage;
            RWStructuredBuffer<float4> output;
            [numthreads(1,1,1)] void computeMain(uint3 tid : SV_DispatchThreadID)
            {
                uint width, height;
                texture.GetDimensions(width, height);
                output[0] = texture.SampleLevel(sampler, float2(tid.xy), float(tid.z));
                output[1] = texture.Load(int3(tid));
                arrayImage.Store(tid, width + height);
            }
        )SLANG";
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SlangResult result = _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.textureOperations.getCount() == 4);
        bool sawWidth = false, sawHeight = false, sawLevel = false, sawFetch = false;
        for (const auto& operation : gFakeNVVMBuilder.textureOperations)
        {
            SLANG_CHECK(operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D);
            SLANG_CHECK(operation.isArray == 0);
            SLANG_CHECK(operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT);
            SLANG_CHECK(operation.elementType.bitWidth == 32);
            SLANG_CHECK(operation.elementType.laneCount == 4);
            sawWidth |= operation.operation == SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH;
            sawHeight |= operation.operation == SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT;
            sawLevel |= operation.operation == SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL;
            sawFetch |= operation.operation == SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL;
        }
        SLANG_CHECK(sawWidth && sawHeight && sawLevel && sawFetch);
        SLANG_CHECK_ABORT(gFakeNVVMBuilder.surfaceOperations.getCount() == 1);
        const auto& store = gFakeNVVMBuilder.surfaceOperations[0];
        SLANG_CHECK(store.operation == SLANG_NVVM_SURFACE_OP_STORE);
        SLANG_CHECK(store.shape == SLANG_NVVM_TEXTURE_SHAPE_2D);
        SLANG_CHECK(store.isArray == 1);
        SLANG_CHECK(store.elementType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER);
        SLANG_CHECK(store.elementType.bitWidth == 32 && store.elementType.laneCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNamedIntrinsicsRejectBeforeModuleCreation)
{
    const char* bodies[] = {
        "uint read() { __intrinsic_asm \"llvm.nvvm.read.ptx.sreg.missing.x\"; }",
        "uint read() { __intrinsic_asm \"llvm.nvvm.read.ptx.sreg.tid.x()\"; }",
        "float read() { __intrinsic_asm \"llvm.nvvm.read.ptx.sreg.tid.x\"; }",
        "uint read() { __intrinsic_asm \"llvm.nvvm.read.ptx.sreg.tid.x\", 1; }",
        "uint read(uint x) { __intrinsic_asm \"llvm.ctpop\"; }",
    };
    for (Index i = 0; i < SLANG_COUNT_OF(bodies); ++i)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << bodies[i] << "\n[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output) { "
               << "output[0] = uint(read(" << (i == 4 ? "7" : "") << ")); }";
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(globalSession, source.getBuffer(), code, diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangNamedIntegerIntrinsicsPreserveExplicitOperands)
{
    _resetDirectNVVMFakes();
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
    globalSession->setSharedLibraryLoader(loader);
    const char* source = R"(
        uint pop(uint x) { __intrinsic_asm "llvm.ctpop", x; }
        uint reverse(uint x) { __intrinsic_asm "llvm.bitreverse", x; }
        uint high(uint x) { __intrinsic_asm "llvm.ctlz", x, false; }
        uint low(uint x) { __intrinsic_asm "llvm.cttz", x, false; }
        uint readUnused(uint ignored) { __intrinsic_asm "llvm.nvvm.read.ptx.sreg.tid.x"; }
        void syncUnused(uint ignored) { __intrinsic_asm "llvm.nvvm.barrier0"; }
        [CUDAKernel] void computeMain(
            uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
        {
            uint value = words[0];
            words[1] = pop(value);
            words[2] = reverse(value);
            words[3] = high(value);
            words[4] = low(value);
            words[5] = readUnused(value);
            syncUnused(value);
        }
    )";
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    const auto result = _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics);
    if (SLANG_FAILED(result))
        getTestReporter()->message(TestMessageType::Info, _getBlobText(diagnostics).getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    SLANG_CHECK(code != nullptr);
    SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 6);
    Index unusedParameterHelpers = 0;
    for (const auto& namedFunction : gFakeNVVMBuilder.namedIntrinsicFunctionNames)
    {
        if (namedFunction.second == "llvm.nvvm.read.ptx.sreg.tid.x" ||
            namedFunction.second == "llvm.nvvm.barrier0")
        {
            const Index type = gFakeNVVMBuilder.functionTypeIndices[namedFunction.first];
            SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[type] == 1);
            ++unusedParameterHelpers;
        }
    }
    SLANG_CHECK(unusedParameterHelpers == 2);
    Index scanCount = 0;
    for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
    {
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations[i] == UINT32_MAX);
        const Index function =
            gFakeNVVMBuilder.blockFunctionIndices[gFakeNVVMBuilder.intrinsicCallerBlockIndices[i]];
        const String& name = gFakeNVVMBuilder.namedIntrinsicFunctionNames[function];
        const Index count = gFakeNVVMBuilder.intrinsicArgumentCounts[i];
        if (name == "llvm.nvvm.read.ptx.sreg.tid.x")
        {
            SLANG_CHECK(count == 0);
            continue;
        }
        const bool isScan = name == "llvm.ctlz" || name == "llvm.cttz";
        SLANG_CHECK(count == (isScan ? 2 : 1));
        const Index offset = gFakeNVVMBuilder.intrinsicArgumentOffsets[i];
        const auto& first = gFakeNVVMBuilder.intrinsicArgumentValueRefs[offset];
        SLANG_CHECK(first.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(first.functionIndex == function);
        if (isScan)
        {
            ++scanCount;
            const auto& flag = gFakeNVVMBuilder.intrinsicArgumentValueRefs[offset + 1];
            SLANG_CHECK_ABORT(flag.kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[flag.index] == 0);
        }
    }
    SLANG_CHECK(scanCount == 2);
    SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNamedIntegerIntrinsicsRejectBeforeModuleCreation)
{
    const char* bodies[] = {
        "uint bits(uint x, bool b) { __intrinsic_asm \"llvm.ctlz\", x, b; }",
        "uint bits(uint x, bool b) { __intrinsic_asm \"llvm.ctpop\", x, false; }",
        "uint bits(uint x, bool b) { __intrinsic_asm \"llvm.ctpop\", uint; }",
        "uint bits(uint x, bool b) { __intrinsic_asm \"llvm.ctlz\", x; }",
        "uint bits(uint x, bool b) { __intrinsic_asm \"llvm.ctpop.i32\", x; }",
    };
    const char* names[] = {
        "llvm.ctlz",
        "llvm.ctpop",
        "GenericAsm assembly=llvm.ctpop",
        "llvm.ctlz",
        "llvm.ctpop.i32"};
    for (Index i = 0; i < SLANG_COUNT_OF(bodies); ++i)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << bodies[i] << R"(
            [CUDAKernel] void computeMain(
                uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
            { words[1] = bits(words[0], words[0] != 0); }
        )";
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(globalSession, source.getBuffer(), code, diagnostics)));
        SLANG_CHECK(code == nullptr);
        const String diagnosticText = _getBlobText(diagnostics);
        const char* expectedCode = i == 2 ? "E52017" : "E52018";
        if (!diagnosticText.contains(expectedCode) || !diagnosticText.contains(names[i]))
            getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        SLANG_CHECK(diagnosticText.contains(expectedCode));
        SLANG_CHECK(diagnosticText.contains(names[i]));
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangTargetSwitchSelectsExplicitIntrinsicInEitherOrder)
{
    for (bool nvvmFirst : {false, true})
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        const char* nvvmCase = "case nvvm: __intrinsic_asm \"llvm.nvvm.read.ptx.sreg.tid.z\"; ";
        const char* cudaCase = "case cuda: __intrinsic_asm \"unselected CUDA implementation\"; ";
        StringBuilder source;
        source << "uint read() { __target_switch { " << (nvvmFirst ? nvvmCase : cudaCase)
               << (nvvmFirst ? cudaCase : nvvmCase) << "} } [CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output) "
               << "{ output[0] = read(); }";
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const auto result = _compileSlangWithDirectNVVM(
            globalSession,
            source.getBuffer(),
            code,
            diagnostics,
            "cuda_sm_8_0");
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames[0] == "llvm.nvvm.read.ptx.sreg.tid.z");
    }
}

SLANG_UNIT_TEST(nvvmSlangNamedSynchronizationUsesVoidReturn)
{
    _resetDirectNVVMFakes();
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
    globalSession->setSharedLibraryLoader(loader);
    const char* source = R"(
        void sync() { __intrinsic_asm "llvm.nvvm.barrier0"; }
        void deviceFence() { __intrinsic_asm "llvm.nvvm.membar.gl"; }
        void groupFence() { __intrinsic_asm "llvm.nvvm.membar.cta"; }
        [CUDAKernel] void computeMain()
        {
            sync(); deviceFence(); groupFence();
            sync(); deviceFence(); groupFence();
        }
    )";
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    const auto result = _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics);
    if (SLANG_FAILED(result))
        getTestReporter()->message(TestMessageType::Info, _getBlobText(diagnostics).getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    for (const char* name : {"llvm.nvvm.barrier0", "llvm.nvvm.membar.gl", "llvm.nvvm.membar.cta"})
    {
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains(name));
        Index matchingCalls = 0;
        for (Index callee : gFakeNVVMBuilder.callCalleeFunctionIndices)
        {
            const auto intrinsic = gFakeNVVMBuilder.namedIntrinsicFunctionNames.tryGetValue(callee);
            if (intrinsic && *intrinsic == name)
                ++matchingCalls;
        }
        SLANG_CHECK(matchingCalls == 2);
    }
}

SLANG_UNIT_TEST(nvvmSlangNamedSynchronizationRejectsBeforeModuleCreation)
{
    const char* bodies[] = {
        "void sync() { __intrinsic_asm \"llvm.nvvm.barrier0()\"; }",
        "void sync() { __intrinsic_asm \"llvm.nvvm.membar.missing\"; }",
        "void sync() { __intrinsic_asm \"llvm.nvvm.read.ptx.sreg.tid.x\"; }",
        "void sync() { __intrinsic_asm \"llvm.nvvm.membar.cta\", 1; }",
        "void sync(uint x) { __intrinsic_asm \"llvm.nvvm.barrier0\", x; }",
        "uint sync() { __intrinsic_asm \"llvm.nvvm.barrier0\"; }",
    };
    for (Index i = 0; i < SLANG_COUNT_OF(bodies); ++i)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << bodies[i] << "\n[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output) { "
               << (i == 5 ? "output[0] = " : "") << "sync(" << (i == 4 ? "7" : "") << "); }";
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(globalSession, source.getBuffer(), code, diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangDeviceLibraryPreflightsBeforeOutputCreation)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    struct Case
    {
        const char* name;
        bool isDouble;
    };
    const Case cases[] = {
        {"__nv_roundf", false},
        {"__nv_rsqrtf", false},
        {"__nv_rsqrt", true},
        {"__nv_expf", false},
        {"__nv_exp", true},
        {"__nv_exp2f", false},
        {"__nv_exp2", true},
        {"__nv_logf", false},
        {"__nv_log", true},
        {"__nv_log2f", false},
        {"__nv_log2", true},
        {"__nv_log10f", false},
        {"__nv_log10", true},
    };
    for (const auto& testCase : cases)
        for (int variant = 0; variant < 6; ++variant)
        {
            _resetDirectNVVMFakes();
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            TempDirectory toolkit;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_configureFakeDirectNVVMLibdevice(session, toolkit)));
            gFakeNVVMBuilder.rejectDeviceLibraryLoad = variant == 4;
            gFakeNVVMBuilder.rejectDeviceLibraryFunctions = variant == 5;
            gFakeNVVMBuilder.replaceDeviceLibraryPath =
                Path::combine(toolkit.path, "nvvm/libdevice/libdevice.10.bc");
            const bool isDouble = variant == 2 ? !testCase.isDouble : testCase.isDouble;
            const char* type = isDouble ? "double" : "float";
            StringBuilder source;
            source << type << " selected(" << type << " x) { __intrinsic_asm \""
                   << (variant == 1 ? "__nv_missing" : testCase.name) << "\""
                   << (variant == 3 ? "; } " : ", x; } ") << "[CUDAKernel] void computeMain("
                   << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { ";
            if (isDouble)
                source
                    << "uint low, high; asuint(selected(asdouble(words[0], words[1])), low, high); "
                    << "words[2] = low; words[3] = high; }";
            else
                source << "words[1] = asuint(selected(asfloat(words[0]))); }";
            ComPtr<slang::IBlob> code, diagnostics;
            const auto result =
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
            SLANG_CHECK(SLANG_SUCCEEDED(result) == (variant == 0));
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryDestroyCount == (variant == 4 ? 0 : 1));
            if (variant == 0)
            {
                SLANG_CHECK(code != nullptr);
                SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
                SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
                SLANG_CHECK(gFakeNVVM.addedLibraryModule == gFakeNVVMBuilder.deviceLibraryBytes);
                SLANG_CHECK(gFakeNVVM.addedLibraryModule != "replaced-after-query");
                SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains(testCase.name));
            }
            else
            {
                SLANG_CHECK(!code);
                SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
                SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
                if (variant == 4)
                {
                    SLANG_CHECK(
                        _getBlobText(diagnostics).contains("fake corrupt selected bitcode"));
                }
                else
                {
                    SLANG_CHECK(_getBlobText(diagnostics).contains("__nv_"));
                }
            }
        }
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangDeviceLibraryDeadHelpersNeedNoLibrary)
{
    _resetDirectNVVMFakes();
    ComPtr<slang::IGlobalSession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
    session->setSharedLibraryLoader(loader);
    ComPtr<slang::IBlob> code, diagnostics;
    const char* source = R"(
        float unused(float value) { __intrinsic_asm "__nv_roundf", value; }
        float unusedRsqrtFloat(float value) { __intrinsic_asm "__nv_rsqrtf", value; }
        double unusedRsqrtDouble(double value) { __intrinsic_asm "__nv_rsqrt", value; }
        float unusedExpFloat(float value) { __intrinsic_asm "__nv_expf", value; }
        double unusedExpDouble(double value) { __intrinsic_asm "__nv_exp", value; }
        float unusedExp2Float(float value) { __intrinsic_asm "__nv_exp2f", value; }
        double unusedExp2Double(double value) { __intrinsic_asm "__nv_exp2", value; }
        float unusedLogFloat(float value) { __intrinsic_asm "__nv_logf", value; }
        double unusedLogDouble(double value) { __intrinsic_asm "__nv_log", value; }
        float unusedLog2Float(float value) { __intrinsic_asm "__nv_log2f", value; }
        double unusedLog2Double(double value) { __intrinsic_asm "__nv_log2", value; }
        float unusedLog10Float(float value) { __intrinsic_asm "__nv_log10f", value; }
        double unusedLog10Double(double value) { __intrinsic_asm "__nv_log10", value; }
        uint readX() { __intrinsic_asm "llvm.nvvm.read.ptx.sreg.tid.x"; }
        [CUDAKernel] void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
        { words[0] = readX(); }
    )";
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(session, source, code, diagnostics)));
    SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
    SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains("llvm.nvvm.read.ptx.sreg.tid.x"));
}

SLANG_UNIT_TEST(nvvmSlangPublicRoundUsesNamedDeviceLibrary)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    const char* bodies[] = {
        "half value = bit_cast<half>(uint16_t(words[0])); "
        "words[1] = uint(bit_cast<uint16_t>(round(value)));",
        "words[1] = asuint(round(asfloat(words[0])));",
        "double value = round(asdouble(words[0], words[1])); "
        "uint low, high; asuint(value, low, high); words[2] = low; words[3] = high;",
    };
    for (Index variant = 0; variant < SLANG_COUNT_OF(bodies); ++variant)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        TempDirectory toolkit;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_configureFakeDirectNVVMLibdevice(session, toolkit)));
        StringBuilder source;
        source << "[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << bodies[variant] << " }";
        ComPtr<slang::IBlob> code, diagnostics;
        const auto result =
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
        if (SLANG_FAILED(result))
        {
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryDestroyCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains(
            variant == 2 ? "__nv_round" : "__nv_roundf"));
        SLANG_CHECK(!gFakeNVVMBuilder.intrinsicOperations.contains(SlangNVVMValueOperation(64)));
        bool widensHalf = false;
        bool narrowsHalf = false;
        for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
        {
            SLANG_CHECK(operation.key.operation != 64);
            if (operation.key.operation != SLANG_NVVM_VALUE_OP_FLOAT_CONVERT ||
                operation.operandCount != 1)
                continue;
            widensHalf |=
                NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat32) &&
                NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat16);
            narrowsHalf |=
                NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat16) &&
                NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat32);
        }
        // The input/output use bit reinterpretation, so only the public Half recipe needs these.
        SLANG_CHECK(widensHalf == (variant == 0));
        SLANG_CHECK(narrowsHalf == (variant == 0));
    }
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangLegacyRoundAssemblyRejectsBeforeOutputCreation)
{
    const char* types[] = {"half", "float", "double"};
    const char* bodies[] = {
        "words[1] = uint(bit_cast<uint16_t>(oldRound(bit_cast<half>(uint16_t(words[0])))));",
        "words[1] = asuint(oldRound(asfloat(words[0])));",
        "uint low, high; asuint(oldRound(asdouble(words[0], words[1])), low, high); "
        "words[2] = low; words[3] = high;",
    };
    for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << types[variant] << " oldRound(" << types[variant] << " x) { "
               << "__intrinsic_asm \"$P_round($0)\"; } " << "[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << bodies[variant] << " }";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        const String text = _getBlobText(diagnostics);
        if (!text.contains("E52017") || !text.contains("$P_round($0)"))
        {
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK(!code);
        SLANG_CHECK(text.contains("E52017"));
        SLANG_CHECK(text.contains("$P_round($0)"));
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangPublicDirectedRoundingUsesNamedDeviceLibrary)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    const char* bodies[] = {
        "half value = bit_cast<half>(uint16_t(words[0])); "
        "words[1] = uint(bit_cast<uint16_t>(selected(value)));",
        "words[1] = asuint(selected(asfloat(words[0])));",
        "double value = selected(asdouble(words[0], words[1])); "
        "uint low, high; asuint(value, low, high); words[2] = low; words[3] = high;",
    };
    const char* types[] = {"half", "float", "double"};
    for (const char* operationName : {"ceil", "floor", "trunc"})
        for (Index variant = 0; variant < SLANG_COUNT_OF(bodies); ++variant)
        {
            _resetDirectNVVMFakes();
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            TempDirectory toolkit;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_configureFakeDirectNVVMLibdevice(session, toolkit)));
            StringBuilder source;
            source << types[variant] << " selected(" << types[variant] << " x) { return "
                   << operationName << "(x); } " << "[CUDAKernel] void computeMain("
                   << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
                   << bodies[variant] << " }";
            ComPtr<slang::IBlob> code, diagnostics;
            const auto result =
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
            if (SLANG_FAILED(result))
            {
                getTestReporter()->message(
                    TestMessageType::Info,
                    _getBlobText(diagnostics).getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK(code != nullptr);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryDestroyCount == 1);
            StringBuilder expectedName;
            expectedName << "__nv_" << operationName << (variant == 2 ? "" : "f");
            SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains(expectedName.getBuffer()));
            for (const uint32_t identity : {42u, 54u, 57u})
                SLANG_CHECK(!gFakeNVVMBuilder.intrinsicOperations.contains(
                    SlangNVVMValueOperation(identity)));
            bool widensHalf = false;
            bool narrowsHalf = false;
            for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
            {
                SLANG_CHECK(
                    operation.key.operation != 42 && operation.key.operation != 54 &&
                    operation.key.operation != 57);
                if (operation.key.operation != SLANG_NVVM_VALUE_OP_FLOAT_CONVERT ||
                    operation.operandCount != 1)
                    continue;
                widensHalf |=
                    NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat32) &&
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat16);
                narrowsHalf |=
                    NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat16) &&
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat32);
            }
            // The input/output use bit reinterpretation, so only the public Half recipe needs
            // these.
            SLANG_CHECK(widensHalf == (variant == 0));
            SLANG_CHECK(narrowsHalf == (variant == 0));
        }
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangLegacyDirectedRoundingAssemblyRejectsBeforeOutputCreation)
{
    const char* types[] = {"half", "float", "double"};
    const char* bodies[] = {
        "words[1] = "
        "uint(bit_cast<uint16_t>(oldDirectedRound(bit_cast<half>(uint16_t(words[0])))));",
        "words[1] = asuint(oldDirectedRound(asfloat(words[0])));",
        "uint low, high; asuint(oldDirectedRound(asdouble(words[0], words[1])), low, high); "
        "words[2] = low; words[3] = high;",
    };
    for (const char* operationName : {"ceil", "floor", "trunc"})
        for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
        {
            _resetDirectNVVMFakes();
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            StringBuilder spelling;
            spelling << "$P_" << operationName << "($0)";
            StringBuilder source;
            source << types[variant] << " oldDirectedRound(" << types[variant] << " x) { "
                   << "__intrinsic_asm \"" << spelling << "\"; } "
                   << "[CUDAKernel] void computeMain("
                   << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
                   << bodies[variant] << " }";
            ComPtr<slang::IBlob> code, diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
            const String text = _getBlobText(diagnostics);
            if (!text.contains("E52017") || !text.contains(spelling.getBuffer()))
            {
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
            }
            SLANG_CHECK(!code);
            SLANG_CHECK(text.contains("E52017"));
            SLANG_CHECK(text.contains(spelling.getBuffer()));
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
}

SLANG_UNIT_TEST(nvvmSlangPublicSqrtUsesNamedIntrinsic)
{
    const char* types[] = {"half", "float", "double"};
    for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
        for (bool rejectNamed : {false, true})
        {
            _resetDirectNVVMFakes();
            gFakeNVVMBuilder.rejectNamedIntrinsics = rejectNamed;
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            const char* type = types[variant];
            StringBuilder source;
            source << "[noinline] " << type << " sqrtHelper(" << type << " x) { return sqrt(x); } "
                   << "[CUDAKernel] void computeMain("
                   << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { ";
            for (Index i = 0; i < 4; ++i)
            {
                source << type << " x" << i << " = ";
                if (variant == 0)
                    source << "bit_cast<half>(uint16_t(words[" << i << "])); ";
                else if (variant == 1)
                    source << "asfloat(words[" << i << "]); ";
                else
                    source << "asdouble(words[" << 2 * i << "], words[" << 2 * i + 1 << "]); ";
            }
            source << type << " scalar = sqrt(x0); " << type << " helper = sqrtHelper(x1); ";
            for (Index size = 2; size <= 4; ++size)
            {
                source << "vector<" << type << ", " << size << "> v" << size << " = sqrt(vector<"
                       << type << ", " << size << ">(";
                for (Index i = 0; i < size; ++i)
                    source << (i ? ", " : "") << "x" << i;
                source << ")); ";
            }
            source << "matrix<" << type << ", 2, 2> m = sqrt(matrix<" << type
                   << ", 2, 2>(x0, x1, x2, x3)); ";
            const char* values[] = {
                "scalar",
                "helper",
                "v2.x",
                "v2.y",
                "v3.x",
                "v3.y",
                "v3.z",
                "v4.x",
                "v4.y",
                "v4.z",
                "v4.w",
                "m[0][0]",
                "m[0][1]",
                "m[1][0]",
                "m[1][1]"};
            for (Index i = 0; i < SLANG_COUNT_OF(values); ++i)
            {
                if (variant == 2)
                    source << "{ uint low, high; asuint(" << values[i] << ", low, high); words["
                           << 8 + 2 * i << "] = low; words[" << 9 + 2 * i << "] = high; } ";
                else
                    source << "words[" << 4 + i
                           << "] = " << (variant == 0 ? "uint(bit_cast<uint16_t>(" : "asuint(")
                           << values[i] << (variant == 0 ? ")); " : "); ");
            }
            source << "}";
            ComPtr<slang::IBlob> code, diagnostics;
            const auto result =
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
            SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
            if (rejectNamed)
            {
                SLANG_CHECK(SLANG_FAILED(result));
                SLANG_CHECK(!code);
                SLANG_CHECK(_getBlobText(diagnostics).contains("E52018"));
                SLANG_CHECK(_getBlobText(diagnostics).contains("llvm.sqrt"));
                SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
                SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
                continue;
            }
            if (SLANG_FAILED(result))
            {
                StringBuilder detail;
                detail << type << ": " << gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount()
                       << "/" << SLANG_COUNT_OF(gFakeNVVMBuilder.pointerOffsetStorage)
                       << " pointer offsets, " << gFakeNVVMBuilder.localStorageValueTypes.getCount()
                       << "/" << SLANG_COUNT_OF(gFakeNVVMBuilder.localStorage) << " local slots, "
                       << gFakeNVVMBuilder.loadPointerValueRefs.getCount() << "/"
                       << SLANG_COUNT_OF(gFakeNVVMBuilder.loadStorage) << " loads recorded\n"
                       << _getBlobText(diagnostics);
                getTestReporter()->message(TestMessageType::Info, detail.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK(code != nullptr);
            // The kernel's word accesses use pointer offsets; ordinary temporary locals have
            // separate storage handles. Count the observable input/output words, not those locals.
            Index wordLoads = 0;
            Index wordStores = 0;
            for (const auto& pointer : gFakeNVVMBuilder.loadPointerValueRefs)
                wordLoads += pointer.kind == FakeNVVMBuilderValueKind::PointerOffset;
            for (const auto& pointer : gFakeNVVMBuilder.storePointerValueRefs)
                wordStores += pointer.kind == FakeNVVMBuilderValueKind::PointerOffset;
            SLANG_CHECK(wordLoads == (variant == 2 ? 8 : 4));
            SLANG_CHECK(wordStores == (variant == 2 ? 30 : 15));
            SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.getCount() > 0);
            for (const auto& name : gFakeNVVMBuilder.namedIntrinsicNames)
                SLANG_CHECK(name == "llvm.sqrt");
            SLANG_CHECK(_countFakeNVVMNoInlineHelperCalls("sqrtHelper", 1) == 1);
            for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
            {
                SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations[i] == UINT32_MAX);
                SLANG_CHECK(NVVMSemantics::areSameType(
                    gFakeNVVMBuilder.intrinsicResultTypes[i],
                    variant == 2 ? NVVMSemantics::kFloat64 : NVVMSemantics::kFloat32));
                SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[i] == 1);
                const Index offset = gFakeNVVMBuilder.intrinsicArgumentOffsets[i];
                const auto& operand = gFakeNVVMBuilder.intrinsicArgumentValueRefs[offset];
                SLANG_CHECK(operand.kind == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(
                    operand.functionIndex ==
                    gFakeNVVMBuilder
                        .blockFunctionIndices[gFakeNVVMBuilder.intrinsicCallerBlockIndices[i]]);
            }
            bool widensHalf = false;
            bool narrowsHalf = false;
            for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
            {
                SLANG_CHECK(operation.key.operation != 36);
                if (operation.key.operation != SLANG_NVVM_VALUE_OP_FLOAT_CONVERT ||
                    operation.operandCount != 1)
                    continue;
                widensHalf |=
                    NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat32) &&
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat16);
                narrowsHalf |=
                    NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat16) &&
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat32);
            }
            SLANG_CHECK(widensHalf == (variant == 0));
            SLANG_CHECK(narrowsHalf == (variant == 0));
        }
}

SLANG_UNIT_TEST(nvvmSlangLegacySqrtAssemblyRejectsBeforeOutputCreation)
{
    // Fresh tagged source is diagnosed as an unknown tag before NVVM planning. Immutable old
    // modules separately prove retirement of numeric 36; this unit owns legacy untagged text.
    const char* types[] = {"half", "float", "double"};
    const char* bodies[] = {
        "words[1] = uint(bit_cast<uint16_t>(oldSqrt(bit_cast<half>(uint16_t(words[0])))));",
        "words[1] = asuint(oldSqrt(asfloat(words[0])));",
        "uint low, high; asuint(oldSqrt(asdouble(words[0], words[1])), low, high); "
        "words[2] = low; words[3] = high;",
    };
    for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << types[variant] << " oldSqrt(" << types[variant] << " x) { "
               << "__intrinsic_asm \"$P_sqrt($0)\"; } " << "[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << bodies[variant] << " }";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        const String text = _getBlobText(diagnostics);
        if (!text.contains("E52017") || !text.contains("$P_sqrt($0)"))
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        SLANG_CHECK(!code);
        SLANG_CHECK(text.contains("E52017"));
        SLANG_CHECK(text.contains("$P_sqrt($0)"));
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangLegacyFracAssemblyRejectsBeforeOutputCreation)
{
    // Fresh tagged source is diagnosed as an unknown tag before NVVM planning. Immutable old
    // modules separately prove retirement of numeric 59; this unit owns legacy untagged text.
    const char* types[] = {"half", "float", "double"};
    const char* bodies[] = {
        "words[1] = uint(bit_cast<uint16_t>(oldFrac(bit_cast<half>(uint16_t(words[0])))));",
        "words[1] = asuint(oldFrac(asfloat(words[0])));",
        "uint low, high; asuint(oldFrac(asdouble(words[0], words[1])), low, high); "
        "words[2] = low; words[3] = high;",
    };
    for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << types[variant] << " oldFrac(" << types[variant] << " x) { "
               << "__intrinsic_asm \"$P_frac($0)\"; } " << "[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << bodies[variant] << " }";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        const String text = _getBlobText(diagnostics);
        if (!text.contains("E52017") || !text.contains("$P_frac($0)"))
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        SLANG_CHECK(!code);
        SLANG_CHECK(text.contains("E52017"));
        SLANG_CHECK(text.contains("$P_frac($0)"));
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangPublicFracUsesNamedFloorAndSubtract)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    const char* types[] = {"half", "float", "double"};
    const char* bodies[] = {
        "half value = bit_cast<half>(uint16_t(words[0])); "
        "words[1] = uint(bit_cast<uint16_t>(selected(value)));",
        "words[1] = asuint(selected(asfloat(words[0])));",
        "uint low, high; asuint(selected(asdouble(words[0], words[1])), low, high); "
        "words[2] = low; words[3] = high;",
    };
    for (const char* operationName : {"frac", "fract"})
        for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
        {
            _resetDirectNVVMFakes();
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            TempDirectory toolkit;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_configureFakeDirectNVVMLibdevice(session, toolkit)));
            StringBuilder source;
            source << "[noinline] " << types[variant] << " selected(" << types[variant]
                   << " x) { return " << operationName << "(x); } "
                   << "[CUDAKernel] void computeMain("
                   << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
                   << bodies[variant] << " }";
            ComPtr<slang::IBlob> code, diagnostics;
            const auto result =
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
            if (SLANG_FAILED(result))
            {
                getTestReporter()->message(
                    TestMessageType::Info,
                    _getBlobText(diagnostics).getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK(code != nullptr);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryDestroyCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount > 0);
            SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
            SLANG_CHECK(_countFakeNVVMNoInlineHelperCalls("selected", 1) == 1);
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 1);
            SLANG_CHECK(
                gFakeNVVMBuilder.namedIntrinsicNames[0] ==
                (variant == 2 ? "__nv_floor" : "__nv_floorf"));
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.intrinsicOperations.getCount() == 1);
            SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations[0] == UINT32_MAX);
            const auto type = variant == 2 ? NVVMSemantics::kFloat64 : NVVMSemantics::kFloat32;
            SLANG_CHECK(NVVMSemantics::areSameType(gFakeNVVMBuilder.intrinsicResultTypes[0], type));
            SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[0] == 1);

            // Consider selected(x), whose body calls frac(x) or the fract alias. The floor
            // helper receives the same value that is the left operand of subtraction. Follow
            // these direct call operands so swapping x and floor(x) cannot satisfy this test.
            const Index floorFunction =
                gFakeNVVMBuilder
                    .blockFunctionIndices[gFakeNVVMBuilder.intrinsicCallerBlockIndices[0]];
            Index floorCall = -1;
            for (Index i = 0; i < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount(); ++i)
            {
                if (gFakeNVVMBuilder.callCalleeFunctionIndices[i] == floorFunction)
                {
                    SLANG_CHECK(floorCall == -1);
                    floorCall = i;
                }
            }
            SLANG_CHECK_ABORT(floorCall >= 0);
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.callArgumentCounts[floorCall] == 1);
            const auto original =
                gFakeNVVMBuilder
                    .callArgumentValueRefs[gFakeNVVMBuilder.callArgumentOffsets[floorCall]];
            const auto subtraction = _findFakeNVVMScalarBinary(
                SLANG_NVVM_VALUE_OP_SUBTRACT,
                original,
                {FakeNVVMBuilderValueKind::Call, floorCall});
            const auto& subtract = gFakeNVVMBuilder.scalarOperations[subtraction.index];
            SLANG_CHECK(NVVMSemantics::areSameType(subtract.resultType, type));
            SLANG_CHECK(NVVMSemantics::areSameType(subtract.operandTypes[0], type));
            SLANG_CHECK(NVVMSemantics::areSameType(subtract.operandTypes[1], type));
            SLANG_CHECK(
                subtract.callerBlockIndex == gFakeNVVMBuilder.callCallerBlockIndices[floorCall]);
            const Index compositionFunction =
                gFakeNVVMBuilder.blockFunctionIndices[subtract.callerBlockIndex];
            Index widen = -1, narrow = -1;
            for (Index i = 0; i < gFakeNVVMBuilder.scalarOperations.getCount(); ++i)
            {
                const auto& operation = gFakeNVVMBuilder.scalarOperations[i];
                SLANG_CHECK(operation.key.operation != 59);
                if (operation.key.operation != SLANG_NVVM_VALUE_OP_FLOAT_CONVERT)
                    continue;
                SLANG_CHECK_ABORT(operation.operandCount == 1);
                if (NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat32) &&
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat16))
                {
                    SLANG_CHECK(widen == -1);
                    widen = i;
                }
                if (NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat16) &&
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat32))
                {
                    SLANG_CHECK(narrow == -1);
                    narrow = i;
                }
            }
            if (variant == 0)
            {
                SLANG_CHECK_ABORT(widen >= 0 && narrow >= 0);
                // Half narrows the result of the entire Float32 composition, never the floor
                // result before subtraction. Its call argument is the exact widening of x.
                const auto& conversion = gFakeNVVMBuilder.scalarOperations[narrow];
                const auto call = conversion.operands[0];
                SLANG_CHECK_ABORT(call.kind == FakeNVVMBuilderValueKind::Call);
                SLANG_CHECK(
                    gFakeNVVMBuilder.callCalleeFunctionIndices[call.index] == compositionFunction);
                SLANG_CHECK_ABORT(gFakeNVVMBuilder.callArgumentCounts[call.index] == 1);
                const auto argument =
                    gFakeNVVMBuilder
                        .callArgumentValueRefs[gFakeNVVMBuilder.callArgumentOffsets[call.index]];
                SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::ScalarOperation);
                SLANG_CHECK(argument.index == widen);
                SLANG_CHECK(
                    conversion.callerBlockIndex ==
                    gFakeNVVMBuilder.callCallerBlockIndices[call.index]);
            }
            else
            {
                SLANG_CHECK(widen == -1 && narrow == -1);
            }
        }
#else
    SLANG_IGNORE_TEST;
#endif
}

// Checks scalar and aggregate calls against the same selected-library contract.
// Consider exp(halfValue): the core module widens once, calls the Float32 body owning the
// named intrinsic, then narrows once. The value-edge checks below preserve that sequence for
// each public function without duplicating the live scalar/vector/matrix test construction.
static void _checkNVVMPublicUnaryDeviceLibrary(
    const char* publicName,
    const char* floatName,
    const char* doubleName,
    SlangNVVMValueOperation retiredOperation)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    const char* types[] = {"half", "float", "double"};
    for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
        for (bool rejectLibrary : {false, true})
        {
            _resetDirectNVVMFakes();
            gFakeNVVMBuilder.rejectDeviceLibraryFunctions = rejectLibrary;
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            TempDirectory toolkit;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_configureFakeDirectNVVMLibdevice(session, toolkit)));
            const char* type = types[variant];
            StringBuilder source;
            source << "[noinline] " << type << " " << publicName << "Helper(" << type
                   << " x) { return " << publicName << "(x); } " << "[CUDAKernel] void computeMain("
                   << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { ";
            for (Index i = 0; i < 4; ++i)
            {
                source << type << " x" << i << " = ";
                if (variant == 0)
                    source << "bit_cast<half>(uint16_t(words[" << i << "])); ";
                else if (variant == 1)
                    source << "asfloat(words[" << i << "]); ";
                else
                    source << "asdouble(words[" << 2 * i << "], words[" << 2 * i + 1 << "]); ";
            }
            source << type << " scalar = " << publicName << "(x0); " << type
                   << " helper = " << publicName << "Helper(x1); ";
            for (Index size = 2; size <= 4; ++size)
            {
                source << "vector<" << type << ", " << size << "> v" << size << " = " << publicName
                       << "(vector<" << type << ", " << size << ">(";
                for (Index i = 0; i < size; ++i)
                    source << (i ? ", " : "") << "x" << i;
                source << ")); ";
            }
            source << "matrix<" << type << ", 2, 2> m = " << publicName << "(matrix<" << type
                   << ", 2, 2>(x0, x1, x2, x3)); ";
            const char* values[] = {
                "scalar",
                "helper",
                "v2.x",
                "v2.y",
                "v3.x",
                "v3.y",
                "v3.z",
                "v4.x",
                "v4.y",
                "v4.z",
                "v4.w",
                "m[0][0]",
                "m[0][1]",
                "m[1][0]",
                "m[1][1]"};
            for (Index i = 0; i < SLANG_COUNT_OF(values); ++i)
            {
                if (variant == 2)
                    source << "{ uint low, high; asuint(" << values[i] << ", low, high); words["
                           << 8 + 2 * i << "] = low; words[" << 9 + 2 * i << "] = high; } ";
                else
                    source << "words[" << 4 + i
                           << "] = " << (variant == 0 ? "uint(bit_cast<uint16_t>(" : "asuint(")
                           << values[i] << (variant == 0 ? ")); " : "); ");
            }
            source << "}";
            ComPtr<slang::IBlob> code, diagnostics;
            const auto result =
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount > 0);
            if (rejectLibrary)
            {
                SLANG_CHECK(SLANG_FAILED(result));
                SLANG_CHECK(!code);
                SLANG_CHECK(_getBlobText(diagnostics).contains("E52018"));
                SLANG_CHECK(
                    _getBlobText(diagnostics).contains(variant == 2 ? doubleName : floatName));
                SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
                SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
                continue;
            }
            if (SLANG_FAILED(result))
            {
                StringBuilder detail;
                detail << type << ": " << gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount()
                       << "/" << SLANG_COUNT_OF(gFakeNVVMBuilder.pointerOffsetStorage)
                       << " pointer offsets, " << gFakeNVVMBuilder.localStorageValueTypes.getCount()
                       << "/" << SLANG_COUNT_OF(gFakeNVVMBuilder.localStorage) << " local slots, "
                       << gFakeNVVMBuilder.loadPointerValueRefs.getCount() << "/"
                       << SLANG_COUNT_OF(gFakeNVVMBuilder.loadStorage) << " loads recorded\n"
                       << _getBlobText(diagnostics);
                getTestReporter()->message(TestMessageType::Info, detail.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK(code != nullptr);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryDestroyCount == 1);
            SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
            SLANG_CHECK(gFakeNVVM.addedLibraryModule == gFakeNVVMBuilder.deviceLibraryBytes);
            // The kernel's word accesses use pointer offsets; ordinary temporary locals have
            // separate storage handles. Count the observable input/output words, not those locals.
            Index wordLoads = 0;
            Index wordStores = 0;
            for (const auto& pointer : gFakeNVVMBuilder.loadPointerValueRefs)
                wordLoads += pointer.kind == FakeNVVMBuilderValueKind::PointerOffset;
            for (const auto& pointer : gFakeNVVMBuilder.storePointerValueRefs)
                wordStores += pointer.kind == FakeNVVMBuilderValueKind::PointerOffset;
            SLANG_CHECK(wordLoads == (variant == 2 ? 8 : 4));
            SLANG_CHECK(wordStores == (variant == 2 ? 30 : 15));
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 1);
            for (const auto& name : gFakeNVVMBuilder.namedIntrinsicNames)
                SLANG_CHECK(name == (variant == 2 ? doubleName : floatName));
            SLANG_CHECK(
                _countFakeNVVMNoInlineHelperCalls((String(publicName) + "Helper").getBuffer(), 1) ==
                1);
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.intrinsicOperations.getCount() == 1);
            for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
            {
                SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations[i] == UINT32_MAX);
                SLANG_CHECK(NVVMSemantics::areSameType(
                    gFakeNVVMBuilder.intrinsicResultTypes[i],
                    variant == 2 ? NVVMSemantics::kFloat64 : NVVMSemantics::kFloat32));
                SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[i] == 1);
                const Index offset = gFakeNVVMBuilder.intrinsicArgumentOffsets[i];
                const auto& operand = gFakeNVVMBuilder.intrinsicArgumentValueRefs[offset];
                SLANG_CHECK(operand.kind == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(
                    operand.functionIndex ==
                    gFakeNVVMBuilder
                        .blockFunctionIndices[gFakeNVVMBuilder.intrinsicCallerBlockIndices[i]]);
            }
            Index widen = -1, narrow = -1;
            for (Index i = 0; i < gFakeNVVMBuilder.scalarOperations.getCount(); ++i)
            {
                const auto& operation = gFakeNVVMBuilder.scalarOperations[i];
                SLANG_CHECK(operation.key.operation != retiredOperation);
                if (operation.key.operation != SLANG_NVVM_VALUE_OP_FLOAT_CONVERT)
                    continue;
                SLANG_CHECK_ABORT(operation.operandCount == 1);
                if (NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat32) &&
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat16))
                {
                    SLANG_CHECK(widen == -1);
                    widen = i;
                }
                if (NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat16) &&
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat32))
                {
                    SLANG_CHECK(narrow == -1);
                    narrow = i;
                }
            }
            if (variant == 0)
            {
                SLANG_CHECK_ABORT(widen >= 0 && narrow >= 0);
                // Consider exp(halfValue) or rsqrt(halfValue), including an aggregate lane.
                // Its scalar Half body widens the argument, calls the Float32 body that owns
                // the named intrinsic, then narrows that call's result. Check these direct edges;
                // merely finding unrelated conversions would not establish Half evaluation.
                const auto& narrowing = gFakeNVVMBuilder.scalarOperations[narrow];
                const auto call = narrowing.operands[0];
                SLANG_CHECK_ABORT(call.kind == FakeNVVMBuilderValueKind::Call);
                const Index namedFunction =
                    gFakeNVVMBuilder
                        .blockFunctionIndices[gFakeNVVMBuilder.intrinsicCallerBlockIndices[0]];
                SLANG_CHECK(
                    gFakeNVVMBuilder.callCalleeFunctionIndices[call.index] == namedFunction);
                SLANG_CHECK_ABORT(gFakeNVVMBuilder.callArgumentCounts[call.index] == 1);
                const auto argument =
                    gFakeNVVMBuilder
                        .callArgumentValueRefs[gFakeNVVMBuilder.callArgumentOffsets[call.index]];
                SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::ScalarOperation);
                SLANG_CHECK(argument.index == widen);
                SLANG_CHECK(
                    narrowing.callerBlockIndex ==
                    gFakeNVVMBuilder.callCallerBlockIndices[call.index]);
                SLANG_CHECK(
                    gFakeNVVMBuilder.scalarOperations[widen].callerBlockIndex ==
                    narrowing.callerBlockIndex);
            }
            else
            {
                SLANG_CHECK(widen == -1 && narrow == -1);
            }
        }
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmSlangPublicRsqrtUsesNamedDeviceLibrary)
{
    _checkNVVMPublicUnaryDeviceLibrary("rsqrt", "__nv_rsqrtf", "__nv_rsqrt", 65);
}

SLANG_UNIT_TEST(nvvmSlangPublicExpUsesNamedDeviceLibrary)
{
    _checkNVVMPublicUnaryDeviceLibrary("exp", "__nv_expf", "__nv_exp", 55);
}

SLANG_UNIT_TEST(nvvmSlangPublicExp2UsesNamedDeviceLibrary)
{
    _checkNVVMPublicUnaryDeviceLibrary("exp2", "__nv_exp2f", "__nv_exp2", 56);
}

SLANG_UNIT_TEST(nvvmSlangPublicLogUsesNamedDeviceLibrary)
{
    _checkNVVMPublicUnaryDeviceLibrary("log", "__nv_logf", "__nv_log", 60);
}

SLANG_UNIT_TEST(nvvmSlangPublicLog2UsesNamedDeviceLibrary)
{
    _checkNVVMPublicUnaryDeviceLibrary("log2", "__nv_log2f", "__nv_log2", 61);
}

SLANG_UNIT_TEST(nvvmSlangPublicLog10UsesNamedDeviceLibrary)
{
    _checkNVVMPublicUnaryDeviceLibrary("log10", "__nv_log10f", "__nv_log10", 62);
}

SLANG_UNIT_TEST(nvvmSlangLegacyRsqrtAssemblyRejectsBeforeOutputCreation)
{
    // Fresh tagged source is diagnosed as an unknown tag before NVVM planning. Immutable old
    // modules separately prove retirement of numeric 65; this unit owns legacy untagged text.
    const char* types[] = {"half", "float", "double"};
    const char* bodies[] = {
        "words[1] = uint(bit_cast<uint16_t>(oldRsqrt(bit_cast<half>(uint16_t(words[0])))));",
        "words[1] = asuint(oldRsqrt(asfloat(words[0])));",
        "uint low, high; asuint(oldRsqrt(asdouble(words[0], words[1])), low, high); "
        "words[2] = low; words[3] = high;",
    };
    for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << types[variant] << " oldRsqrt(" << types[variant] << " x) { "
               << "__intrinsic_asm \"$P_rsqrt($0)\"; } " << "[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << bodies[variant] << " }";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        const String text = _getBlobText(diagnostics);
        if (!text.contains("E52017") || !text.contains("$P_rsqrt($0)"))
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        SLANG_CHECK(!code);
        SLANG_CHECK(text.contains("E52017"));
        SLANG_CHECK(text.contains("$P_rsqrt($0)"));
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangLegacyExpAssemblyRejectsBeforeOutputCreation)
{
    // Fresh tagged source is diagnosed as an unknown tag before NVVM planning. Immutable old
    // modules separately prove retirement of numeric 55; this unit owns legacy untagged text.
    const char* types[] = {"half", "float", "double"};
    const char* bodies[] = {
        "words[1] = uint(bit_cast<uint16_t>(oldExp(bit_cast<half>(uint16_t(words[0])))));",
        "words[1] = asuint(oldExp(asfloat(words[0])));",
        "uint low, high; asuint(oldExp(asdouble(words[0], words[1])), low, high); "
        "words[2] = low; words[3] = high;",
    };
    for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << types[variant] << " oldExp(" << types[variant] << " x) { "
               << "__intrinsic_asm \"$P_exp($0)\"; } " << "[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << bodies[variant] << " }";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        const String text = _getBlobText(diagnostics);
        if (!text.contains("E52017") || !text.contains("$P_exp($0)"))
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        SLANG_CHECK(!code);
        SLANG_CHECK(text.contains("E52017"));
        SLANG_CHECK(text.contains("$P_exp($0)"));
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangLegacyExp2AssemblyRejectsBeforeOutputCreation)
{
    // The builder unit directly rejects reserved numeric 56 without a module-version gate.
    // This unit independently rejects legacy untagged text before any output module is created.
    const char* types[] = {"half", "float", "double"};
    const char* bodies[] = {
        "words[1] = uint(bit_cast<uint16_t>(oldExp2(bit_cast<half>(uint16_t(words[0])))));",
        "words[1] = asuint(oldExp2(asfloat(words[0])));",
        "uint low, high; asuint(oldExp2(asdouble(words[0], words[1])), low, high); "
        "words[2] = low; words[3] = high;",
    };
    for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << types[variant] << " oldExp2(" << types[variant] << " x) { "
               << "__intrinsic_asm \"$P_exp2($0)\"; } " << "[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << bodies[variant] << " }";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        const String text = _getBlobText(diagnostics);
        if (!text.contains("E52017") || !text.contains("$P_exp2($0)"))
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        SLANG_CHECK(!code);
        SLANG_CHECK(text.contains("E52017"));
        SLANG_CHECK(text.contains("$P_exp2($0)"));
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

// Reject legacy logarithm text during planning, before library or output-module creation.
static void _checkNVVMLegacyLogAssembly(const char* operation)
{
    // Builder units reject each reserved numeric ID independently of the module-version gate.
    // This unit independently rejects legacy untagged text before any output module is created.
    StringBuilder legacyAssembly;
    legacyAssembly << "$P_" << operation << "($0)";
    const char* types[] = {"half", "float", "double"};
    const char* bodies[] = {
        "words[1] = uint(bit_cast<uint16_t>(oldLog(bit_cast<half>(uint16_t(words[0])))));",
        "words[1] = asuint(oldLog(asfloat(words[0])));",
        "uint low, high; asuint(oldLog(asdouble(words[0], words[1])), low, high); "
        "words[2] = low; words[3] = high;",
    };
    for (Index variant = 0; variant < SLANG_COUNT_OF(types); ++variant)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << types[variant] << " oldLog(" << types[variant] << " x) { " << "__intrinsic_asm \""
               << legacyAssembly << "\"; } " << "[CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << bodies[variant] << " }";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        const String text = _getBlobText(diagnostics);
        if (!text.contains("E52017") || !text.contains(legacyAssembly.getUnownedSlice()))
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        SLANG_CHECK(!code);
        SLANG_CHECK(text.contains("E52017"));
        SLANG_CHECK(text.contains(legacyAssembly.getUnownedSlice()));
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangLegacyLogAssemblyRejectsBeforeOutputCreation)
{
    _checkNVVMLegacyLogAssembly("log");
}

SLANG_UNIT_TEST(nvvmSlangLegacyLog2AssemblyRejectsBeforeOutputCreation)
{
    _checkNVVMLegacyLogAssembly("log2");
}

SLANG_UNIT_TEST(nvvmSlangLegacyLog10AssemblyRejectsBeforeOutputCreation)
{
    _checkNVVMLegacyLogAssembly("log10");
}

SLANG_UNIT_TEST(nvvmSlangCoreMathUsesNamedCalls)
{
    for (const auto& testCase : kNVVMCoreMathTestCases)
        for (uint32_t width : {16u, 32u, 64u})
        {
            _resetDirectNVVMFakes();
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            TempDirectory toolkit;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_configureFakeDirectNVVMLibdevice(session, toolkit)));
            const char* type = width == 16 ? "half" : width == 32 ? "float" : "double";
            StringBuilder arguments, parameters, vectorArguments;
            for (uint32_t i = 0; i < testCase.operandCount; ++i)
            {
                arguments << (i ? ", " : "") << "x" << i;
                parameters << (i ? ", " : "") << type << " x" << i;
                vectorArguments << (i ? ", " : "") << "vector<" << type << ",2>(x" << i << ", x"
                                << ((i + 1) % 3) << ")";
            }
            StringBuilder source;
            source << "[noinline] " << type << " mathHelper(" << parameters << ") { return "
                   << testCase.name << "(" << arguments << "); } "
                   << "[CUDAKernel] void computeMain("
                   << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { ";
            for (uint32_t i = 0; i < 3; ++i)
            {
                source << type << " x" << i << " = ";
                if (width == 16)
                    source << "bit_cast<half>(uint16_t(words[" << i << "])); ";
                else if (width == 32)
                    source << "asfloat(words[" << i << "]); ";
                else
                    source << "asdouble(words[" << 2 * i << "], words[" << 2 * i + 1 << "]); ";
            }
            source << type << " scalar = mathHelper(" << arguments << "); vector<" << type
                   << ",2> aggregate = " << testCase.name << "(" << vectorArguments << "); ";
            const char* values[] = {"scalar", "aggregate.x", "aggregate.y"};
            for (uint32_t i = 0; i < SLANG_COUNT_OF(values); ++i)
            {
                if (width == 64)
                    source << "{ uint low, high; asuint(" << values[i] << ", low, high); words["
                           << 6 + 2 * i << "] = low; words[" << 7 + 2 * i << "] = high; } ";
                else if (width == 16)
                    source << "words[" << 3 + i << "] = uint(bit_cast<uint16_t>(" << values[i]
                           << ")); ";
                else
                    source << "words[" << 3 + i << "] = asuint(" << values[i] << "); ";
            }
            source << "}";
            ComPtr<slang::IBlob> code, diagnostics;
            const auto result =
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics);
            if (SLANG_FAILED(result))
                getTestReporter()->message(
                    TestMessageType::Info,
                    _getBlobText(diagnostics).getBuffer());
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK(code != nullptr);
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.namedIntrinsicNames.getCount() == 1);
            SLANG_CHECK(
                gFakeNVVMBuilder.namedIntrinsicNames[0] ==
                (width == 64 ? testCase.doubleName : testCase.floatName));
            SLANG_CHECK_ABORT(gFakeNVVMBuilder.intrinsicOperations.getCount() == 1);
            SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations[0] == UINT32_MAX);
            SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[0] == testCase.operandCount);
            const Index namedFunction =
                gFakeNVVMBuilder
                    .blockFunctionIndices[gFakeNVVMBuilder.intrinsicCallerBlockIndices[0]];
            const Index argumentOffset = gFakeNVVMBuilder.intrinsicArgumentOffsets[0];
            for (uint32_t i = 0; i < testCase.operandCount; ++i)
            {
                const auto& argument =
                    gFakeNVVMBuilder.intrinsicArgumentValueRefs[argumentOffset + i];
                SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(argument.index == i && argument.functionIndex == namedFunction);
            }
            List<Index> narrowedCalls;
            for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
            {
                // Public fmod also uses a named call; numeric58 is reserved for canonical FRem.
                SLANG_CHECK(operation.key.operation != testCase.operation);
                if (operation.key.operation != SLANG_NVVM_VALUE_OP_FLOAT_CONVERT ||
                    !NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat16))
                    continue;
                SLANG_CHECK(
                    NVVMSemantics::areSameType(operation.operandTypes[0], NVVMSemantics::kFloat32));
                const auto call = operation.operands[0];
                SLANG_CHECK_ABORT(call.kind == FakeNVVMBuilderValueKind::Call);
                SLANG_CHECK(!narrowedCalls.contains(call.index));
                narrowedCalls.add(call.index);
                SLANG_CHECK(
                    gFakeNVVMBuilder.callCallerBlockIndices[call.index] ==
                    operation.callerBlockIndex);
                SLANG_CHECK(
                    gFakeNVVMBuilder.callCalleeFunctionIndices[call.index] == namedFunction);
                SLANG_CHECK_ABORT(
                    gFakeNVVMBuilder.callArgumentCounts[call.index] == testCase.operandCount);
                for (uint32_t i = 0; i < testCase.operandCount; ++i)
                {
                    const auto argument =
                        gFakeNVVMBuilder.callArgumentValueRefs
                            [gFakeNVVMBuilder.callArgumentOffsets[call.index] + i];
                    SLANG_CHECK_ABORT(argument.kind == FakeNVVMBuilderValueKind::ScalarOperation);
                    const auto& widening = gFakeNVVMBuilder.scalarOperations[argument.index];
                    SLANG_CHECK(widening.key.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT);
                    SLANG_CHECK(
                        NVVMSemantics::areSameType(widening.resultType, NVVMSemantics::kFloat32));
                    SLANG_CHECK(NVVMSemantics::areSameType(
                        widening.operandTypes[0],
                        NVVMSemantics::kFloat16));
                    SLANG_CHECK(widening.callerBlockIndex == operation.callerBlockIndex);
                    // Half ABI lowering may decode a physical parameter; forced-inline fmod
                    // may instead widen a vector lane. The direct call/cast edges are invariant.
                }
            }
            if (width == 16)
            {
                // Require exactly one narrowing for every emitted call, including each inlined
                // fmod composition. Counting just one body would reject its valid scalar map.
                SLANG_CHECK(narrowedCalls.getCount() > 0);
                for (Index call = 0; call < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount();
                     ++call)
                    if (gFakeNVVMBuilder.callCalleeFunctionIndices[call] == namedFunction)
                        SLANG_CHECK(narrowedCalls.contains(call));
            }
            else
                SLANG_CHECK(narrowedCalls.getCount() == 0);
        }
}

SLANG_UNIT_TEST(nvvmSlangCoreMathLegacyRoutesRejectBeforeOutput)
{
    for (const auto& testCase : kNVVMCoreMathTestCases)
        for (bool tagged : {false, true})
        {
            _resetDirectNVVMFakes();
            ComPtr<slang::IGlobalSession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            session->setSharedLibraryLoader(loader);
            StringBuilder source;
            source << "float legacy(";
            for (uint32_t i = 0; i < testCase.operandCount; ++i)
                source << (i ? ", " : "") << "float x" << i;
            source << ") { __intrinsic_asm";
            if (tagged)
            {
                const char first = testCase.name[0] - 'a' + 'A';
                source << "(nvvm" << first << (testCase.name + 1) << ")";
            }
            source << " \"$P_" << testCase.name << "(";
            for (uint32_t i = 0; i < testCase.operandCount; ++i)
                source << (i ? ", " : "") << "$" << i;
            source << ")\"; } [CUDAKernel] void computeMain("
                   << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
                   << "words[3] = asuint(legacy(";
            for (uint32_t i = 0; i < testCase.operandCount; ++i)
                source << (i ? ", " : "") << "asfloat(words[" << i << "])";
            source << ")); }";
            ComPtr<slang::IBlob> code, diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
            SLANG_CHECK(!code);
            if (!tagged)
                SLANG_CHECK(_getBlobText(diagnostics).contains("E52017"));
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
    _resetDirectNVVMFakes();
    ComPtr<slang::IGlobalSession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
    session->setSharedLibraryLoader(loader);
    const char* source = R"slang(
        void legacy(float x, out float s, out float c)
        { __intrinsic_asm "$P_sincos($0, $1, $2)"; }
        [CUDAKernel] void computeMain(
            uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
        { float s, c; legacy(asfloat(words[0]), s, c); words[1]=asuint(s); words[2]=asuint(c); }
    )slang";
    ComPtr<slang::IBlob> code, diagnostics;
    SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(session, source, code, diagnostics)));
    SLANG_CHECK(!code);
    SLANG_CHECK(_getBlobText(diagnostics).contains("E52017"));
    SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangCoreBitsLegacyTextRejectsBeforeOutput)
{
    struct LegacyCase
    {
        const char* declaration;
        const char* use;
    };
    const LegacyCase cases[] = {
        {R"slang(half legacy(int16_t x) { __intrinsic_asm "__short_as_half"; })slang",
         "words[1] = uint(asuint16(legacy(int16_t(words[0]))));"},
        {R"slang(half legacy(uint16_t x) { __intrinsic_asm "__ushort_as_half"; })slang",
         "words[1] = uint(asuint16(legacy(uint16_t(words[0]))));"},
        {R"slang(uint16_t legacy(half x) { __intrinsic_asm "__half_as_ushort"; })slang",
         "words[1] = uint(legacy(asfloat16(uint16_t(words[0]))));"},
        {R"slang(float legacy(uint x) { __intrinsic_asm "__half2float(__ushort_as_half($0))"; })slang",
         "words[1] = asuint(legacy(words[0]));"},
        {R"slang(uint legacy(float x) { __intrinsic_asm "__half_as_ushort(__float2half($0))"; })slang",
         "words[1] = legacy(asfloat(words[0]));"},
        {R"slang(double legacy(uint x, uint y) { __intrinsic_asm "$P_asdouble($0, $1)"; })slang",
         "uint lo, hi; asuint(legacy(words[0], words[1]), lo, hi); words[2]=lo; words[3]=hi;"},
        {R"slang(void legacy(double x, out uint lo, out uint hi) { __intrinsic_asm "$P_asuint($0, $1, $2)"; })slang",
         "uint lo, hi; legacy(asdouble(words[0], words[1]), lo, hi); words[2]=lo; words[3]=hi;"},
    };
    List<String> sources;
    for (const auto& testCase : cases)
    {
        StringBuilder source;
        source << testCase.declaration << " [CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << testCase.use << " }";
        sources.add(source.produceString());
    }
    for (const char* name : {"isfinite", "isinf", "isnan"})
        for (const char* type : {"half", "float", "double"})
        {
            StringBuilder source;
            source << "bool legacy(" << type << " x) { __intrinsic_asm \"$P_" << name
                   << "($0)\"; } [CUDAKernel] void computeMain("
                   << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
                   << "words[1] = uint(legacy(" << type << "(asfloat(words[0])))); }";
            sources.add(source.produceString());
        }
    for (const auto& source : sources)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        SLANG_CHECK(!code);
        SLANG_CHECK(_getBlobText(diagnostics).contains("E52017"));
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryQueryCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangCoreValuesLegacyRoutesRejectBeforeOutput)
{
    struct LegacyCase
    {
        const char* declaration;
        const char* use;
    };
    const LegacyCase cases[] = {
        {R"slang(float old(float x) { __intrinsic_asm "$P_abs($0)"; })slang",
         "words[1]=asuint(old(asfloat(words[0])));"},
        {R"slang(int old(float x) { __intrinsic_asm "$P_sign($0)"; })slang",
         "words[1]=uint(old(asfloat(words[0])));"},
        {R"slang(float old(float x,float y) { __intrinsic_asm "$P_min($0, $1)"; })slang",
         "words[1]=asuint(old(asfloat(words[0]),asfloat(words[1])));"},
        {R"slang(float old(float x,float y) { __intrinsic_asm "$P_max($0, $1)"; })slang",
         "words[1]=asuint(old(asfloat(words[0]),asfloat(words[1])));"},
        {R"slang(uint old() { __intrinsic_asm "clock"; })slang", "words[0]=old();"},
        {R"slang(int64_t old() { __intrinsic_asm "clock64"; })slang", "words[0]=uint(old());"},
        {R"slang(float old(float x) { __intrinsic_asm(nvvmAbs) "$P_abs($0)"; })slang",
         "words[1]=asuint(old(asfloat(words[0])));"},
        {R"slang(int old(float x) { __intrinsic_asm(nvvmSign) "$P_sign($0)"; })slang",
         "words[1]=uint(old(asfloat(words[0])));"},
        {R"slang(float old(float x,float y) { __intrinsic_asm(nvvmMin) "$P_min($0, $1)"; })slang",
         "words[1]=asuint(old(asfloat(words[0]),asfloat(words[1])));"},
        {R"slang(float old(float x,float y) { __intrinsic_asm(nvvmMax) "$P_max($0, $1)"; })slang",
         "words[1]=asuint(old(asfloat(words[0]),asfloat(words[1])));"},
    };
    List<String> sources;
    sources.add(R"slang(
        void old(RWByteAddressBuffer b, uint offset, float value, out float previous)
        { __intrinsic_asm "(*$3 = atomicAdd($0._getPtrAt<float>($1), $2))"; }
        RWByteAddressBuffer buffer;
        [CUDAKernel] void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
        { float previous; old(buffer, words[0], asfloat(words[1]), previous); words[2]=asuint(previous); }
    )slang");
    sources.add(R"slang(
        void old(RWByteAddressBuffer b, uint offset, uint64_t expected, uint64_t desired, out uint64_t previous)
        { __intrinsic_asm "(*$4 = atomicCAS($0._getPtrAt<uint64_t>($1), $2, $3))"; }
        RWByteAddressBuffer buffer;
        [CUDAKernel] void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
        { uint64_t previous; old(buffer, words[0], uint64_t(words[1]), uint64_t(words[2]), previous); words[3]=uint(previous); }
    )slang");
    for (const auto& testCase : cases)
    {
        StringBuilder source;
        source << testCase.declaration << " [CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
               << testCase.use << " }";
        sources.add(source.produceString());
    }
    for (const char* operation :
         {"Add", "Subtract", "Min", "Max", "BitAnd", "BitOr", "BitXor", "Increment", "Decrement"})
    {
        StringBuilder source;
        source << "void old(inout uint x, uint y) { __intrinsic_asm(nvvmAtomicReduce" << operation
               << ") \"legacy atomic reduction\"; } [CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { "
                  "old(words[0], words[1]); }";
        sources.add(source.produceString());
    }
    for (const auto& source : sources)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        SLANG_CHECK(!code);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangAtomicReductionProducersRejectInvalidInputs)
{
    for (const char* call :
         {"__atomic_reduce_inc(*words, MemoryOrder::Acquire);",
          "__atomic_reduce_dec(*words, MemoryOrder::Acquire);",
          "__atomic_reduce_inc(*words);",
          "__atomic_reduce_dec(*words);"})
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        const bool nonRelaxed = String(call).contains("Acquire");
        StringBuilder source;
        source << "[CUDAKernel] void computeMain(uniform Ptr<" << (nonRelaxed ? "uint" : "float")
               << ", Access::ReadWrite, AddressSpace::Device> words) { " << call << " }";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        SLANG_CHECK(!code);
        const String text = _getBlobText(diagnostics);
        SLANG_CHECK(text.contains(
            nonRelaxed ? (String(call).contains("inc") ? "atomicInc" : "atomicDec")
                       : "requires an integer type"));
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangLocalAtomicReductionRejectsAtProducer)
{
    _resetDirectNVVMFakes();
    ComPtr<slang::IGlobalSession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
    session->setSharedLibraryLoader(loader);
    ComPtr<slang::IBlob> code, diagnostics;
    SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
        session,
        kDirectNVVMUnsupportedLocalAtomicReductionSource,
        code,
        diagnostics)));
    SLANG_CHECK(!code);
    SLANG_CHECK(_getBlobText(diagnostics).contains("E41403"));
    SLANG_CHECK(_getBlobText(diagnostics).contains("invalid atomic destination"));
    SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}

// Public wave APIs now expose named primitives and ordinary composition instead of numeric recipes.
SLANG_UNIT_TEST(nvvmSlangCoreWaveProducersUseNamedCalls)
{
    struct SourceCase
    {
        const char* source;
        const char* primitive;
    };
    const SourceCase cases[] = {
        {kDirectNVVMWaveReadLaneAtUIntSource, "llvm.nvvm.shfl.sync.idx.i32"},
        {kDirectNVVMWaveReadLaneAtIntSource, "llvm.nvvm.shfl.sync.idx.i32"},
        {kDirectNVVMWaveReadLaneAtFloatSource, "llvm.nvvm.shfl.sync.idx.f32"},
        {kDirectNVVMUnmaskedWaveReadLaneAtUIntSource, "llvm.nvvm.shfl.sync.idx.i32"},
        {kDirectNVVMUnmaskedWaveReadLaneAtIntSource, "llvm.nvvm.shfl.sync.idx.i32"},
        {kDirectNVVMUnmaskedWaveReadLaneAtFloatSource, "llvm.nvvm.shfl.sync.idx.f32"},
        {kDirectNVVMWaveReadLaneFirstUIntSource, "llvm.nvvm.shfl.sync.idx.i32"},
        {kDirectNVVMWaveReadLaneFirstIntSource, "llvm.nvvm.shfl.sync.idx.i32"},
        {kDirectNVVMWaveReadLaneFirstFloatSource, "llvm.nvvm.shfl.sync.idx.f32"},
        {kDirectNVVMWaveIsFirstLaneSource, "llvm.nvvm.read.ptx.sreg.laneid"},
        {kDirectNVVMWaveActiveAnyTrueSource, "llvm.nvvm.vote.any.sync"},
        {kDirectNVVMWaveActiveAllTrueSource, "llvm.nvvm.vote.all.sync"},
        {kDirectNVVMWaveActiveAllEqualIntSource, "llvm.nvvm.match.any.sync.i32"},
        {kDirectNVVMWaveActiveAllEqualUIntSource, "llvm.nvvm.match.any.sync.i32"},
        {kDirectNVVMWaveActiveAllEqualFloatSource, "llvm.nvvm.match.any.sync.i32"},
    };
    for (const auto& testCase : cases)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code, diagnostics;
        const auto result =
            _compileSlangWithDirectNVVM(session, testCase.source, code, diagnostics);
        if (SLANG_FAILED(result))
            getTestReporter()->message(
                TestMessageType::Info,
                _getBlobText(diagnostics).getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK(code);
        SLANG_CHECK(gFakeNVVMBuilder.namedIntrinsicNames.contains(testCase.primitive));
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            const auto operation = gFakeNVVMBuilder.intrinsicOperations[i];
            SLANG_CHECK(operation != 16 && (operation < 19 || operation > 23));
            if (gFakeNVVMBuilder.intrinsicNames[i] != "llvm.nvvm.shfl.sync.idx.i32" &&
                gFakeNVVMBuilder.intrinsicNames[i] != "llvm.nvvm.shfl.sync.idx.f32")
                continue;
            SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[i] == 4);
            const auto offset = gFakeNVVMBuilder.intrinsicArgumentOffsets[i];
            for (Index operand = 0; operand < 3; ++operand)
            {
                const auto ref = gFakeNVVMBuilder.intrinsicArgumentValueRefs[offset + operand];
                SLANG_CHECK(ref.kind == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(ref.index == operand);
            }
            const auto clamp = gFakeNVVMBuilder.intrinsicArgumentValueRefs[offset + 3];
            SLANG_CHECK_ABORT(clamp.kind == FakeNVVMBuilderValueKind::IntegerConstant);
            SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[clamp.index] == 31);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    }
}

// The hardware snapshot stays an effectful canonical instruction, distinct from logical synthesis.
SLANG_UNIT_TEST(nvvmSlangHardwareAndLogicalWaveMasksStayDistinct)
{
    _resetDirectNVVMFakes();
    ComPtr<slang::IGlobalSession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
    session->setSharedLibraryLoader(loader);
    ComPtr<slang::IBlob> code, diagnostics;
    const char* source = R"slang(
        [CUDAKernel] void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
        {
            words[0] = __WaveGetConvergedMask();
            words[1] = __WaveGetConvergedMask();
            words[2] = WaveGetActiveMask();
        }
    )slang";
    const auto result = _compileSlangWithDirectNVVM(session, source, code, diagnostics);
    if (SLANG_FAILED(result))
        getTestReporter()->message(TestMessageType::Info, _getBlobText(diagnostics).getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    uint32_t hardware = 0, logical = 0;
    for (auto operation : gFakeNVVMBuilder.intrinsicOperations)
    {
        hardware += operation == SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK;
        logical += operation == SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT;
    }
    SLANG_CHECK(hardware == 2);
    SLANG_CHECK(logical >= 1);
    SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 3);
}

SLANG_UNIT_TEST(nvvmSlangCoreWaveLegacyRoutesRejectBeforeOutput)
{
    struct LegacyCase
    {
        const char* declaration;
        const char* use;
        const char* diagnostic = "E52017";
    };
    const LegacyCase cases[] = {
        {R"slang(uint old() { __intrinsic_asm "__activemask()"; })slang", "old()"},
        {R"slang(uint4 old() { __intrinsic_asm "make_uint4(__activemask(), 0, 0, 0)"; })slang",
         "old().x"},
        {R"slang(uint old() { __intrinsic_asm(nvvmWaveLaneIndex) "_getLaneId()"; })slang",
         "old()",
         "E36121"},
        {R"slang(uint old() { __intrinsic_asm(nvvmWaveLaneCount) "(warpSize)"; })slang",
         "old()",
         "E36121"},
        {R"slang(uint old(uint m,uint v,int l) { __intrinsic_asm(nvvmWaveReadLaneAt) "__shfl_sync($0, $1, $2)"; })slang",
         "old(words[0],words[1],int(words[2]))",
         "E36121"},
        {R"slang(uint old(uint m,uint v) { __intrinsic_asm(nvvmWaveReadLaneFirst) "_waveReadFirst($0, $1)"; })slang",
         "old(words[0],words[1])",
         "E36121"},
        {R"slang(bool old(uint m) { __intrinsic_asm(nvvmWaveMaskIsFirstLane) "(($0 & -$0) == (WarpMask(1) << _getLaneId()))"; })slang",
         "uint(old(words[0]))",
         "E36121"},
        {R"slang(bool old(uint m,bool v) { __intrinsic_asm(nvvmWaveMaskAnyTrue) "(__any_sync($0, $1) != 0)"; })slang",
         "uint(old(words[0],words[1]!=0))",
         "E36121"},
        {R"slang(bool old(uint m,bool v) { __intrinsic_asm(nvvmWaveMaskAllTrue) "(__all_sync($0, $1) != 0)"; })slang",
         "uint(old(words[0],words[1]!=0))",
         "E36121"},
        {R"slang(bool old(uint m,uint v) { __intrinsic_asm(nvvmWaveMaskAllEqual) "_waveAllEqual($0, $1)"; })slang",
         "uint(old(words[0],words[1]))",
         "E36121"},
        {R"slang(uint old(uint m,bool v) { __intrinsic_asm(nvvmWaveMaskBallot) "__ballot_sync($0, $1)"; })slang",
         "old(words[0],words[1]!=0)",
         "E36121"},
        {R"slang(uint old(uint m,bool v) { __intrinsic_asm "__ballot_sync($0, $1)"; })slang",
         "old(words[0],words[1]!=0)"},
        {R"slang(uint old(uint m,bool v) { __intrinsic_asm "__popc(__ballot_sync($0, $1))"; })slang",
         "old(words[0],words[1]!=0)"},
        {R"slang(uint2 old(uint m,uint2 v,int l) { __intrinsic_asm "_waveShuffleMultiple($0, $1, $2)"; })slang",
         "old(words[0],uint2(words[1],words[2]),int(words[3])).x"},
        {R"slang(bool old(uint m,uint2 v) { __intrinsic_asm "_waveAllEqualMultiple($0, $1)"; })slang",
         "uint(old(words[0],uint2(words[1],words[2])))"},
    };
    for (const auto& testCase : cases)
    {
        StringBuilder source;
        source << testCase.declaration << " [CUDAKernel] void computeMain("
               << "uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words) { words[4]="
               << testCase.use << "; }";
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        SLANG_CHECK(!code);
        SLANG_CHECK(_getBlobText(diagnostics).contains(testCase.diagnostic));
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

// Exact legacy bodies must reject before provider creation, independently of module version.
SLANG_UNIT_TEST(nvvmSlangMaskedWaveLegacyTextRejectsBeforeOutput)
{
    struct LegacyWaveCase
    {
        const char* scalar;
        const char* aggregate;
    };
    const LegacyWaveCase cases[] = {
        {"_waveSum($1.x, $0)", "_waveSumMultiple($1.x, $0)"},
        {"_waveProduct($1.x, $0)", "_waveProductMultiple($1.x, $0)"},
        {"_waveMin($1.x, $0)", "_waveMinMultiple($1.x, $0)"},
        {"_waveMax($1.x, $0)", "_waveMaxMultiple($1.x, $0)"},
        {"_waveAnd($1.x, $0)", "_waveAndMultiple($1.x, $0)"},
        {"_waveOr($1.x, $0)", "_waveOrMultiple($1.x, $0)"},
        {"_waveXor($1.x, $0)", "_waveXorMultiple($1.x, $0)"},
        {"_wavePrefixSum($1.x, $0) ", "_wavePrefixSumMultiple($1.x, $0) "},
        {"_wavePrefixProduct($1.x, $0) ", "_wavePrefixProductMultiple($1.x, $0) "},
        {"_wavePrefixAnd($1.x, $0) ", "_wavePrefixAndMultiple($1.x, $0) "},
        {"_wavePrefixOr($1.x, $0) ", "_wavePrefixOrMultiple($1.x, $0) "},
        {"_wavePrefixXor($1.x, $0) ", "_wavePrefixXorMultiple($1.x, $0) "},
        {"_wavePrefixSum($1.x, $0) + $0", "_wavePrefixSumMultiple($1.x, $0) + $0"},
        {"_wavePrefixProduct($1.x, $0) * $0", "_wavePrefixProductMultiple($1.x, $0) * $0"},
        {"_wavePrefixAnd($1.x, $0) & $0", "_wavePrefixAndMultiple($1.x, $0) & $0"},
        {"_wavePrefixOr($1.x, $0) | $0", "_wavePrefixOrMultiple($1.x, $0) | $0"},
        {"_wavePrefixXor($1.x, $0) ^ $0", "_wavePrefixXorMultiple($1.x, $0) ^ $0"},
        {"_wavePrefixExclusiveMin(($1).x, $0)", "_wavePrefixExclusiveMinMultiple(($1).x, $0)"},
        {"_wavePrefixExclusiveMax(($1).x, $0)", "_wavePrefixExclusiveMaxMultiple(($1).x, $0)"},
        {"_wavePrefixInclusiveMin(($1).x, $0)", "_wavePrefixInclusiveMinMultiple(($1).x, $0)"},
        {"_wavePrefixInclusiveMax(($1).x, $0)", "_wavePrefixInclusiveMaxMultiple(($1).x, $0)"},
    };
    List<String> sources;
    for (const auto& testCase : cases)
    {
        for (bool aggregate : {false, true})
        {
            StringBuilder source;
            source << (aggregate ? "int2" : "int") << " old(" << (aggregate ? "int2" : "int")
                   << " value,uint4 mask) { __intrinsic_asm \""
                   << (aggregate ? testCase.aggregate : testCase.scalar)
                   << "\"; } [CUDAKernel] void computeMain(uniform "
                      "Ptr<int,Access::ReadWrite,AddressSpace::Device> outp) { outp[1]="
                   << (aggregate ? "old(int2(outp[0]),uint4(15,0,0,0)).x"
                                 : "old(outp[0],uint4(15,0,0,0))")
                   << "; }";
            sources.add(source.produceString());
        }
    }
    for (const char* text : {"_slang_quadAny", "_slang_quadAll", "bool($0)"})
    {
        StringBuilder source;
        source << "bool old(bool value) { __intrinsic_asm \"" << text
               << "\"; } [CUDAKernel] void computeMain(uniform "
                  "Ptr<int,Access::ReadWrite,AddressSpace::Device> outp) { "
                  "outp[1]=int(old(outp[0]!=0)); }";
        sources.add(source.produceString());
    }
    for (const auto& source : sources)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        SLANG_CHECK(!code);
        SLANG_CHECK(_getBlobText(diagnostics).contains("E52017"));
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.deviceLibraryLoadCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangMaskedWaveScalarAdmissionStaysBounded)
{
    const char* expressions[] = {
        "WaveMultiSum(int64_t(outp[0]),mask)",
        "WaveMultiProduct(half(outp[0]),mask)",
        "WaveMultiBitAnd(uint64_t(outp[0]),mask)",
    };
    for (const char* expression : expressions)
    {
        StringBuilder source;
        source << "[CUDAKernel] void computeMain(uniform "
                  "Ptr<int,Access::ReadWrite,AddressSpace::Device> outp) { uint4 "
                  "mask=uint4(15,0,0,0); outp[1]=int("
               << expression << "); }";
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(session, source.getBuffer(), code, diagnostics)));
        SLANG_CHECK(!code);
        SLANG_CHECK(
            _getBlobText(diagnostics).contains("Unsupported NVVM partition operation scalar type"));
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangCoreTailLegacyRoutesRejectBeforeOutput)
{
    struct Case
    {
        const char* declaration;
        const char* invocation;
    };
    const Case cases[] = {
        {R"SLANG(float legacy(float x, out int e) { __intrinsic_asm "$P_frexp($0, $1)"; })SLANG",
         "int e; output[0] = legacy(asfloat(input[0]), e); output[1] = e;"},
        {R"SLANG(double legacy(double x, out int e) { __intrinsic_asm "$P_frexp($0, $1)"; })SLANG",
         "int e; output[0] = float(legacy(double(input[0]), e)); output[1] = e;"},
        {R"SLANG(half legacy(half x, out int e) { __intrinsic_asm "$P_frexp($0, $1)"; })SLANG",
         "int e; output[0] = float(legacy(half(input[0]), e)); output[1] = e;"},
        {R"SLANG(half legacy(half x, out half e) { __intrinsic_asm "$P_modf($0, $1)"; })SLANG",
         "half e; output[0] = float(legacy(half(input[0]), e)); output[1] = float(e);"},
        {R"SLANG(BFloat16 legacy(vector<BFloat16,2> x, vector<BFloat16,2> y) { __intrinsic_asm "_slang_vector_dot"; })SLANG",
         "let b = bit_cast<BFloat16>(uint16_t(input[0])); output[0] = "
         "float(legacy(vector<BFloat16,2>(b,b), vector<BFloat16,2>(b,b)));"},
        {R"SLANG(int legacy() { __intrinsic_asm "sizeof($[0])", float3; })SLANG",
         "output[0] = legacy();"},
        {R"SLANG(int legacy(float3 x) { __intrinsic_asm "sizeof($T0)"; })SLANG",
         "output[0] = legacy(float3(input[0]));"},
        {R"SLANG(int legacy() { __intrinsic_asm "alignof($[0])", float3; })SLANG",
         "output[0] = legacy();"},
        {R"SLANG(int legacy(float3 x) { __intrinsic_asm "alignof($T0)"; })SLANG",
         "output[0] = legacy(float3(input[0]));"},
    };
    for (const auto& test : cases)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << test.declaration
               << "\nStructuredBuffer<uint> input; RWStructuredBuffer<float> output; "
               << "[numthreads(1,1,1)] void computeMain() {" << test.invocation << "}";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            session,
            source.getBuffer(),
            code,
            diagnostics,
            "cuda_sm_8_0")));
        const String text = _getBlobText(diagnostics);
        if (!text.contains("E52017"))
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        SLANG_CHECK(text.contains("E52017"));
        SLANG_CHECK(text.contains("GenericAsm assembly="));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangCoreLayoutQueriesRejectInvalidSize)
{
    struct Case
    {
        const char* declaration;
        const char* diagnostic;
    };
    const Case cases[] = {
        {"struct Empty {}; typedef Empty Sized;", "E41400"},
        {"typedef int Sized[536870912];", "E41402"},
    };
    for (const auto& test : cases)
    {
        _resetDirectNVVMFakes();
        ComPtr<slang::IGlobalSession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        session->setSharedLibraryLoader(loader);
        StringBuilder source;
        source << test.declaration << "\nRWStructuredBuffer<int> output; "
               << "[numthreads(1,1,1)] void computeMain() { output[0] = __sizeOf<Sized>(); }";
        ComPtr<slang::IBlob> code, diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            session,
            source.getBuffer(),
            code,
            diagnostics,
            "cuda_sm_8_0")));
        const String text = _getBlobText(diagnostics);
        if (!text.contains(test.diagnostic))
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        SLANG_CHECK(text.contains(test.diagnostic));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangBFloat16CoreDotComposition)
{
    _resetDirectNVVMFakes();
    ComPtr<slang::IGlobalSession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef())));
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
    session->setSharedLibraryLoader(loader);
    const char* source = R"SLANG(
        StructuredBuffer<uint> input; RWStructuredBuffer<uint> output;
        [numthreads(1,1,1)] void computeMain() {
            let x = bit_cast<BFloat16>(uint16_t(input[0]));
            let y = bit_cast<BFloat16>(uint16_t(input[1]));
            output[0] = uint(bit_cast<uint16_t>(dot(vector<BFloat16,1>(x), vector<BFloat16,1>(y))));
            output[1] = uint(bit_cast<uint16_t>(dot(vector<BFloat16,2>(x,y), vector<BFloat16,2>(y,x))));
        })SLANG";
    ComPtr<slang::IBlob> code, diagnostics;
    auto result = _compileSlangWithDirectNVVM(session, source, code, diagnostics, "cuda_sm_8_0");
    if (SLANG_FAILED(result))
        getTestReporter()->message(TestMessageType::Info, _getBlobText(diagnostics).getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    uint32_t fmas = 0, floatProducts = 0;
    for (const auto& operation : gFakeNVVMBuilder.scalarOperations)
    {
        if (operation.key.operation == SLANG_NVVM_VALUE_OP_FMA)
        {
            ++fmas;
            SLANG_CHECK(operation.operandCount == 3);
            SLANG_CHECK(NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kBFloat16));
        }
        if (operation.key.operation == SLANG_NVVM_VALUE_OP_MULTIPLY &&
            NVVMSemantics::areSameType(operation.resultType, NVVMSemantics::kFloat32))
            ++floatProducts;
        SLANG_CHECK(operation.key.operation != 82);
    }
    SLANG_CHECK(fmas == 4);
    SLANG_CHECK(floatProducts == 1);
}
