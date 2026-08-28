// unit-test-nvvm-emitter.cpp

#include "unit-test-nvvm-support.h"

SLANG_UNIT_TEST(nvvmSlangV3RoutesGenericScalarFamilies)
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
        {kDirectNVVMIntegerNegateSource, Family::Unary, SLANG_NVVM_INTEGER_UNARY_OP_NEGATE},
        {kDirectNVVMIntegerMultiplySource, Family::Binary, SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY},
        {kDirectNVVMIntegerEqualSource, Family::Compare, SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL},
    };

    for (const auto& testCase : kCases)
    {
        _resetDirectNVVMFakes();
        _enableFakeNVVMBuilderV3();
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
                    gFakeNVVMBuilder
                        .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Unary)] == 1);
            }
            else if (testCase.family == Family::Binary)
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder
                        .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)] == 1);
            }
            else
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder
                        .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Compare)] ==
                    1);
            }
            SLANG_CHECK(gFakeNVVMBuilder.scalarV3Operations.getCount() == 1);
            SLANG_CHECK(gFakeNVVMBuilder.scalarV3Operations[0].operation == testCase.operation);
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
    _enableFakeNVVMBuilderV3();
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
        SLANG_CHECK(gFakeNVVMBuilder.scalarV3FamilyCallCounts[Index(family)] == 1);
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
    _enableFakeNVVMBuilderV3();
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
            gFakeNVVMBuilder
                .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingCompare)] ==
            1);
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
    _enableFakeNVVMBuilderV3();
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
    _enableFakeNVVMBuilderV3();
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
    _enableFakeNVVMBuilderV3();
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
            gFakeNVVMBuilder
                .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingBinary)] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 1);
        const FakeNVVMBuilderScalarOperation& addition = gFakeNVVMBuilder.scalarOperations[0];
        SLANG_CHECK(addition.key.family == FakeNVVMBuilderScalarFamily::FloatingBinary);
        SLANG_CHECK(addition.key.operation == SLANG_NVVM_FLOATING_BINARY_OP_ADD);
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
    _enableFakeNVVMBuilderV3();
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
        SLANG_CHECK(
            gFakeNVVMBuilder.intrinsicOperations[0] == SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_INDEX);
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

