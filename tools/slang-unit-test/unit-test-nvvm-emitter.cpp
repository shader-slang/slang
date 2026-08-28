// unit-test-nvvm-emitter.cpp

#include "unit-test-nvvm-support.h"

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
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.addIntegerPhiIncomingCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::IntegerPhi);
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
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.addIntegerPhiIncomingCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::Float);
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
        SLANG_CHECK(
            gFakeNVVMBuilder.callResultTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::Float);
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
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations[0] == SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX);
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
        SLANG_CHECK(
            gFakeNVVMBuilder.callResultTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::Integer);
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

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 6);
        SLANG_CHECK(gFakeNVVMBuilder.getVectorTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementType == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.executionRegisterOperations.getCount() == 4);
        const SlangNVVMValueOperation expectedOperations[] = {
            SLANG_NVVM_VALUE_OP_THREAD_INDEX,
            SLANG_NVVM_VALUE_OP_BLOCK_INDEX,
            SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS,
            SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS,
        };
        for (Index i = 0; i < SLANG_COUNT_OF(expectedOperations); ++i)
        {
            SLANG_CHECK(gFakeNVVMBuilder.executionRegisterOperations[i] == expectedOperations[i]);
            SLANG_CHECK(
                gFakeNVVMBuilder.scalarReturnValueRefs[i].kind ==
                FakeNVVMBuilderValueKind::ExecutionRegister);
            SLANG_CHECK(gFakeNVVMBuilder.scalarReturnValueRefs[i].index == i);
        }
        SLANG_CHECK(gFakeNVVMBuilder.workgroupBarrierCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.callResultKinds.getCount() == 5);
        for (Index i = 0; i < 4; ++i)
            SLANG_CHECK(
                gFakeNVVMBuilder.callResultKinds[i] == FakeNVVMBuilderResultTypeKind::UInt3);
        SLANG_CHECK(gFakeNVVMBuilder.callResultKinds[4] == FakeNVVMBuilderResultTypeKind::Void);

        SLANG_CHECK(gFakeNVVMBuilder.emitVectorElementExtractCallCount == 12);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementBaseValueRefs.getCount() == 12);
        for (Index i = 0; i < 12; ++i)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.vectorElementBaseValueRefs[i].kind ==
                FakeNVVMBuilderValueKind::Call);
            SLANG_CHECK(gFakeNVVMBuilder.vectorElementBaseValueRefs[i].index == i / 3);
            SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[i] == uint32_t(i % 3));
            SLANG_CHECK(
                gFakeNVVMBuilder.storeValueRefs[i].kind == FakeNVVMBuilderValueKind::VectorElement);
            SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[i].index == i);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 12);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 2);
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
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 6);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emittedValueOperations.getCount() == 0);

        const int64_t expectedValues[] = {1, 4, 4, 16, 8, 32};
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
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageLinkage == SLANG_NVVM_GLOBAL_LINKAGE_EXTERNAL);
        SLANG_CHECK(
            gFakeNVVMBuilder.globalStorageAddressSpace == SLANG_NVVM_ADDRESS_SPACE_CONSTANT);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageAlignment == 8);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageNames.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageNames[0] == "SLANG_globalParams");

        SLANG_CHECK(gFakeNVVMBuilder.executionRegisterOperations.getCount() == 3);
        SLANG_CHECK(
            gFakeNVVMBuilder.executionRegisterOperations[0] == SLANG_NVVM_VALUE_OP_BLOCK_INDEX);
        SLANG_CHECK(
            gFakeNVVMBuilder.executionRegisterOperations[1] ==
            SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS);
        SLANG_CHECK(
            gFakeNVVMBuilder.executionRegisterOperations[2] == SLANG_NVVM_VALUE_OP_THREAD_INDEX);
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
        SLANG_CHECK(gFakeNVVMBuilder.emitVectorElementExtractCallCount == 1);

        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldPointerBaseValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::GlobalStorage);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldPointerIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignment == 8);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::ResourceView);
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldValueCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldValueIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
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
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldValueCallCount == 1);
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
        SLANG_CHECK(gFakeNVVMBuilder.emitVectorElementExtractCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices.getCount() == 5);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[0] == 2);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[1] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[2] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[3] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[4] == 0);

        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldPointerCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldValueCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldValueTypeKinds.getCount() == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldValueTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::Float);
        SLANG_CHECK(
            gFakeNVVMBuilder.structFieldValueTypeKinds[1] == FakeNVVMBuilderScalarTypeKind::Float);
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

        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 2);
        for (Index i = 0; i < 2; ++i)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.arrayElementPointerBaseValueRefs[i].kind ==
                FakeNVVMBuilderValueKind::GlobalStorage);
            SLANG_CHECK(gFakeNVVMBuilder.arrayElementPointerBaseValueRefs[i].index == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitRelaxedGlobalI32AtomicAddCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.workgroupBarrierCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 1);
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
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations[0] == SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations[1] == SLANG_NVVM_VALUE_OP_WAVE_LANE_COUNT);
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

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneAtUIntUsesDirectPipeline)
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
            kDirectNVVMWaveReadLaneAtUIntSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations.getCount() == 2);
        Index shuffleIntrinsicIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT)
            {
                shuffleIntrinsicIndex = i;
            }
        }
        SLANG_CHECK_ABORT(shuffleIntrinsicIndex >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[shuffleIntrinsicIndex] == 3);
        const Index shuffleArgumentOffset =
            gFakeNVVMBuilder.intrinsicArgumentOffsets[shuffleIntrinsicIndex];
        const Index shuffleBlockIndex =
            gFakeNVVMBuilder.intrinsicCallerBlockIndices[shuffleIntrinsicIndex];
        const Index shuffleFunctionIndex = gFakeNVVMBuilder.blockFunctionIndices[shuffleBlockIndex];
        for (Index i = 0; i < 3; ++i)
        {
            const FakeNVVMBuilderValueRef& argument =
                gFakeNVVMBuilder.intrinsicArgumentValueRefs[shuffleArgumentOffset + i];
            SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(argument.functionIndex == shuffleFunctionIndex);
            SLANG_CHECK(argument.index == i);
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 2);
        Index shuffleCallIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.callArgumentCounts.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.callArgumentCounts[i] == 3)
                shuffleCallIndex = i;
        }
        SLANG_CHECK_ABORT(shuffleCallIndex >= 0);
        const Index callArgumentOffset = gFakeNVVMBuilder.callArgumentOffsets[shuffleCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset + 0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset + 1].kind ==
            FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset + 2].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneAtIntUsesDirectPipeline)
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
            kDirectNVVMWaveReadLaneAtIntSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations.getCount() == 2);
        Index shuffleIntrinsicIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT)
            {
                shuffleIntrinsicIndex = i;
            }
        }
        SLANG_CHECK_ABORT(shuffleIntrinsicIndex >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[shuffleIntrinsicIndex] == 3);
        const Index shuffleArgumentOffset =
            gFakeNVVMBuilder.intrinsicArgumentOffsets[shuffleIntrinsicIndex];
        const Index shuffleBlockIndex =
            gFakeNVVMBuilder.intrinsicCallerBlockIndices[shuffleIntrinsicIndex];
        const Index shuffleFunctionIndex = gFakeNVVMBuilder.blockFunctionIndices[shuffleBlockIndex];
        for (Index i = 0; i < 3; ++i)
        {
            const FakeNVVMBuilderValueRef& argument =
                gFakeNVVMBuilder.intrinsicArgumentValueRefs[shuffleArgumentOffset + i];
            SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(argument.functionIndex == shuffleFunctionIndex);
            SLANG_CHECK(argument.index == i);
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 2);
        Index shuffleCallIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.callArgumentCounts.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.callArgumentCounts[i] == 3)
                shuffleCallIndex = i;
        }
        SLANG_CHECK_ABORT(shuffleCallIndex >= 0);
        const Index callArgumentOffset = gFakeNVVMBuilder.callArgumentOffsets[shuffleCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset + 0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset + 1].kind ==
            FakeNVVMBuilderValueKind::Load);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset + 2].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneAtFloatUsesDirectPipeline)
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
            kDirectNVVMWaveReadLaneAtFloatSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations.getCount() == 2);
        Index shuffleIntrinsicIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT)
            {
                shuffleIntrinsicIndex = i;
            }
        }
        SLANG_CHECK_ABORT(shuffleIntrinsicIndex >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[shuffleIntrinsicIndex] == 3);
        const Index shuffleArgumentOffset =
            gFakeNVVMBuilder.intrinsicArgumentOffsets[shuffleIntrinsicIndex];
        const Index shuffleBlockIndex =
            gFakeNVVMBuilder.intrinsicCallerBlockIndices[shuffleIntrinsicIndex];
        const Index shuffleFunctionIndex = gFakeNVVMBuilder.blockFunctionIndices[shuffleBlockIndex];
        for (Index i = 0; i < 3; ++i)
        {
            const FakeNVVMBuilderValueRef& argument =
                gFakeNVVMBuilder.intrinsicArgumentValueRefs[shuffleArgumentOffset + i];
            SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(argument.functionIndex == shuffleFunctionIndex);
            SLANG_CHECK(argument.index == i);
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 2);
        Index shuffleCallIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.callArgumentCounts.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.callArgumentCounts[i] == 3)
                shuffleCallIndex = i;
        }
        SLANG_CHECK_ABORT(shuffleCallIndex >= 0);
        const Index callArgumentOffset = gFakeNVVMBuilder.callArgumentOffsets[shuffleCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset + 0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset + 1].kind ==
            FakeNVVMBuilderValueKind::Load);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[callArgumentOffset + 2].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Call);
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
static void _checkPublicWaveReadDirectPipeline(
    const char* source,
    SlangNVVMValueOperation waveOperation,
    Index waveArgumentCount,
    FakeNVVMBuilderValueKind entryValueKind,
    Index expectedPointerOffsetCount,
    Index expectedLoadCount)
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
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(globalSession, source, code, diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicOperations.getCount() == 3);
        Index ballotIntrinsicIndex = -1;
        Index waveIntrinsicIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT)
                ballotIntrinsicIndex = i;
            else if (gFakeNVVMBuilder.intrinsicOperations[i] == waveOperation)
                waveIntrinsicIndex = i;
        }
        SLANG_CHECK_ABORT(ballotIntrinsicIndex >= 0);
        SLANG_CHECK_ABORT(waveIntrinsicIndex >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[ballotIntrinsicIndex] == 2);
        SLANG_CHECK(
            gFakeNVVMBuilder.intrinsicArgumentCounts[waveIntrinsicIndex] == waveArgumentCount);

        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 4);
        Index publicCallIndex = -1;
        Index activeMaskCallIndex = -1;
        Index maskedWaveCallIndex = -1;
        for (Index callIndex = 0; callIndex < gFakeNVVMBuilder.callArgumentCounts.getCount();
             ++callIndex)
        {
            const Index argumentCount = gFakeNVVMBuilder.callArgumentCounts[callIndex];
            const Index argumentOffset = gFakeNVVMBuilder.callArgumentOffsets[callIndex];
            if (argumentCount == 1)
            {
                activeMaskCallIndex = callIndex;
            }
            else if (argumentCount == waveArgumentCount)
            {
                const FakeNVVMBuilderValueRef& lastArgument =
                    gFakeNVVMBuilder.callArgumentValueRefs[argumentOffset + waveArgumentCount - 1];
                if (lastArgument.kind == FakeNVVMBuilderValueKind::Intrinsic)
                    publicCallIndex = callIndex;
                else
                    maskedWaveCallIndex = callIndex;
            }
        }
        SLANG_CHECK_ABORT(publicCallIndex >= 0);
        SLANG_CHECK_ABORT(activeMaskCallIndex >= 0);
        SLANG_CHECK_ABORT(maskedWaveCallIndex >= 0);

        const Index publicArgumentOffset = gFakeNVVMBuilder.callArgumentOffsets[publicCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[publicArgumentOffset + 0].kind ==
            entryValueKind);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[publicArgumentOffset + waveArgumentCount - 1]
                .kind == FakeNVVMBuilderValueKind::Intrinsic);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[publicArgumentOffset + waveArgumentCount - 1]
                .index == ballotIntrinsicIndex);
        if (waveArgumentCount == 3)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.callArgumentValueRefs[publicArgumentOffset + 1].kind ==
                FakeNVVMBuilderValueKind::Parameter);
        }

        const Index activeMaskArgumentOffset =
            gFakeNVVMBuilder.callArgumentOffsets[activeMaskCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[activeMaskArgumentOffset].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        const Index maskedWaveArgumentOffset =
            gFakeNVVMBuilder.callArgumentOffsets[maskedWaveCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[maskedWaveArgumentOffset + 0].kind ==
            FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[maskedWaveArgumentOffset + 0].index ==
            activeMaskCallIndex);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[maskedWaveArgumentOffset + 1].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        if (waveArgumentCount == 3)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.callArgumentValueRefs[maskedWaveArgumentOffset + 2].kind ==
                FakeNVVMBuilderValueKind::Parameter);
        }
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == expectedPointerOffsetCount);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == expectedLoadCount);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == publicCallIndex);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangUnmaskedWaveReadLaneAtUIntUsesDirectPipeline)
{
    _checkPublicWaveReadDirectPipeline(
        kDirectNVVMUnmaskedWaveReadLaneAtUIntSource,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        3,
        FakeNVVMBuilderValueKind::Call,
        1,
        0);
}

