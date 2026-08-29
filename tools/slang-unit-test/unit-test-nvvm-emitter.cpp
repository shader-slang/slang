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
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.callResultKinds[i] == FakeNVVMBuilderResultTypeKind::ValueVector);
            SLANG_CHECK(gFakeNVVMBuilder.callResultTypes[i] == _getFakeNVVMBuilderVectorType(3));
        }
        SLANG_CHECK(gFakeNVVMBuilder.callResultKinds[4] == FakeNVVMBuilderResultTypeKind::Void);

        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 12);
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

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementType == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.executionRegisterOperations.getCount() == 3);

        SLANG_CHECK(gFakeNVVMBuilder.emitVectorConstructCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorConstructResultTypes.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.vectorConstructResultTypes[0] == _getFakeNVVMBuilderVectorType(2));
        SLANG_CHECK(gFakeNVVMBuilder.vectorConstructElementCounts[0] == 2);
        const Index constructOffset = gFakeNVVMBuilder.vectorConstructElementOffsets[0];
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
        SLANG_CHECK(
            gFakeNVVMBuilder.vectorElementBaseValueRefs[first.index].kind ==
            FakeNVVMBuilderValueKind::Call);
        const FakeNVVMBuilderValueRef firstBase =
            gFakeNVVMBuilder.vectorElementBaseValueRefs[first.index];
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
        bool sawFloat32x3Remainder = false;
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
            sawFloat32x3Remainder = sawFloat32x3Remainder ||
                                    (operation.key.operation == SLANG_NVVM_VALUE_OP_REMAINDER &&
                                     type.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                                     type.bitWidth == 32 && type.laneCount == 3);
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
        SLANG_CHECK(sawFloat32x3Remainder);
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
        SLANG_CHECK(gFakeNVVMBuilder.globalStorageLinkage == SLANG_NVVM_LINKAGE_EXTERNAL);
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
        for (auto resultType : gFakeNVVMBuilder.loadResultTypeKinds)
        {
            sawCompactVectorLoad |= resultType == FakeNVVMBuilderScalarTypeKind::NumericArray;
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
        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices.getCount() == 5);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[0] == 2);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[1] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[2] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[3] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.vectorElementIndices[4] == 0);

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
        for (SlangNVVMTypeHandle resultType : gFakeNVVMBuilder.callResultTypes)
        {
            if (resultType == _getFakeNVVMBuilderBooleanType())
                ++booleanCallCount;
        }
        SLANG_CHECK(booleanCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::ScalarPhi);
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
        for (SlangNVVMTypeHandle resultType : gFakeNVVMBuilder.callResultTypes)
        {
            if (resultType == _getFakeNVVMBuilderBooleanType())
                ++booleanCallCount;
        }
        SLANG_CHECK(booleanCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarPhiTypes[0] == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::ScalarPhi);
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
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeResultTypes[chooseType] == half2Type);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeResultTypes[adjustType] == half2Type);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[chooseType] == 3);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[adjustType] == 1);
        const Index chooseParameterOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[chooseType];
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterTypes[chooseParameterOffset] == half2Type);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[chooseParameterOffset + 1] == half2Type);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypes[chooseParameterOffset + 2] ==
            _getFakeNVVMBuilderBooleanType());
        const Index adjustParameterOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[adjustType];
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterTypes[adjustParameterOffset] == half2Type);

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
        bool sawHalf2Call = false;
        for (SlangNVVMTypeHandle callType : gFakeNVVMBuilder.callResultTypes)
            sawHalf2Call |= callType == half2Type;
        SLANG_CHECK(sawHalf2Call);
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
            SLANG_CHECK(operation.operands[0].kind == FakeNVVMBuilderValueKind::Parameter);
            const SlangNVVMValueTypeDesc& operandType = operation.operandTypes[0];
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
        SLANG_CHECK(sawBool);
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
        SLANG_CHECK(gFakeNVVMBuilder.emitAggregateElementExtractCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadResultTypeKinds[0] == FakeNVVMBuilderScalarTypeKind::ScalarStruct);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 4);
        const uint32_t expectedStoreAlignments[] = {4, 4, 8, 8};
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
        SLANG_CHECK(finalValue.kind == FakeNVVMBuilderValueKind::Load);
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

    const char* expectedDiagnostics[] = {
        "helper function result type",
        "invalid vector element count",
    };
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

        SLANG_CHECK(gFakeNVVMBuilder.emitSequentialElementExtractCallCount == 6);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 5);
        for (uint32_t alignment : gFakeNVVMBuilder.storeAlignments)
            SLANG_CHECK(alignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRejectsDoubleVectorStructuredBufferBeforeProviderMutation)
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
            kDirectNVVMUnsupportedDoubleVectorStructuredBufferSource,
            code,
            diagnostics);
        SLANG_CHECK(SLANG_FAILED(result));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("entry-point parameter") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
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
        kDirectNVVMUnsupportedNestedAggregateParameterSource,
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
        {kDirectNVVMUnsupportedFloatArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedHalfAddSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedDoubleAddSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedNestedArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedNestedLocalArraySource, "'var'"},
        {kDirectNVVMUnsupportedSharedFloatArraySource, "'sequential element pointer'"},
        {kDirectNVVMUnsupportedStructPointerSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedArrayPointerHelperSource, "'helper function parameter'"},
        {kDirectNVVMNonCanonicalCUDAOffsetSource, "'CUDA layout query'"},
        {kDirectNVVMUnsupportedFixedSamplerArrayStorageSource, "'struct field address'"},
        {kDirectNVVMUnsupportedNestedParameterBlockSource, "'struct field address'"},
        {kDirectNVVMUnsupportedNestedConstantBufferSource, "'struct field address'"},
        {kDirectNVVMFloatingSineSource, "'GenericAsm'"},
        {kDirectNVVMUnsupportedScalarTruthinessSignatureSource, "'GenericAsm'"},
        {kDirectNVVMUnsupportedOpaqueHalfConversionSignatureSource, "'GenericAsm'"},
        {kDirectNVVMUnsupportedSurfaceSignatureSource, "'GenericAsm'"},
        {kDirectNVVMLogicalNotSource, "'entry-point parameter'"},
        {kDirectNVVMWideAtomicAddSource, "'selected atomic operation'"},
        {kDirectNVVMFloatingAtomicAddSource, "'selected atomic operation'"},
        {kDirectNVVMAtomicSubSource, "'atomicSub'"},
        {kDirectNVVMAtomicExchangeSource, "'atomicExchange'"},
        {kDirectNVVMAcquireGlobalI32AtomicAddSource, "'selected atomic operation'"},
        {kDirectNVVMPointerEqualSource, "'cmpEQ'"},
        {kDirectNVVMPointerNotEqualSource, "'cmpNE'"},
        {kDirectNVVMPointerGreaterThanSource, "'cmpGT'"},
        {kDirectNVVMPointerLessEqualSource, "'cmpLE'"},
        {kDirectNVVMPointerGreaterEqualSource, "'cmpGE'"},
    };

    // Noncanonical layout, escaping or dynamically addressed local memory, logical NOT, libdevice
    // calls, malformed-signature opaque-Half and surface helpers, atomic-add ABI variants,
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