SLANG_UNIT_TEST(nvvmSlangWaveLaneCountUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
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
        SLANG_CHECK(
            gFakeNVVMBuilder.intrinsicOperations[0] == SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_INDEX);
        SLANG_CHECK(
            gFakeNVVMBuilder.intrinsicOperations[1] == SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_COUNT);
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
    _enableFakeNVVMBuilderV3();
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
            if (gFakeNVVMBuilder.intrinsicOperations[i] ==
                SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_UINT)
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
    _enableFakeNVVMBuilderV3();
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
            if (gFakeNVVMBuilder.intrinsicOperations[i] ==
                SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_INT)
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
    _enableFakeNVVMBuilderV3();
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
            if (gFakeNVVMBuilder.intrinsicOperations[i] ==
                SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_FLOAT)
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
    _enableFakeNVVMBuilderV3();
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
            if (gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_BALLOT)
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

// Compiles one public scalar wave-read fixture and checks its canonical mask-to-shuffle topology.
static void _checkUnmaskedWaveReadLaneAtDirectPipeline(
    const char* source,
    SlangNVVMIntrinsicOp_3 shuffleOperation,
    FakeNVVMBuilderValueKind entryValueKind,
    Index expectedPointerOffsetCount,
    Index expectedLoadCount)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
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
        Index shuffleIntrinsicIndex = -1;
        for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
        {
            if (gFakeNVVMBuilder.intrinsicOperations[i] == SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_BALLOT)
                ballotIntrinsicIndex = i;
            else if (gFakeNVVMBuilder.intrinsicOperations[i] == shuffleOperation)
                shuffleIntrinsicIndex = i;
        }
        SLANG_CHECK_ABORT(ballotIntrinsicIndex >= 0);
        SLANG_CHECK_ABORT(shuffleIntrinsicIndex >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[ballotIntrinsicIndex] == 2);
        SLANG_CHECK(gFakeNVVMBuilder.intrinsicArgumentCounts[shuffleIntrinsicIndex] == 3);

        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 4);
        Index publicCallIndex = -1;
        Index activeMaskCallIndex = -1;
        Index maskedShuffleCallIndex = -1;
        for (Index callIndex = 0; callIndex < gFakeNVVMBuilder.callArgumentCounts.getCount();
             ++callIndex)
        {
            const Index argumentCount = gFakeNVVMBuilder.callArgumentCounts[callIndex];
            const Index argumentOffset = gFakeNVVMBuilder.callArgumentOffsets[callIndex];
            if (argumentCount == 1)
            {
                activeMaskCallIndex = callIndex;
            }
            else if (argumentCount == 3)
            {
                const FakeNVVMBuilderValueRef& thirdArgument =
                    gFakeNVVMBuilder.callArgumentValueRefs[argumentOffset + 2];
                if (thirdArgument.kind == FakeNVVMBuilderValueKind::Intrinsic)
                    publicCallIndex = callIndex;
                else
                    maskedShuffleCallIndex = callIndex;
            }
        }
        SLANG_CHECK_ABORT(publicCallIndex >= 0);
        SLANG_CHECK_ABORT(activeMaskCallIndex >= 0);
        SLANG_CHECK_ABORT(maskedShuffleCallIndex >= 0);

        const Index publicArgumentOffset = gFakeNVVMBuilder.callArgumentOffsets[publicCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[publicArgumentOffset + 0].kind ==
            entryValueKind);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[publicArgumentOffset + 1].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[publicArgumentOffset + 2].kind ==
            FakeNVVMBuilderValueKind::Intrinsic);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[publicArgumentOffset + 2].index ==
            ballotIntrinsicIndex);

        const Index activeMaskArgumentOffset =
            gFakeNVVMBuilder.callArgumentOffsets[activeMaskCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[activeMaskArgumentOffset].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        const Index maskedShuffleArgumentOffset =
            gFakeNVVMBuilder.callArgumentOffsets[maskedShuffleCallIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[maskedShuffleArgumentOffset + 0].kind ==
            FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[maskedShuffleArgumentOffset + 0].index ==
            activeMaskCallIndex);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[maskedShuffleArgumentOffset + 1].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(
            gFakeNVVMBuilder.callArgumentValueRefs[maskedShuffleArgumentOffset + 2].kind ==
            FakeNVVMBuilderValueKind::Parameter);
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
    _checkUnmaskedWaveReadLaneAtDirectPipeline(
        kDirectNVVMUnmaskedWaveReadLaneAtUIntSource,
        SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_UINT,
        FakeNVVMBuilderValueKind::Call,
        1,
        0);
}

SLANG_UNIT_TEST(nvvmSlangUnmaskedWaveReadLaneAtIntUsesDirectPipeline)
{
    _checkUnmaskedWaveReadLaneAtDirectPipeline(
        kDirectNVVMUnmaskedWaveReadLaneAtIntSource,
        SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_INT,
        FakeNVVMBuilderValueKind::Load,
        2,
        1);
}

SLANG_UNIT_TEST(nvvmSlangUnmaskedWaveReadLaneAtFloatUsesDirectPipeline)
{
    _checkUnmaskedWaveReadLaneAtDirectPipeline(
        kDirectNVVMUnmaskedWaveReadLaneAtFloatSource,
        SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_FLOAT,
        FakeNVVMBuilderValueKind::Load,
        2,
        1);
}

SLANG_UNIT_TEST(nvvmSlangFloat32CopyUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
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
            gFakeNVVMBuilder
                .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingBinary)] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder
                .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingUnary)] == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