SLANG_UNIT_TEST(nvvmSlangUnmaskedWaveReadLaneAtIntUsesDirectPipeline)
{
    _checkPublicWaveReadDirectPipeline(
        kDirectNVVMUnmaskedWaveReadLaneAtIntSource,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        3,
        FakeNVVMBuilderValueKind::Load,
        2,
        1);
}

SLANG_UNIT_TEST(nvvmSlangUnmaskedWaveReadLaneAtFloatUsesDirectPipeline)
{
    _checkPublicWaveReadDirectPipeline(
        kDirectNVVMUnmaskedWaveReadLaneAtFloatSource,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        3,
        FakeNVVMBuilderValueKind::Load,
        2,
        1);
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneFirstUIntUsesDirectPipeline)
{
    _checkPublicWaveReadDirectPipeline(
        kDirectNVVMWaveReadLaneFirstUIntSource,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        2,
        FakeNVVMBuilderValueKind::Call,
        1,
        0);
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneFirstIntUsesDirectPipeline)
{
    _checkPublicWaveReadDirectPipeline(
        kDirectNVVMWaveReadLaneFirstIntSource,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        2,
        FakeNVVMBuilderValueKind::Load,
        2,
        1);
}

SLANG_UNIT_TEST(nvvmSlangWaveReadLaneFirstFloatUsesDirectPipeline)
{
    _checkPublicWaveReadDirectPipeline(
        kDirectNVVMWaveReadLaneFirstFloatSource,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        2,
        FakeNVVMBuilderValueKind::Load,
        2,
        1);
}