static void _runNVVMSlangNegotiatesFloat32Capability(
    const char* source,
    SlangNVVMBuilderFeature_3 feature,
    FakeNVVMBuilderScalarFamily family)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features.words[feature / 64u] &= ~(uint64_t(1) << (feature % 64u));
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(
            SLANG_FAILED(_compileSlangWithDirectNVVM(globalSession, source, code, diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.scalarV3FamilyCallCounts[Index(family)] == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

static void _runNVVMSlangNegotiatesFloat32ArithmeticCapability(
    NVVMFloat32ArithmeticTestOperation testOperation)
{
    const NVVMFloat32ArithmeticTestCase& testCase =
        _getNVVMFloat32ArithmeticTestCase(testOperation);
    const FakeNVVMBuilderScalarFamily family = testCase.operandCount == 1
                                                   ? FakeNVVMBuilderScalarFamily::FloatingUnary
                                                   : FakeNVVMBuilderScalarFamily::FloatingBinary;
    _runNVVMSlangNegotiatesFloat32Capability(testCase.source, testCase.feature, family);
}

#define NVVM_FLOAT32_ARITHMETIC_CAPABILITY_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                        \
    {                                                            \
        _runNVVMSlangNegotiatesFloat32ArithmeticCapability(      \
            NVVMFloat32ArithmeticTestOperation::OPERATION);      \
    }

NVVM_FLOAT32_ARITHMETIC_CAPABILITY_TEST(nvvmSlangNegotiatesFloat32AddCapability, Add)
NVVM_FLOAT32_ARITHMETIC_CAPABILITY_TEST(nvvmSlangNegotiatesFloat32SubtractCapability, Subtract)
NVVM_FLOAT32_ARITHMETIC_CAPABILITY_TEST(nvvmSlangNegotiatesFloat32MultiplyCapability, Multiply)
NVVM_FLOAT32_ARITHMETIC_CAPABILITY_TEST(nvvmSlangNegotiatesFloat32DivideCapability, Divide)
NVVM_FLOAT32_ARITHMETIC_CAPABILITY_TEST(nvvmSlangNegotiatesFloat32NegateCapability, Negate)

#undef NVVM_FLOAT32_ARITHMETIC_CAPABILITY_TEST

static void _runNVVMSlangNegotiatesFloat32ComparisonCapability(
    NVVMFloat32ComparisonTestOperation testOperation)
{
    const NVVMFloat32ComparisonTestCase& testCase =
        _getNVVMFloat32ComparisonTestCase(testOperation);
    _runNVVMSlangNegotiatesFloat32Capability(
        testCase.source,
        testCase.feature,
        FakeNVVMBuilderScalarFamily::FloatingCompare);
}

#define NVVM_FLOAT32_COMPARISON_CAPABILITY_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                        \
    {                                                            \
        _runNVVMSlangNegotiatesFloat32ComparisonCapability(      \
            NVVMFloat32ComparisonTestOperation::OPERATION);      \
    }

NVVM_FLOAT32_COMPARISON_CAPABILITY_TEST(nvvmSlangNegotiatesFloat32EqualCapability, OrderedEqual)
NVVM_FLOAT32_COMPARISON_CAPABILITY_TEST(
    nvvmSlangNegotiatesFloat32NotEqualCapability,
    UnorderedNotEqual)
NVVM_FLOAT32_COMPARISON_CAPABILITY_TEST(
    nvvmSlangNegotiatesFloat32GreaterThanCapability,
    OrderedGreaterThan)
NVVM_FLOAT32_COMPARISON_CAPABILITY_TEST(
    nvvmSlangNegotiatesFloat32LessEqualCapability,
    OrderedLessEqual)
NVVM_FLOAT32_COMPARISON_CAPABILITY_TEST(
    nvvmSlangNegotiatesFloat32GreaterEqualCapability,
    OrderedGreaterEqual)
NVVM_FLOAT32_COMPARISON_CAPABILITY_TEST(
    nvvmSlangNegotiatesFloat32LessThanCapability,
    OrderedLessThan)

#undef NVVM_FLOAT32_COMPARISON_CAPABILITY_TEST

SLANG_UNIT_TEST(nvvmSlangNegotiatesFloat32ConstantCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features
        .words[SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT % 64u));
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
            kDirectNVVMFloat32ConstantSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointConstantCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarPhiCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features.words[SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI % 64u));
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
            kDirectNVVMFloat32PhiSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.addPhiIncomingCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesGenericScalarFunctionCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features
        .words[SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS % 64u));
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
            kDirectNVVMFloat32FunctionSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitValueReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesWaveLaneIndexCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features.words[SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX % 64u));
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
            kDirectNVVMWaveLaneIndexSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesWaveLaneCountCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features.words[SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT % 64u));
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
            kDirectNVVMWaveLaneCountSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesWaveReadLaneAtUIntCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features
        .words[SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT % 64u));
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
            kDirectNVVMWaveReadLaneAtUIntSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesWaveReadLaneAtIntCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features.words[SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT % 64u));
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
            kDirectNVVMWaveReadLaneAtIntSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesWaveReadLaneAtFloatCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features
        .words[SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT % 64u));
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
            kDirectNVVMWaveReadLaneAtFloatSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesWaveMaskBallotCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features.words[SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT % 64u));
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
            kDirectNVVMWaveActiveMaskSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