SLANG_UNIT_TEST(nvvmSlangWaveIsFirstLaneUsesDirectPipeline)
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
            kDirectNVVMWaveIsFirstLaneSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 4);
        Index isFirstIntrinsicIndex = -1;
        Index ballotCount = 0;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.intrinsicOperations[i] ==
                SLANG_NVVM_VALUE_OP_WAVE_MASK_IS_FIRST_LANE)
            {
                isFirstIntrinsicIndex = i;
            }
            else if (
                gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT)
            {
                ++ballotCount;
            }
        }
        SLANG_CHECK_ABORT(isFirstIntrinsicIndex >= 0);
        SLANG_CHECK(ballotCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[isFirstIntrinsicIndex] == 1);
        const Index argumentOffset =
            gFakeNVVMBuilder.intrinsicArgumentOffsets[isFirstIntrinsicIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.intrinsicArgumentValueRefs[argumentOffset].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 4);
        Index booleanCallCount = 0;
        for (FakeNVVMBuilderScalarTypeKind resultKind : gFakeNVVMBuilder.callResultTypeKinds)
        {
            if (resultKind == FakeNVVMBuilderScalarTypeKind::Boolean)
                ++booleanCallCount;
        }
        SLANG_CHECK(booleanCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::IntegerPhi);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