// Proves that every constituent operation is required before provider module construction.
static void _checkUnmaskedWaveReadLaneAtCapabilities(
    const char* source,
    SlangNVVMBuilderFeature_3 shuffleFeature)
{
    const SlangNVVMBuilderFeature_3 requiredFeatures[] = {
        SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX,
        shuffleFeature,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT,
    };
    for (SlangNVVMBuilderFeature_3 missingFeature : requiredFeatures)
    {
        _resetDirectNVVMFakes();
        _enableFakeNVVMBuilderV3();
        gFakeNVVMBuilder.apiV3.features.words[missingFeature / 64u] &=
            ~(uint64_t(1) << (missingFeature % 64u));
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
            SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitIntrinsicCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesUnmaskedWaveReadLaneAtUIntCapabilities)
{
    _checkUnmaskedWaveReadLaneAtCapabilities(
        kDirectNVVMUnmaskedWaveReadLaneAtUIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesUnmaskedWaveReadLaneAtIntCapabilities)
{
    _checkUnmaskedWaveReadLaneAtCapabilities(
        kDirectNVVMUnmaskedWaveReadLaneAtIntSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesUnmaskedWaveReadLaneAtFloatCapabilities)
{
    _checkUnmaskedWaveReadLaneAtCapabilities(
        kDirectNVVMUnmaskedWaveReadLaneAtFloatSource,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesFloat32CopyCapability)
{
    _resetDirectNVVMFakes();
    _enableFakeNVVMBuilderV3();
    gFakeNVVMBuilder.apiV3.features.words[SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD / 64u] &=
        ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD % 64u));
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
            kDirectNVVMFloat32CopySource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
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
                    SLANG_NVVM_INTEGER_BINARY_OP_3_ADD);
                const Index subIndex = _findFakeNVVMBuilderScalarOperation(
                    FakeNVVMBuilderScalarFamily::Binary,
                    SLANG_NVVM_INTEGER_BINARY_OP_3_SUB);
                const Index compareIndex = _findFakeNVVMBuilderScalarOperation(
                    FakeNVVMBuilderScalarFamily::Compare,
                    SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN);
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
                    SLANG_NVVM_INTEGER_BINARY_OP_3_ADD);
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
                    SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN);
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
                            SLANG_NVVM_INTEGER_BINARY_OP_3_ADD))
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
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 2);
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
                SLANG_NVVM_INTEGER_BINARY_OP_3_ADD));
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

        SLANG_CHECK(gFakeNVVMBuilder.integerReturnValueRefs.getCount() == 2);
        bool sawIncrementReturn = false;
        bool sawIncrementTwiceReturn = false;
        for (Index returnIndex = 0;
             returnIndex < gFakeNVVMBuilder.integerReturnValueRefs.getCount();
             ++returnIndex)
        {
            const Index returnBlock = gFakeNVVMBuilder.integerReturnBlockIndices[returnIndex];
            const Index returnFunction = gFakeNVVMBuilder.blockFunctionIndices[returnBlock];
            const FakeNVVMBuilderValueRef returnValue =
                gFakeNVVMBuilder.integerReturnValueRefs[returnIndex];
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
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 1);
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

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getRawRWStructuredBufferI32TypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 0);
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
            FakeNVVMBuilderParameterTypeKind::RawRWStructuredBufferI32);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitRawRWStructuredBufferI32ElementPointerCallCount == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerBufferValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerIndexValueRefs.getCount() == 1);
        const FakeNVVMBuilderValueRef buffer =
            gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerBufferValueRefs[0];
        const FakeNVVMBuilderValueRef index =
            gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerIndexValueRefs[0];
        SLANG_CHECK(buffer.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(buffer.functionIndex == 0);
        SLANG_CHECK(buffer.index == 0);
        SLANG_CHECK(index.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(index.functionIndex == 0);
        SLANG_CHECK(index.index == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerCallerBlockIndices[0] == 0);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == 42);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::RawRWStructuredBufferI32ElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
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
SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarControlFlowCapability)
{
    // A provider frozen at the exact Slice 4 scalar-memory prefix can still compile scalar loads
    // and stores through the public route.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE);
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
            kDirectNVVMCopyScalarSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // The same provider cannot satisfy the append-only control-flow prefix. Capability rejection
    // happens after discovery but before the emitter creates or mutates a module.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE);
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
            kDirectNVVMChooseScalarSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarSSACapability)
{
    // The exact Slice 7 provider retains its published branch capability.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE);
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
            kDirectNVVMChooseScalarSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // Each Slice 8 shape is legal Slang IR but needs the new complete builder prefix. Discovery
    // succeeds and E52016 is emitted before module creation or libNVVM use.
    static const char* kSources[] = {
        kDirectNVVMIntegerConstantSource,
        kDirectNVVMMergePhiSource,
        kDirectNVVMFiniteLoopSource,
    };
    for (const char* source : kSources)
    {
        _resetDirectNVVMFakes();
        gFakeNVVMBuilder.apiV2.structureSize =
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE);
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
            SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarFunctionCapability)
{
    // A provider frozen at the exact Slice 8 prefix can still compile the complete loop shape
    // published by that slice.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE);
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
            kDirectNVVMFiniteLoopSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // A reachable scalar helper needs the append-only Slice 9 prefix. Reject it after discovery
    // but before module creation or libNVVM use.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE);
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
            kDirectNVVMScalarFunctionSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarPointerArithmeticCapability)
{
    // A provider frozen at the exact Slice 9 prefix retains the complete scalar-function graph
    // published by that slice.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE);
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
            kDirectNVVMScalarFunctionSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 4);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // The same provider cannot satisfy the append-only pointer-arithmetic prefix. Discovery
    // succeeds, then E52016 is reported before module creation or libNVVM use.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE);
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
            kDirectNVVMPointerOffsetSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarArrayAddressingCapability)
{
    // The exact Slice 10 provider retains the pointer-offset program published by that slice.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE);
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
            kDirectNVVMPointerOffsetSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // The array program needs the coherent Slice 11 suffix. Gate after discovery but before any
    // provider module or libNVVM program is created.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE);
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
            kDirectNVVMFixedDeviceArraySource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

struct NVVMScalarCapabilityCase
{
    uint32_t frozenV2Size;
    const char* predecessorSource;
    const NVVMScalarTestCase* predecessorScalar;
    bool predecessorUsesArrayAddressing;
    bool predecessorUsesAtomic;
};

static NVVMScalarCapabilityCase _getNVVMScalarCapabilityCase(NVVMScalarTestOperation operation)
{
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE),
            kDirectNVVMFixedDeviceArraySource,
            nullptr,
            true,
            false};
    case NVVMScalarTestOperation::BitAnd:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE),
            kDirectNVVMIntegerMultiplySource,
            &_getNVVMScalarTestCase(NVVMScalarTestOperation::Multiply),
            false,
            false};
    case NVVMScalarTestOperation::BitOr:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE),
            kDirectNVVMIntegerBitAndSource,
            &_getNVVMScalarTestCase(NVVMScalarTestOperation::BitAnd),
            false,
            false};
    case NVVMScalarTestOperation::BitXor:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE),
            kDirectNVVMIntegerBitOrSource,
            &_getNVVMScalarTestCase(NVVMScalarTestOperation::BitOr),
            false,
            false};
    case NVVMScalarTestOperation::BitNot:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE),
            kDirectNVVMIntegerBitXorSource,
            &_getNVVMScalarTestCase(NVVMScalarTestOperation::BitXor),
            false,
            false};
    case NVVMScalarTestOperation::Negate:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE),
            kDirectNVVMIntegerBitNotSource,
            &_getNVVMScalarTestCase(NVVMScalarTestOperation::BitNot),
            false,
            false};
    case NVVMScalarTestOperation::Equal:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE),
            kDirectNVVMRelaxedGlobalI32AtomicAddSource,
            nullptr,
            false,
            true};
    case NVVMScalarTestOperation::NotEqual:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_EQUAL_MIN_SIZE),
            kDirectNVVMIntegerEqualSource,
            &_getNVVMScalarTestCase(NVVMScalarTestOperation::Equal),
            false,
            false};
    case NVVMScalarTestOperation::SignedGreaterThan:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NOT_EQUAL_MIN_SIZE),
            kDirectNVVMIntegerNotEqualSource,
            &_getNVVMScalarTestCase(NVVMScalarTestOperation::NotEqual),
            false,
            false};
    case NVVMScalarTestOperation::SignedLessEqual:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_THAN_MIN_SIZE),
            kDirectNVVMIntegerSignedGreaterThanSource,
            &_getNVVMScalarTestCase(NVVMScalarTestOperation::SignedGreaterThan),
            false,
            false};
    case NVVMScalarTestOperation::SignedGreaterEqual:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_LESS_EQUAL_MIN_SIZE),
            kDirectNVVMIntegerSignedLessEqualSource,
            &_getNVVMScalarTestCase(NVVMScalarTestOperation::SignedLessEqual),
            false,
            false};
    }
    SLANG_UNEXPECTED("unknown NVVM scalar capability operation");
}