static void _checkPublicWavePredicateDirectPipeline(
    const char* source,
    SlangNVVMValueOperation operation,
    Index expectedBooleanParameterCount,
    Index expectedFloatParameterCount = 0)
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
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(globalSession, source, code, diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 4);
        Index predicateIntrinsicIndex = -1;
        Index ballotCount = 0;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.intrinsicOperations[i] == operation)
            {
                predicateIntrinsicIndex = i;
            }
            else if (
                gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT)
            {
                ++ballotCount;
            }
        }
        SLANG_CHECK_ABORT(predicateIntrinsicIndex >= 0);
        SLANG_CHECK(ballotCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[predicateIntrinsicIndex] == 2);
        const Index argumentOffset =
            gFakeNVVMBuilder.intrinsicArgumentOffsets[predicateIntrinsicIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.intrinsicArgumentValueRefs[argumentOffset].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(
            gFakeNVVMBuilder.intrinsicArgumentValueRefs[argumentOffset + 1].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        Index booleanParameterCount = 0;
        Index floatParameterCount = 0;
        for (FakeNVVMBuilderParameterTypeKind parameterKind :
             gFakeNVVMBuilder.functionParameterTypeKinds)
        {
            if (parameterKind == FakeNVVMBuilderParameterTypeKind::Boolean)
                ++booleanParameterCount;
            else if (parameterKind == FakeNVVMBuilderParameterTypeKind::Float)
                ++floatParameterCount;
        }
        SLANG_CHECK(booleanParameterCount == expectedBooleanParameterCount);
        SLANG_CHECK(floatParameterCount == expectedFloatParameterCount);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 4);
        Index booleanCallCount = 0;
        for (FakeNVVMBuilderScalarTypeKind resultKind : gFakeNVVMBuilder.callResultTypeKinds)
        {
            if (resultKind == FakeNVVMBuilderScalarTypeKind::Boolean)
                ++booleanCallCount;
        }
        SLANG_CHECK(booleanCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::IntegerPhi);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAnyTrueUsesDirectPipeline)
{
    _checkPublicWavePredicateDirectPipeline(
        kDirectNVVMWaveActiveAnyTrueSource,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ANY_TRUE,
        2);
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAllTrueUsesDirectPipeline)
{
    _checkPublicWavePredicateDirectPipeline(
        kDirectNVVMWaveActiveAllTrueSource,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_TRUE,
        2);
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAllEqualIntUsesDirectPipeline)
{
    _checkPublicWavePredicateDirectPipeline(
        kDirectNVVMWaveActiveAllEqualIntSource,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        0);
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAllEqualUIntUsesDirectPipeline)
{
    _checkPublicWavePredicateDirectPipeline(
        kDirectNVVMWaveActiveAllEqualUIntSource,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        0);
}

SLANG_UNIT_TEST(nvvmSlangWaveActiveAllEqualFloatUsesDirectPipeline)
{
    _checkPublicWavePredicateDirectPipeline(
        kDirectNVVMWaveActiveAllEqualFloatSource,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        0,
        2);
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
            SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == expected.phiCount);
            SLANG_CHECK(gFakeNVVMBuilder.addIntegerPhiIncomingCallCount == expected.incomingCount);
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
                const Index mergeBlock = gFakeNVVMBuilder.integerPhiTargetBlockIndices[0];
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
                SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount() == 2);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::IntegerPhi);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
                SLANG_CHECK(gFakeNVVMBuilder.storeBlockIndices[0] == mergeBlock);

                Index xPredecessor = -1;
                Index yPredecessor = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount(); ++i)
                {
                    SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPhiIndices[i] == 0);
                    const FakeNVVMBuilderValueRef valueRef =
                        gFakeNVVMBuilder.integerPhiIncomingValueRefs[i];
                    SLANG_CHECK(valueRef.kind == FakeNVVMBuilderValueKind::Parameter);
                    if (valueRef.index == 1)
                    {
                        xPredecessor =
                            gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[i];
                    }
                    else if (valueRef.index == 2)
                    {
                        yPredecessor =
                            gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[i];
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
                SLANG_CHECK(gFakeNVVMBuilder.integerPhiTargetBlockIndices.getCount() == 2);
                const Index headerBlock = gFakeNVVMBuilder.integerPhiTargetBlockIndices[0];
                SLANG_CHECK(headerBlock != 0);
                SLANG_CHECK(gFakeNVVMBuilder.integerPhiTargetBlockIndices[1] == headerBlock);
                SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount() == 4);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::IntegerPhi);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 1);
                const Index compareIndex = _findFakeNVVMBuilderScalarOperation(
                    FakeNVVMBuilderScalarFamily::Compare,
                    SLANG_NVVM_VALUE_OP_LESS_THAN);
                SLANG_CHECK_ABORT(compareIndex >= 0);
                const FakeNVVMBuilderScalarOperation& comparison =
                    gFakeNVVMBuilder.scalarOperations[compareIndex];
                SLANG_CHECK(comparison.operands[0].kind == FakeNVVMBuilderValueKind::IntegerPhi);
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
                        left.kind == FakeNVVMBuilderValueKind::IntegerPhi && left.index == 0;
                    const bool rightIsI =
                        right.kind == FakeNVVMBuilderValueKind::IntegerPhi && right.index == 0;
                    const bool leftIsSum =
                        left.kind == FakeNVVMBuilderValueKind::IntegerPhi && left.index == 1;
                    const bool rightIsSum =
                        right.kind == FakeNVVMBuilderValueKind::IntegerPhi && right.index == 1;
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
                for (Index i = 0; i < gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount(); ++i)
                {
                    const FakeNVVMBuilderValueRef valueRef =
                        gFakeNVVMBuilder.integerPhiIncomingValueRefs[i];
                    if (gFakeNVVMBuilder.integerPhiIncomingPhiIndices[i] == 0 &&
                        valueRef.kind == FakeNVVMBuilderValueKind::IntegerConstant &&
                        valueRef.index == zeroIndex)
                    {
                        entryBlock = gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[i];
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
                for (Index i = 0; i < gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount(); ++i)
                {
                    const FakeNVVMBuilderValueRef valueRef =
                        gFakeNVVMBuilder.integerPhiIncomingValueRefs[i];
                    if (gFakeNVVMBuilder.integerPhiIncomingPhiIndices[i] == 0 &&
                        valueRef.kind == FakeNVVMBuilderValueKind::ScalarOperation &&
                        valueRef.index == nextIIndex)
                    {
                        continueBlock =
                            gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[i];
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
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
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
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
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

        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementPointerBaseValueRefs.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementPointerIndexValueRefs.getCount() == 2);
        for (Index elementIndex = 0; elementIndex < 2; ++elementIndex)
        {
            const FakeNVVMBuilderValueRef base =
                gFakeNVVMBuilder.arrayElementPointerBaseValueRefs[elementIndex];
            const FakeNVVMBuilderValueRef index =
                gFakeNVVMBuilder.arrayElementPointerIndexValueRefs[elementIndex];
            SLANG_CHECK(base.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(base.functionIndex == 0);
            SLANG_CHECK(base.index == elementIndex);
            SLANG_CHECK(index.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(index.functionIndex == 0);
            SLANG_CHECK(index.index == 2);
            SLANG_CHECK(gFakeNVVMBuilder.arrayElementPointerCallerBlockIndices[elementIndex] == 0);
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs[0].index == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
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
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
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

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 2);
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
            FakeNVVMBuilderParameterTypeKind::IntegerResourceView);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldValueCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldValueBaseValueRefs.getCount() == 1);
        const FakeNVVMBuilderValueRef buffer = gFakeNVVMBuilder.structFieldValueBaseValueRefs[0];
        SLANG_CHECK(buffer.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(buffer.functionIndex == 0);
        SLANG_CHECK(buffer.index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.structFieldValueIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::StructFieldValue);
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

        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 0);
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
            (isCompare ? FakeNVVMBuilderValueKind::IntegerPhi
                       : FakeNVVMBuilderValueKind::ScalarOperation));
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == (isCompare ? 1 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == (isCompare ? 2 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == (isCompare ? 1 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.addIntegerPhiIncomingCallCount == (isCompare ? 2 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == (isCompare ? 2 : 0));
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 0);
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
    };
    static const DirectCase kCases[] = {
        {kDirectNVVMRelaxedGlobalI32AtomicAddSource, 1, false},
        {kDirectNVVMRelaxedGlobalI32AtomicAddOldValueSource, 2, true},
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
            SLANG_CHECK(gFakeNVVMBuilder.emitRelaxedGlobalI32AtomicAddCallCount == 1);
            SLANG_CHECK(
                gFakeNVVMBuilder.relaxedGlobalI32AtomicAddCallerBlockIndices.getCount() == 1);
            SLANG_CHECK(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddCallerBlockIndices[0] == 0);
            SLANG_CHECK(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddPointerValueRefs.getCount() == 1);
            const FakeNVVMBuilderValueRef pointer =
                gFakeNVVMBuilder.relaxedGlobalI32AtomicAddPointerValueRefs[0];
            SLANG_CHECK(pointer.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(pointer.functionIndex == 0);
            SLANG_CHECK(pointer.index == 0);
            SLANG_CHECK(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddValueRefs.getCount() == 1);
            const FakeNVVMBuilderValueRef value =
                gFakeNVVMBuilder.relaxedGlobalI32AtomicAddValueRefs[0];
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
                    FakeNVVMBuilderValueKind::RelaxedGlobalI32AtomicAdd);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
                SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
            }

            SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 0);
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

NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerEqualUsesDirectPipeline, Equal)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerNotEqualUsesDirectPipeline, NotEqual)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerSignedGreaterThanUsesDirectPipeline, SignedGreaterThan)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerSignedLessEqualUsesDirectPipeline, SignedLessEqual)
NVVM_SCALAR_DIRECT_TEST(nvvmSlangIntegerSignedGreaterEqualUsesDirectPipeline, SignedGreaterEqual)