static void _runNVVMScalarCapabilityNegotiation(NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    const NVVMScalarCapabilityCase capabilityCase = _getNVVMScalarCapabilityCase(operation);

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = capabilityCase.frozenV2Size;
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
            capabilityCase.predecessorSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(
            _getFakeNVVMBuilderScalarOperationCallCount(
                testCase.key.family,
                testCase.key.operation) == 0);
        if (capabilityCase.predecessorScalar)
        {
            SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 1);
            SLANG_CHECK(_isFakeNVVMBuilderScalarOperation(
                gFakeNVVMBuilder.scalarOperations[0].key,
                capabilityCase.predecessorScalar->key.family,
                capabilityCase.predecessorScalar->key.operation));
        }
        if (capabilityCase.predecessorUsesArrayAddressing)
            SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 2);
        if (capabilityCase.predecessorUsesAtomic)
            SLANG_CHECK(gFakeNVVMBuilder.emitRelaxedGlobalI32AtomicAddCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = capabilityCase.frozenV2Size;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(
            _compileSlangWithDirectNVVM(globalSession, testCase.source, code, diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(
            _getFakeNVVMBuilderScalarOperationCallCount(
                testCase.key.family,
                testCase.key.operation) == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

#define NVVM_SCALAR_CAPABILITY_TEST(NAME, OPERATION)                             \
    SLANG_UNIT_TEST(NAME)                                                        \
    {                                                                            \
        _runNVVMScalarCapabilityNegotiation(NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_CAPABILITY_TEST(nvvmSlangNegotiatesScalarIntegerMultiplyCapability, Multiply)
NVVM_SCALAR_CAPABILITY_TEST(nvvmSlangNegotiatesScalarIntegerBitAndCapability, BitAnd)
NVVM_SCALAR_CAPABILITY_TEST(nvvmSlangNegotiatesScalarIntegerBitOrCapability, BitOr)
NVVM_SCALAR_CAPABILITY_TEST(nvvmSlangNegotiatesScalarIntegerBitXorCapability, BitXor)
NVVM_SCALAR_CAPABILITY_TEST(nvvmSlangNegotiatesScalarIntegerBitNotCapability, BitNot)
NVVM_SCALAR_CAPABILITY_TEST(nvvmSlangNegotiatesScalarIntegerNegateCapability, Negate)
SLANG_UNIT_TEST(nvvmSlangNegotiatesRelaxedGlobalI32AtomicAddCapability)
{
    // The exact Slice 17 provider retains integer negate from that slice.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE);
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
            kDirectNVVMIntegerNegateSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        const NVVMScalarTestCase& negateCase =
            _getNVVMScalarTestCase(NVVMScalarTestOperation::Negate);
        SLANG_CHECK(
            _getFakeNVVMBuilderScalarOperationCallCount(
                negateCase.key.family,
                negateCase.key.operation) == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitRelaxedGlobalI32AtomicAddCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addedModule.getLength() == sizeof(kFakeNVVMBuilderBitcode));
        SLANG_CHECK(
            ::memcmp(
                gFakeNVVM.addedModule.getBuffer(),
                kFakeNVVMBuilderBitcode,
                sizeof(kFakeNVVMBuilderBitcode)) == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // Relaxed global i32 atomic add needs the appended Slice 19 operation. Discovery succeeds,
    // then E52016 is reported before module creation or libNVVM use.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE);
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
            kDirectNVVMRelaxedGlobalI32AtomicAddSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitRelaxedGlobalI32AtomicAddCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

NVVM_SCALAR_CAPABILITY_TEST(nvvmSlangNegotiatesScalarIntegerEqualCapability, Equal)
NVVM_SCALAR_CAPABILITY_TEST(nvvmSlangNegotiatesScalarIntegerNotEqualCapability, NotEqual)
NVVM_SCALAR_CAPABILITY_TEST(
    nvvmSlangNegotiatesScalarIntegerSignedGreaterThanCapability,
    SignedGreaterThan)
NVVM_SCALAR_CAPABILITY_TEST(
    nvvmSlangNegotiatesScalarIntegerSignedLessEqualCapability,
    SignedLessEqual)
NVVM_SCALAR_CAPABILITY_TEST(
    nvvmSlangNegotiatesScalarIntegerSignedGreaterEqualCapability,
    SignedGreaterEqual)

#undef NVVM_SCALAR_CAPABILITY_TEST
SLANG_UNIT_TEST(nvvmSlangNegotiatesRawRWStructuredBufferI32Capability)
{
    // The exact Slice 25 provider retains signed-i32 greater-equal.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE);
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
            kDirectNVVMIntegerSignedGreaterEqualSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        const NVVMScalarTestCase& greaterEqualCase =
            _getNVVMScalarTestCase(NVVMScalarTestOperation::SignedGreaterEqual);
        SLANG_CHECK(
            _getFakeNVVMBuilderScalarOperationCallCount(
                greaterEqualCase.key.family,
                greaterEqualCase.key.operation) == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getRawRWStructuredBufferI32TypeCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // The resource program needs Slice 26. Gate after discovery and before module creation.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE);
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
            kDirectNVVMRawRWStructuredBufferI32StoreSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getRawRWStructuredBufferI32TypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitRawRWStructuredBufferI32ElementPointerCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRejectsAdjacentStructuredBufferShapesBeforeProviderMutation)
{
    static const char* kUnsupportedSources[] = {
        kDirectNVVMConventionalRWStructuredBufferI32StoreSource,
        kDirectNVVMRawRWStructuredBufferU32StoreSource,
        kDirectNVVMRawRWStructuredBufferF32StoreSource,
        kDirectNVVMRawStructuredBufferI32LoadSource,
        kDirectNVVMRawRWStructuredBufferI32LoadSource,
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
            SLANG_CHECK(gFakeNVVMBuilder.getRawRWStructuredBufferI32TypeCallCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.emitRawRWStructuredBufferI32ElementPointerCallCount == 0);
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

SLANG_UNIT_TEST(nvvmSlangUnsupportedIRStopsBeforeEmission)
{
    struct UnsupportedCase
    {
        const char* source;
        const char* expectedConstruct;
    };
    static const UnsupportedCase kCases[] = {
        {kDirectNVVMUnsupportedCallSource, "'helper function result type'"},
        {kDirectNVVMUnsupportedPointerHelperParameterSource, "'helper function parameter'"},
        {kDirectNVVMUnsupportedPointerHelperResultSource, "'helper function result type'"},
        {kDirectNVVMUnsignedPointerOffsetSource, "'integer_constant'"},
        {kDirectNVVMUnsignedFixedArrayIndexSource, "'signed i32 value'"},
        {kDirectNVVMUnsupportedFloatArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedHalfAddSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedDoubleAddSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedNestedArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedLocalArraySource, "'var'"},
        {kDirectNVVMUnsupportedStructPointerSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedArrayPointerHelperSource, "'helper function parameter'"},
        {kDirectNVVMUnsignedMultiplySource, "'signed i32 multiplication'"},
        {kDirectNVVMWideIntegerMultiplySource, "'entry-point parameter'"},
        {kDirectNVVMFloatingMultiplySource, "'castFloatToInt'"},
        {kDirectNVVMFloatingSineSource, "'castFloatToInt'"},
        {kDirectNVVMIntegerLeftShiftSource, "'shl'"},
        {kDirectNVVMIntegerRightShiftSource, "'shr'"},
        {kDirectNVVMIntegerDivideSource, "'div'"},
        {kDirectNVVMIntegerRemainderSource, "'irem'"},
        {kDirectNVVMUnsignedIntegerBitAndSource, "'signed i32 bitwise AND'"},
        {kDirectNVVMWideIntegerBitAndSource, "'entry-point parameter'"},
        {kDirectNVVMUnsignedIntegerBitOrSource, "'signed i32 bitwise OR'"},
        {kDirectNVVMWideIntegerBitOrSource, "'entry-point parameter'"},
        {kDirectNVVMUnsignedIntegerBitXorSource, "'signed i32 bitwise XOR'"},
        {kDirectNVVMWideIntegerBitXorSource, "'entry-point parameter'"},
        {kDirectNVVMLogicalNotSource, "'entry-point parameter'"},
        {kDirectNVVMUnsignedIntegerBitNotSource, "'signed i32 bitwise NOT'"},
        {kDirectNVVMWideIntegerBitNotSource, "'entry-point parameter'"},
        {kDirectNVVMUnsignedIntegerNegateSource, "'signed i32 arithmetic negation'"},
        {kDirectNVVMWideIntegerNegateSource, "'entry-point parameter'"},
        {kDirectNVVMFloatingNegateSource, "'castFloatToInt'"},
        {kDirectNVVMUnsignedAtomicAddSource, "'relaxed global signed i32 atomic add'"},
        {kDirectNVVMWideAtomicAddSource, "'entry-point parameter'"},
        {kDirectNVVMFloatingAtomicAddSource, "'relaxed global signed i32 atomic add'"},
        {kDirectNVVMAtomicSubSource, "'atomicSub'"},
        {kDirectNVVMAtomicExchangeSource, "'atomicExchange'"},
        {kDirectNVVMAcquireGlobalI32AtomicAddSource, "'relaxed atomic-add memory order'"},
        {kDirectNVVMGroupSharedI32AtomicAddSource, "'device scalar pointer'"},
        {kDirectNVVMUnsignedIntegerEqualSource, "'signed i32 value'"},
        {kDirectNVVMWideIntegerEqualSource, "'entry-point parameter'"},
        {kDirectNVVMPointerEqualSource, "'signed i32 value'"},
        {kDirectNVVMUnsignedIntegerNotEqualSource, "'signed i32 value'"},
        {kDirectNVVMWideIntegerNotEqualSource, "'entry-point parameter'"},
        {kDirectNVVMPointerNotEqualSource, "'signed i32 value'"},
        {kDirectNVVMUnsignedIntegerGreaterThanSource, "'signed i32 value'"},
        {kDirectNVVMWideIntegerGreaterThanSource, "'entry-point parameter'"},
        {kDirectNVVMPointerGreaterThanSource, "'signed i32 value'"},
        {kDirectNVVMUnsignedIntegerLessEqualSource, "'signed i32 value'"},
        {kDirectNVVMWideIntegerLessEqualSource, "'entry-point parameter'"},
        {kDirectNVVMPointerLessEqualSource, "'signed i32 value'"},
        {kDirectNVVMUnsignedIntegerGreaterEqualSource, "'signed i32 value'"},
        {kDirectNVVMWideIntegerGreaterEqualSource, "'entry-point parameter'"},
        {kDirectNVVMPointerGreaterEqualSource, "'signed i32 value'"},
    };

    // The direct subset retains scalar-only helper/value policy. Adjacent aggregate, local-memory,
    // multiply ABI variants, logical NOT/shifts/division/remainder, unsigned/wide AND/OR/XOR/NOT,
    // unsigned/wide negate and floating-negate-plus-cast and atomic-add ABI variants,
    // non-relaxed atomic-add order,
    // adjacent atomic operations, group-shared atomic add, unsigned/wide equality, floating and
    // unsigned/wide inequality and ordered comparisons, pointer comparisons, unsigned indices,
    // and helper-array-pointer shapes remain deterministic before builder discovery.
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