#undef NVVM_SCALAR_DIRECT_TEST
SLANG_UNIT_TEST(nvvmSlangRejectsAdjacentStructuredBufferShapesBeforeProviderMutation)
{
    static const char* kUnsupportedSources[] = {
        kDirectNVVMRawRWStructuredBufferF64StoreSource,
        kDirectNVVMRawStructuredBufferI32LoadSource,
        kDirectNVVMRawRWStructuredBufferI32AtomicAddSource,
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
            SLANG_CHECK(gFakeNVVMBuilder.emitStructFieldValueCallCount == 0);
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

SLANG_UNIT_TEST(nvvmSlangRejectsConventionalRawKernelParameters)
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
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMConventionalParameterizedComputeSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52017") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
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
            kDirectNVVMWaveLaneIndexSource,
            {
                SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX,
                NVVMSemantics::kUnsignedI32,
                nullptr,
                0,
            },
            "wave lane index intrinsic",
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

SLANG_UNIT_TEST(nvvmSlangUnsupportedIRStopsBeforeEmission)
{
    struct UnsupportedCase
    {
        const char* source;
        const char* expectedConstruct;
    };
    static const UnsupportedCase kCases[] = {
        {kDirectNVVMUnsupportedCallSource, "'CUDA kernel decoration'"},
        {kDirectNVVMUnsupportedPointerHelperParameterSource, "'helper function parameter'"},
        {kDirectNVVMUnsupportedPointerHelperResultSource, "'helper function result type'"},
        {kDirectNVVMUnsignedPointerOffsetSource, "'integer_constant'"},
        {kDirectNVVMUnsignedFixedArrayIndexSource, "'signed i32 value'"},
        {kDirectNVVMUnsupportedFloatArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedHalfAddSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedDoubleAddSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedNestedArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedLocalArraySource, "'var'"},
        {kDirectNVVMUnsupportedSharedFloatArraySource, "'device i32 array element pointer'"},
        {kDirectNVVMUnsupportedStructPointerSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedArrayPointerHelperSource, "'helper function parameter'"},
        {kDirectNVVMNonCanonicalCUDAOffsetSource, "'CUDA layout query'"},
        {kDirectNVVMUnsupportedFixedSamplerArrayStorageSource,
         "'conventional global parameter field address'"},
        {kDirectNVVMUnsupportedNestedParameterBlockSource,
         "'conventional global parameter field address'"},
        {kDirectNVVMUnsupportedNestedConstantBufferSource,
         "'conventional global parameter field address'"},
        {kDirectNVVMFloatingSineSource, "'GenericAsm'"},
        {kDirectNVVMIntegerLeftShiftSource, "'shl'"},
        {kDirectNVVMIntegerRightShiftSource, "'shr'"},
        {kDirectNVVMIntegerDivideSource, "'div'"},
        {kDirectNVVMIntegerRemainderSource, "'irem'"},
        {kDirectNVVMLogicalNotSource, "'entry-point parameter'"},
        {kDirectNVVMUnsignedAtomicAddSource, "'relaxed global signed i32 atomic add'"},
        {kDirectNVVMWideAtomicAddSource, "'relaxed global signed i32 atomic add'"},
        {kDirectNVVMFloatingAtomicAddSource, "'relaxed global signed i32 atomic add'"},
        {kDirectNVVMAtomicSubSource, "'atomicSub'"},
        {kDirectNVVMAtomicExchangeSource, "'atomicExchange'"},
        {kDirectNVVMAcquireGlobalI32AtomicAddSource, "'relaxed atomic-add memory order'"},
        {kDirectNVVMGroupSharedI32AtomicAddSource, "'device scalar pointer'"},
        {kDirectNVVMPointerEqualSource, "'cmpEQ'"},
        {kDirectNVVMPointerNotEqualSource, "'cmpNE'"},
        {kDirectNVVMPointerGreaterThanSource, "'cmpGT'"},
        {kDirectNVVMPointerLessEqualSource, "'cmpLE'"},
        {kDirectNVVMPointerGreaterEqualSource, "'cmpGE'"},
    };

    // The direct subset retains scalar-only runtime helper/value policy. Noncanonical layout,
    // local memory, logical NOT/shifts/division/remainder, libdevice calls, atomic-add ABI
    // variants, non-relaxed atomic-add order, adjacent atomic operations, group-shared atomic add,
    // non-i32 shared arrays, pointer comparisons, unsigned indices, and helper-array-pointer
    // shapes remain deterministic before builder discovery.
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
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}
