// unit-test-nvvm-builder.cpp

#include "unit-test-nvvm-support.h"

static bool _supportsNVVMScalarBuilderOperation(
    const NVVMIRBuilder& builder,
    NVVMScalarTestOperation operation)
{
    SlangNVVMValueOperation valueOperation = 0;
    bool isUnary = false;
    bool isCompare = false;
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        valueOperation = SLANG_NVVM_VALUE_OP_MULTIPLY;
        break;
    case NVVMScalarTestOperation::BitAnd:
        valueOperation = SLANG_NVVM_VALUE_OP_BIT_AND;
        break;
    case NVVMScalarTestOperation::BitOr:
        valueOperation = SLANG_NVVM_VALUE_OP_BIT_OR;
        break;
    case NVVMScalarTestOperation::BitXor:
        valueOperation = SLANG_NVVM_VALUE_OP_BIT_XOR;
        break;
    case NVVMScalarTestOperation::BitNot:
        valueOperation = SLANG_NVVM_VALUE_OP_BIT_NOT;
        isUnary = true;
        break;
    case NVVMScalarTestOperation::Negate:
        valueOperation = SLANG_NVVM_VALUE_OP_NEGATE;
        isUnary = true;
        break;
    case NVVMScalarTestOperation::Equal:
        valueOperation = SLANG_NVVM_VALUE_OP_EQUAL;
        isCompare = true;
        break;
    case NVVMScalarTestOperation::NotEqual:
        valueOperation = SLANG_NVVM_VALUE_OP_NOT_EQUAL;
        isCompare = true;
        break;
    case NVVMScalarTestOperation::SignedGreaterThan:
        valueOperation = SLANG_NVVM_VALUE_OP_GREATER_THAN;
        isCompare = true;
        break;
    case NVVMScalarTestOperation::SignedLessEqual:
        valueOperation = SLANG_NVVM_VALUE_OP_LESS_EQUAL;
        isCompare = true;
        break;
    case NVVMScalarTestOperation::SignedGreaterEqual:
        valueOperation = SLANG_NVVM_VALUE_OP_GREATER_EQUAL;
        isCompare = true;
        break;
    default:
        return false;
    }

    SlangNVVMValueTypeDesc operandTypes[] = {
        NVVMSemantics::kSignedI32,
        NVVMSemantics::kSignedI32,
    };
    const SlangNVVMValueOperationDesc desc = {
        valueOperation,
        isCompare ? NVVMSemantics::kBool : NVVMSemantics::kSignedI32,
        operandTypes,
        isUnary ? 1u : 2u,
    };
    return builder.supportsValueOperation(desc);
}

static SlangResult _emitNVVMScalarBuilderOperation(
    NVVMIRBuilder& builder,
    NVVMScalarTestOperation operation,
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue)
{
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        return builder.emitIntegerMultiply(module, left, right, outValue);
    case NVVMScalarTestOperation::BitAnd:
        return builder.emitIntegerBitAnd(module, left, right, outValue);
    case NVVMScalarTestOperation::BitOr:
        return builder.emitIntegerBitOr(module, left, right, outValue);
    case NVVMScalarTestOperation::BitXor:
        return builder.emitIntegerBitXor(module, left, right, outValue);
    case NVVMScalarTestOperation::BitNot:
        return builder.emitIntegerBitNot(module, left, outValue);
    case NVVMScalarTestOperation::Negate:
        return builder.emitIntegerNegate(module, left, outValue);
    case NVVMScalarTestOperation::Equal:
        return builder.emitIntegerEqual(module, left, right, outValue);
    case NVVMScalarTestOperation::NotEqual:
        return builder.emitIntegerNotEqual(module, left, right, outValue);
    case NVVMScalarTestOperation::SignedGreaterThan:
        return builder.emitIntegerSignedGreaterThan(module, left, right, outValue);
    case NVVMScalarTestOperation::SignedLessEqual:
        return builder.emitIntegerSignedLessEqual(module, left, right, outValue);
    case NVVMScalarTestOperation::SignedGreaterEqual:
        return builder.emitIntegerSignedGreaterEqual(module, left, right, outValue);
    }
    return SLANG_E_INVALID_ARG;
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesExactCurrentABI)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        SLANG_CHECK(builder.isInitialized());
        SLANG_CHECK(builder.getAPI().llvmVersionMajor == 14);
        SLANG_CHECK(builder.getAPI().llvmVersionMinor == 0);
        SLANG_CHECK(builder.getAPI().llvmVersionPatch == 6);
        SLANG_CHECK(builder.getAPI().nvvmIRVersionMajor == 2);
        SLANG_CHECK(builder.getAPI().nvvmIRVersionMinor == 0);
        SLANG_CHECK(builder.getAPI().pointerModel == SLANG_NVVM_POINTER_MODEL_TYPED);
        SLANG_CHECK(builder.getFoundationAPI()->createModule != nullptr);
        SLANG_CHECK(builder.getConstructionAPI()->getStructType != nullptr);
        SLANG_CHECK(builder.getConstructionAPI()->declareGlobalStorage != nullptr);
        SLANG_CHECK(builder.getConstructionAPI()->emitLocalStorage != nullptr);
        SLANG_CHECK(builder.getConstructionAPI()->emitStructFieldPointer != nullptr);
        SLANG_CHECK(builder.getConstructionAPI()->emitByteOffsetPointer != nullptr);
        SLANG_CHECK(builder.getConstructionAPI()->emitSequentialElementPointer != nullptr);
        SLANG_CHECK(builder.getValueOperationsAPI()->emitOperation != nullptr);
        SLANG_CHECK(builder.getSurfaceOperationsAPI()->emitOperation != nullptr);
        SLANG_CHECK(builder.getTextureOperationsAPI()->emitOperation != nullptr);
        StringBuilder expectedABI;
        expectedABI << "builder-abi=" << SLANG_NVVM_BUILDER_ABI_REVISION;
        SLANG_CHECK(builder.getVersionString().indexOf(expectedABI.getUnownedSlice()) >= 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderQueriesTypedTextureOperations)
{
    _resetDirectNVVMFakes();
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
    NVVMIRBuilder builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));

    const SlangNVVMValueTypeDesc floatType = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        32,
        1,
    };
    const SlangNVVMTextureShape shapes[] = {
        SLANG_NVVM_TEXTURE_SHAPE_1D,
        SLANG_NVVM_TEXTURE_SHAPE_2D,
        SLANG_NVVM_TEXTURE_SHAPE_3D,
        SLANG_NVVM_TEXTURE_SHAPE_CUBE,
    };
    for (const auto shape : shapes)
    {
        const SlangNVVMTextureOperationDesc operation = {
            SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL,
            shape,
            0,
            floatType,
        };
        SLANG_CHECK(builder.supportsTextureOperation(operation));
        if (shape != SLANG_NVVM_TEXTURE_SHAPE_3D)
        {
            SlangNVVMTextureOperationDesc arrayOperation = operation;
            arrayOperation.isArray = 1;
            SLANG_CHECK(builder.supportsTextureOperation(arrayOperation));
        }
    }

    SlangNVVMTextureOperationDesc unsupported = {
        SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL,
        SLANG_NVVM_TEXTURE_SHAPE_3D,
        1,
        floatType,
    };
    SLANG_CHECK(!builder.supportsTextureOperation(unsupported));
    unsupported.shape = SLANG_NVVM_TEXTURE_SHAPE_2D;
    unsupported.elementType.laneCount = 2;
    SLANG_CHECK(builder.supportsTextureOperation(unsupported));
    unsupported.elementType.laneCount = 4;
    SLANG_CHECK(builder.supportsTextureOperation(unsupported));
    unsupported.elementType.laneCount = 3;
    SLANG_CHECK(!builder.supportsTextureOperation(unsupported));

    for (const auto shape : shapes)
    {
        SlangNVVMTextureOperationDesc query = {
            SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH,
            shape,
            0,
            floatType,
        };
        SLANG_CHECK(builder.supportsTextureOperation(query));
        if (shape != SLANG_NVVM_TEXTURE_SHAPE_3D)
        {
            query.isArray = 1;
            SLANG_CHECK(builder.supportsTextureOperation(query));
            query.isArray = 0;
        }

        query.operation = SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT;
        SLANG_CHECK(
            builder.supportsTextureOperation(query) == (shape != SLANG_NVVM_TEXTURE_SHAPE_1D));
        query.operation = SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH;
        SLANG_CHECK(
            builder.supportsTextureOperation(query) == (shape == SLANG_NVVM_TEXTURE_SHAPE_3D));
    }

    const SlangNVVMValueTypeKind fetchKinds[] = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    };
    const uint32_t fetchLaneCounts[] = {1, 2, 4};
    for (const auto kind : fetchKinds)
    {
        for (const auto laneCount : fetchLaneCounts)
        {
            SlangNVVMTextureOperationDesc fetch = {
                SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL,
                SLANG_NVVM_TEXTURE_SHAPE_2D,
                0,
                {kind, 32, laneCount},
            };
            SLANG_CHECK(builder.supportsTextureOperation(fetch));
            fetch.isArray = 1;
            SLANG_CHECK(builder.supportsTextureOperation(fetch));
            fetch.shape = SLANG_NVVM_TEXTURE_SHAPE_3D;
            fetch.isArray = 0;
            SLANG_CHECK(builder.supportsTextureOperation(fetch));
        }
    }

    SlangNVVMTextureOperationDesc unsupportedFetch = {
        SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL,
        SLANG_NVVM_TEXTURE_SHAPE_1D,
        0,
        {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, 1},
    };
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedFetch));
    unsupportedFetch.shape = SLANG_NVVM_TEXTURE_SHAPE_CUBE;
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedFetch));
    unsupportedFetch.shape = SLANG_NVVM_TEXTURE_SHAPE_3D;
    unsupportedFetch.isArray = 1;
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedFetch));
    unsupportedFetch.shape = SLANG_NVVM_TEXTURE_SHAPE_2D;
    unsupportedFetch.isArray = 0;
    unsupportedFetch.elementType.laneCount = 3;
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedFetch));
    unsupportedFetch.elementType.laneCount = 1;
    unsupportedFetch.elementType.bitWidth = 16;
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedFetch));
}

SLANG_UNIT_TEST(nvvmIRBuilderEmitsVectorTextureSamples)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("vector-texture-sample"), module.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle int64Type = nullptr;
    SlangNVVMTypeHandle float2Type = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, 32, floatType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, int64Type)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getVectorType(module.module, floatType, 2, float2Type)));

    const SlangNVVMTypeHandle parameterTypes[] = {int64Type, float2Type, floatType};
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("sample2DVector"),
        function)));

    SlangNVVMValueHandle operands[3] = {};
    for (size_t parameterIndex = 0; parameterIndex < SLANG_COUNT_OF(operands); ++parameterIndex)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(
            module.module,
            function,
            parameterIndex,
            operands[parameterIndex])));
    }
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));

    const SlangNVVMTextureOperationDesc operation = {
        SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL,
        SLANG_NVVM_TEXTURE_SHAPE_2D,
        0,
        {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, 4},
    };
    SlangNVVMValueHandle result = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitTextureOperation(
        module.module,
        operation,
        operands,
        SLANG_COUNT_OF(operands),
        result)));
    SLANG_CHECK_ABORT(result != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (const auto format : formats)
    {
        ComPtr<ISlangBlob> assemblyBlob;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assemblyBlob)));
        const String assembly = _getBlobText(assemblyBlob);
        SLANG_CHECK(assembly.indexOf("@llvm.nvvm.tex.unified.2d.level.v4f32.f32") >= 0);
        SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("insertelement")) == 4);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderEmitsIntegerCoordinateTextureFetches)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("integer-texture-fetches"), module.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle int32Type = nullptr;
    SlangNVVMTypeHandle int64Type = nullptr;
    SlangNVVMTypeHandle int2Type = nullptr;
    SlangNVVMTypeHandle int3Type = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, int32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, int64Type)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getVectorType(module.module, int32Type, 2, int2Type)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getVectorType(module.module, int32Type, 3, int3Type)));

    const SlangNVVMTextureShape shapes[] = {
        SLANG_NVVM_TEXTURE_SHAPE_2D,
        SLANG_NVVM_TEXTURE_SHAPE_3D,
        SLANG_NVVM_TEXTURE_SHAPE_2D,
    };
    const uint32_t isArrays[] = {0, 0, 1};
    const SlangNVVMTypeHandle coordinateTypes[] = {int2Type, int3Type, int3Type};
    const UnownedStringSlice functionNames[] = {
        toSlice("fetch2D"),
        toSlice("fetch3D"),
        toSlice("fetch2DArray"),
    };
    const SlangNVVMValueTypeKind resultKinds[] = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    };
    const uint32_t resultLaneCounts[] = {1, 2, 4};

    for (size_t shapeIndex = 0; shapeIndex < SLANG_COUNT_OF(shapes); ++shapeIndex)
    {
        const SlangNVVMTypeHandle parameterTypes[] = {
            int64Type,
            coordinateTypes[shapeIndex],
            int32Type,
        };
        SlangNVVMTypeHandle functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            module.module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            module.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            functionNames[shapeIndex],
            function)));
        SlangNVVMValueHandle operands[3] = {};
        for (size_t parameterIndex = 0; parameterIndex < SLANG_COUNT_OF(operands); ++parameterIndex)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(
                module.module,
                function,
                parameterIndex,
                operands[parameterIndex])));
        }
        SlangNVVMBlockHandle entryBlock = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));

        for (const auto kind : resultKinds)
        {
            for (const auto laneCount : resultLaneCounts)
            {
                const SlangNVVMTextureOperationDesc operation = {
                    SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL,
                    shapes[shapeIndex],
                    isArrays[shapeIndex],
                    {kind, 32, laneCount},
                };
                SlangNVVMValueHandle result = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitTextureOperation(
                    module.module,
                    operation,
                    operands,
                    SLANG_COUNT_OF(operands),
                    result)));
                SLANG_CHECK_ABORT(result != nullptr);
            }
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    }

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    const char* shapeNames[] = {"2d", "3d", "a2d"};
    const char* dataTypeNames[] = {"f32", "s32", "u32"};
    for (const auto format : formats)
    {
        ComPtr<ISlangBlob> assemblyBlob;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assemblyBlob)));
        const String assembly = _getBlobText(assemblyBlob);
        for (const char* shapeName : shapeNames)
        {
            for (const char* dataTypeName : dataTypeNames)
            {
                StringBuilder instruction;
                instruction << "tex.level." << shapeName << ".v4." << dataTypeName << ".s32";
                SLANG_CHECK(
                    _countOccurrences(assembly.getUnownedSlice(), instruction.getUnownedSlice()) ==
                    3);
            }
        }
        SLANG_CHECK(assembly.indexOf("asm \"tex.level.") >= 0);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderEmitsIntegerSwitchAndTextureQueries)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("switch-texture-queries"), module.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle int32Type = nullptr;
    SlangNVVMTypeHandle int64Type = nullptr;
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, int32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, int64Type)));
    const SlangNVVMTypeHandle parameterTypes[] = {int64Type, int32Type};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));

    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("switchTextureQueries"),
        function)));
    SlangNVVMValueHandle texture = nullptr;
    SlangNVVMValueHandle selector = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, texture)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, selector)));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle widthBlock = nullptr;
    SlangNVVMBlockHandle heightBlock = nullptr;
    SlangNVVMBlockHandle depthBlock = nullptr;
    SlangNVVMBlockHandle defaultBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("width"), widthBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("height"), heightBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("depth"), depthBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("default"), defaultBlock)));

    SlangNVVMValueHandle caseValues[3] = {};
    for (size_t i = 0; i < SLANG_COUNT_OF(caseValues); ++i)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getIntegerConstant(module.module, int32Type, int64_t(i), caseValues[i])));
    }
    const SlangNVVMBlockHandle caseBlocks[] = {widthBlock, heightBlock, depthBlock};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SLANG_CHECK(
        builder.emitSwitch(
            module.module,
            selector,
            nullptr,
            caseBlocks,
            SLANG_COUNT_OF(caseBlocks),
            defaultBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitSwitch(
        module.module,
        selector,
        caseValues,
        caseBlocks,
        SLANG_COUNT_OF(caseBlocks),
        defaultBlock)));

    const SlangNVVMTextureOperation operations[] = {
        SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH,
        SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT,
        SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH,
    };
    const SlangNVVMTextureShape shapes[] = {
        SLANG_NVVM_TEXTURE_SHAPE_1D,
        SLANG_NVVM_TEXTURE_SHAPE_2D,
        SLANG_NVVM_TEXTURE_SHAPE_3D,
    };
    const SlangNVVMValueTypeDesc floatType = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        32,
        1,
    };
    for (size_t i = 0; i < SLANG_COUNT_OF(caseBlocks); ++i)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, caseBlocks[i])));
        const SlangNVVMTextureOperationDesc operation = {
            operations[i],
            shapes[i],
            0,
            floatType,
        };
        SlangNVVMValueHandle queryResult = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitTextureOperation(module.module, operation, &texture, 1, queryResult)));
        SLANG_CHECK_ABORT(queryResult != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, defaultBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitUnreachable(module.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(module.module, function)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (const auto format : formats)
    {
        ComPtr<ISlangBlob> assemblyBlob;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assemblyBlob)));
        const String assembly = _getBlobText(assemblyBlob);
        const UnownedStringSlice assemblySlice = assembly.getUnownedSlice();
        SLANG_CHECK(assembly.indexOf("switch i32 ") >= 0);
        SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("unreachable")) == 1);
        SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("call i32 @llvm.nvvm.txq.")) == 3);
        SLANG_CHECK(assembly.indexOf("@llvm.nvvm.txq.width(i64") >= 0);
        SLANG_CHECK(assembly.indexOf("@llvm.nvvm.txq.height(i64") >= 0);
        SLANG_CHECK(assembly.indexOf("@llvm.nvvm.txq.depth(i64") >= 0);
        SLANG_CHECK(assembly.indexOf("nounwind readnone") >= 0);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderQueriesTypedSurfaceOperations)
{
    _resetDirectNVVMFakes();
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
    NVVMIRBuilder builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));

    const SlangNVVMSurfaceOperationDesc load2D = {
        SLANG_NVVM_SURFACE_OP_LOAD,
        SLANG_NVVM_TEXTURE_SHAPE_2D,
        0,
        {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 16, 4},
        SLANG_NVVM_SURFACE_BOUNDARY_ZERO,
        SLANG_NVVM_SURFACE_STORAGE_NATIVE,
    };
    SLANG_CHECK(builder.supportsSurfaceOperation(load2D));

    SlangNVVMSurfaceOperationDesc unsupported = load2D;
    unsupported.elementType.laneCount = 3;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = load2D;
    unsupported.shape = SLANG_NVVM_TEXTURE_SHAPE_3D;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = load2D;
    unsupported.boundaryMode = SlangNVVMSurfaceBoundaryMode(1);
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    const SlangNVVMSurfaceOperationDesc formattedStore2D = {
        SLANG_NVVM_SURFACE_OP_STORE,
        SLANG_NVVM_TEXTURE_SHAPE_2D,
        0,
        {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, 4},
        SLANG_NVVM_SURFACE_BOUNDARY_ZERO,
        SLANG_NVVM_SURFACE_STORAGE_FLOAT16,
    };
    SLANG_CHECK(builder.supportsSurfaceOperation(formattedStore2D));
    unsupported = formattedStore2D;
    unsupported.elementType.bitWidth = 16;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = load2D;
    unsupported.storageFormat = SLANG_NVVM_SURFACE_STORAGE_FLOAT16;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));

    for (SlangNVVMValueTypeKind kind :
         {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
          SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
          SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER})
    {
        for (SlangNVVMTextureShape shape :
             {SLANG_NVVM_TEXTURE_SHAPE_1D,
              SLANG_NVVM_TEXTURE_SHAPE_2D,
              SLANG_NVVM_TEXTURE_SHAPE_3D})
        {
            for (uint32_t laneCount : {1u, 2u, 4u})
            {
                for (SlangNVVMSurfaceOperation operation :
                     {SLANG_NVVM_SURFACE_OP_LOAD, SLANG_NVVM_SURFACE_OP_STORE})
                {
                    SlangNVVMSurfaceOperationDesc native32 = {
                        operation,
                        shape,
                        0,
                        {kind, 32, laneCount},
                        SLANG_NVVM_SURFACE_BOUNDARY_ZERO,
                        SLANG_NVVM_SURFACE_STORAGE_NATIVE,
                    };
                    SLANG_CHECK(builder.supportsSurfaceOperation(native32));
                    if (shape == SLANG_NVVM_TEXTURE_SHAPE_2D)
                    {
                        native32.isArray = 1;
                        SLANG_CHECK(builder.supportsSurfaceOperation(native32));
                    }
                }
            }
        }
    }

    unsupported = load2D;
    unsupported.elementType = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, 3};
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = formattedStore2D;
    unsupported.shape = SLANG_NVVM_TEXTURE_SHAPE_3D;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = formattedStore2D;
    unsupported.isArray = 1;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = load2D;
    unsupported.shape = SLANG_NVVM_TEXTURE_SHAPE_CUBE;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = load2D;
    unsupported.isArray = 1;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsCurrentABIMismatches)
{
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.omitAPISymbol = true;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.acceptedABIRevision = SLANG_NVVM_BUILDER_ABI_REVISION + 1;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.api.llvmVersionMajor = 15;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderRequiresCompleteCurrentInterfaces)
{
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.omittedInterface = SLANG_NVVM_BUILDER_INTERFACE_ATOMIC_OPERATIONS;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.omittedInterface = SLANG_NVVM_BUILDER_INTERFACE_TEXTURE_OPERATIONS;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.omittedInterface = SLANG_NVVM_BUILDER_INTERFACE_SURFACE_OPERATIONS;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.omittedInterface = SLANG_NVVM_BUILDER_INTERFACE_CONSTRUCTION;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.foundation.createModule = nullptr;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.construction.emitCall = nullptr;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.construction.setFunctionParameterAttributes = nullptr;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.construction.emitVectorConstruct = nullptr;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.construction.emitByteOffsetPointer = nullptr;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.construction.emitLocalStorage = nullptr;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.valueOperations.isOperationSupported = nullptr;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.atomicOperationsAPI.emitOperation = nullptr;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderSerializesEmptyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    const SlangNVVMBuilderAPI& api = builder.getAPI();
    SLANG_CHECK(api.llvmVersionMajor == 14);
    SLANG_CHECK(api.llvmVersionMinor == 0);
    SLANG_CHECK(api.llvmVersionPatch == 6);
    SLANG_CHECK(api.nvvmIRVersionMajor == 2);
    SLANG_CHECK(api.nvvmIRVersionMinor == 0);
    SLANG_CHECK(api.pointerModel == SLANG_NVVM_POINTER_MODEL_TYPED);

    static const char kKernelName[] = "slangSlice3aEmpty";
    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _buildEmptyNVVMKernel(builder, toSlice(kKernelName), assemblyBlob, bitcodeBlob)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);

    const String assembly(UnownedStringSlice(
        static_cast<const char*>(assemblyBlob->getBufferPointer()),
        assemblyBlob->getBufferSize()));
    static const char kExpectedDataLayout[] =
        "target datalayout = \"e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
        "i64:64:64-i128:128:128-f32:32:32-f64:64:64-v16:16:16-v32:32:32-v64:64:64-"
        "v128:128:128-n16:32:64\"";
    SLANG_CHECK(assembly.indexOf(kExpectedDataLayout) >= 0);
    SLANG_CHECK(assembly.indexOf("target triple = \"nvptx64-nvidia-cuda\"") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @slangSlice3aEmpty()") >= 0);
    SLANG_CHECK(assembly.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(assembly.indexOf("!nvvm.annotations") >= 0);
    SLANG_CHECK(assembly.indexOf("void ()* @slangSlice3aEmpty") >= 0);
    SLANG_CHECK(assembly.indexOf("!\"kernel\", i32 1") >= 0);

    SLANG_CHECK(bitcodeBlob->getBufferSize() > 4);
    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, sizeof(kBitcodeMagic)) == 0);
    bool hasEmbeddedNull = false;
    const uint8_t* bitcodeBytes = static_cast<const uint8_t*>(bitcodeBlob->getBufferPointer());
    for (size_t i = 0; i < bitcodeBlob->getBufferSize(); ++i)
        hasEmbeddedNull = hasEmbeddedNull || bitcodeBytes[i] == 0;
    SLANG_CHECK(hasEmbeddedNull);
}

SLANG_UNIT_TEST(nvvmIRBuilderPreservesFunctionContracts)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("function-contracts"), scope.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionType(scope.module, voidType, nullptr, 0, functionType)));

    SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.declareFunction(
            scope.module,
            functionType,
            SlangNVVMLinkage(2),
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("invalidLinkage"),
            rejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_INTERNAL,
            SlangNVVMFunctionFlags(SLANG_NVVM_FUNCTION_FLAG_NO_INLINE << 1),
            toSlice("invalidFlags"),
            rejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);

    auto defineFunction =
        [&](const char* name, SlangNVVMLinkage linkage, SlangNVVMFunctionFlags flags)
    {
        SlangNVVMValueHandle function = nullptr;
        SlangNVVMBlockHandle entryBlock = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            linkage,
            flags,
            UnownedStringSlice(name),
            function)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(scope.module, function, toSlice("entry"), entryBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entryBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
        return function;
    };

    defineFunction(
        "internalNoInline",
        SLANG_NVVM_LINKAGE_INTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NO_INLINE);
    defineFunction("internalPlain", SLANG_NVVM_LINKAGE_INTERNAL, SLANG_NVVM_FUNCTION_FLAG_NONE);
    defineFunction(
        "exportedNoInline",
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NO_INLINE);
    SlangNVVMValueHandle kernel = defineFunction(
        "functionContractKernel",
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(scope.module, kernel)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(text.indexOf("define internal void @internalNoInline() #0") >= 0);
        SLANG_CHECK(text.indexOf("define internal void @internalPlain()") >= 0);
        SLANG_CHECK(text.indexOf("define void @exportedNoInline() #0") >= 0);
        SLANG_CHECK(text.indexOf("define void @functionContractKernel()") >= 0);
        SLANG_CHECK(text.indexOf("attributes #0 = { noinline }") >= 0);
        SLANG_CHECK(text.indexOf("invalidLinkage") < 0);
        SLANG_CHECK(text.indexOf("invalidFlags") < 0);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderPreservesByValueParameterContracts)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("by-value-parameters"), scope.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle int64Type = nullptr;
    SlangNVVMTypeHandle int16Type = nullptr;
    SlangNVVMTypeHandle aggregateType = nullptr;
    SlangNVVMTypeHandle aggregatePointerType = nullptr;
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 64, int64Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 16, int16Type)));
    const SlangNVVMTypeHandle fieldTypes[] = {int64Type, int16Type};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .getStructType(scope.module, fieldTypes, SLANG_COUNT_OF(fieldTypes), aggregateType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        scope.module,
        aggregateType,
        SLANG_NVVM_ADDRESS_SPACE_GENERIC,
        aggregatePointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(scope.module, voidType, &aggregatePointerType, 1, functionType)));

    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("byValueKernel"),
        function)));

    SLANG_CHECK(
        builder.setFunctionParameterAttributes(
            scope.module,
            function,
            1,
            SLANG_NVVM_PARAMETER_FLAG_BY_VALUE,
            aggregateType,
            8) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.setFunctionParameterAttributes(
            scope.module,
            function,
            0,
            SlangNVVMParameterFlags(SLANG_NVVM_PARAMETER_FLAG_BY_VALUE << 1),
            aggregateType,
            8) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.setFunctionParameterAttributes(
            scope.module,
            function,
            0,
            SLANG_NVVM_PARAMETER_FLAG_NONE,
            aggregateType,
            8) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.setFunctionParameterAttributes(
            scope.module,
            function,
            0,
            SLANG_NVVM_PARAMETER_FLAG_BY_VALUE,
            int64Type,
            8) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.setFunctionParameterAttributes(
            scope.module,
            function,
            0,
            SLANG_NVVM_PARAMETER_FLAG_BY_VALUE,
            aggregateType,
            3) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setFunctionParameterAttributes(
        scope.module,
        function,
        0,
        SLANG_NVVM_PARAMETER_FLAG_BY_VALUE,
        aggregateType,
        8)));
    SLANG_CHECK(
        builder.setFunctionParameterAttributes(
            scope.module,
            function,
            0,
            SLANG_NVVM_PARAMETER_FLAG_BY_VALUE,
            aggregateType,
            8) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle parameter = nullptr;
    SlangNVVMValueHandle firstFieldPointer = nullptr;
    SlangNVVMValueHandle firstField = nullptr;
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, parameter)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitStructFieldPointer(scope.module, parameter, 0, firstFieldPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitLoad(
        scope.module,
        firstFieldPointer,
        8,
        SLANG_NVVM_LOAD_FLAG_INVARIANT,
        firstField)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(scope.module, function)));

    ComPtr<ISlangBlob> llvmAssembly;
    ComPtr<ISlangBlob> nvvmAssembly;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        llvmAssembly)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        nvvmAssembly)));
    const String llvmText = _getBlobText(llvmAssembly);
    const String nvvmText = _getBlobText(nvvmAssembly);
    SLANG_CHECK(
        llvmText.indexOf("{ i64, i16 }* byval({ i64, i16 }) align 8 %slangParameter0") >= 0);
    SLANG_CHECK(nvvmText.indexOf("{ i64, i16 }* byval align 8 %slangParameter0") >= 0);
    SLANG_CHECK(nvvmText.indexOf("byval(") < 0);
    SLANG_CHECK(
        nvvmText.indexOf(
            "getelementptr inbounds { i64, i16 }, { i64, i16 }* %slangParameter0, i32 0, i32 0") >=
        0);
    SLANG_CHECK(nvvmText.indexOf("load i64") >= 0);
    SLANG_CHECK(nvvmText.indexOf("!invariant.load") >= 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsLocalAggregatePointerCalls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule scope;
    ScopedNVVMBuilderModule foreignScope;
    scope.builder = &builder;
    foreignScope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("local-aggregate-pointer-calls"), scope.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("foreign-local-types"), foreignScope.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignScope.module, 32, foreignIntegerType)));
    SlangNVVMTypeHandle aggregateType = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getStructType(scope.module, &integerType, 1, aggregateType)));
    SlangNVVMTypeHandle aggregatePointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        scope.module,
        aggregateType,
        SLANG_NVVM_ADDRESS_SPACE_GENERIC,
        aggregatePointerType)));

    SlangNVVMTypeHandle helperType = nullptr;
    SlangNVVMTypeHandle kernelType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(scope.module, voidType, &aggregatePointerType, 1, helperType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionType(scope.module, voidType, nullptr, 0, kernelType)));
    SlangNVVMValueHandle helper = nullptr;
    SlangNVVMValueHandle kernel = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        helperType,
        SLANG_NVVM_LINKAGE_INTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("mutateAggregate"),
        helper)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("localAggregateKernel"),
        kernel)));

    SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder
            .emitLocalStorage(scope.module, aggregateType, 4, toSlice("beforeBlock"), rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);

    SlangNVVMBlockHandle helperBlock = nullptr;
    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, helper, toSlice("entry"), helperBlock)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, kernel, toSlice("entry"), kernelBlock)));

    SlangNVVMValueHandle helperParameter = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, helper, 0, helperParameter)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, helperBlock)));
    SlangNVVMValueHandle fieldPointer = nullptr;
    SlangNVVMValueHandle fieldValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitStructFieldPointer(scope.module, helperParameter, 0, fieldPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitLoad(scope.module, fieldPointer, 4, SLANG_NVVM_LOAD_FLAG_NONE, fieldValue)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(scope.module, fieldValue, fieldPointer, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, kernelBlock)));
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder
            .emitLocalStorage(scope.module, foreignIntegerType, 4, toSlice("foreign"), rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitLocalStorage(scope.module, aggregateType, 3, toSlice("misaligned"), rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);

    SlangNVVMValueHandle local = nullptr;
    SlangNVVMValueHandle initialField = nullptr;
    SlangNVVMValueHandle initialValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitLocalStorage(scope.module, aggregateType, 4, toSlice("slangLocal"), local)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerConstant(scope.module, integerType, 7, initialField)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .emitAggregateConstruct(scope.module, aggregateType, &initialField, 1, initialValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(scope.module, initialValue, local, 4)));
    SlangNVVMValueHandle call = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitCall(scope.module, helper, &local, 1, call)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(scope.module, kernel)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(text.indexOf("%slangLocal = alloca { i32 }, align 4") >= 0);
        SLANG_CHECK(text.indexOf("call void @mutateAggregate({ i32 }* %slangLocal)") >= 0);
        SLANG_CHECK(text.indexOf("getelementptr inbounds { i32 }") >= 0);
        SLANG_CHECK(text.indexOf("store { i32 }") >= 0);
        SLANG_CHECK(text.indexOf("!nvvm.annotations") >= 0);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsRawViewValueCalls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("raw-view-value-calls"), scope.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle countType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 64, countType)));

    SlangNVVMTypeHandle dataPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        scope.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        dataPointerType)));
    const SlangNVVMTypeHandle viewFieldTypes[] = {dataPointerType, countType};
    SlangNVVMTypeHandle viewType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getStructType(
        scope.module,
        viewFieldTypes,
        SLANG_COUNT_OF(viewFieldTypes),
        viewType)));

    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(scope.module, voidType, &viewType, 1, functionType)));
    SlangNVVMValueHandle helper = nullptr;
    SlangNVVMValueHandle caller = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_INTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("consumeRawView"),
        helper)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("forwardRawView"),
        caller)));

    SlangNVVMBlockHandle helperBlock = nullptr;
    SlangNVVMBlockHandle callerBlock = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, helper, toSlice("entry"), helperBlock)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, caller, toSlice("entry"), callerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, helperBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));

    SlangNVVMValueHandle callerView = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, caller, 0, callerView)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, callerBlock)));
    SlangNVVMValueHandle call = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitCall(scope.module, helper, &callerView, 1, call)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(
            text.indexOf("define internal void @consumeRawView({ i32 addrspace(1)*, i64 }") >= 0);
        SLANG_CHECK(text.indexOf("define void @forwardRawView({ i32 addrspace(1)*, i64 }") >= 0);
        SLANG_CHECK(text.indexOf("call void @consumeRawView({ i32 addrspace(1)*, i64 }") >= 0);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsUnknownOperationsWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("unknown-value-operations"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _populateEmptyNVVMKernel(builder, scope.module, toSlice("unknownOperations"))));

    ComPtr<ISlangBlob> before;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, before)));

    SlangNVVMValueHandle output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerUnary(scope.module, SlangNVVMValueOperation(99), nullptr, output) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerBinaryOperation(
            scope.module,
            SlangNVVMValueOperation(99),
            nullptr,
            nullptr,
            output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCompare(
            scope.module,
            SlangNVVMValueOperation(99),
            nullptr,
            nullptr,
            output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitFloatingBinary(
            scope.module,
            SlangNVVMValueOperation(99),
            nullptr,
            nullptr,
            output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitFloatingCompare(
            scope.module,
            SlangNVVMValueOperation(99),
            nullptr,
            nullptr,
            output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);

    const SlangNVVMBuilderValueOperationsAPI* valueAPI = builder.getValueOperationsAPI();
    SLANG_CHECK_ABORT(valueAPI != nullptr);
    const SlangNVVMValueTypeDesc signedI32 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        32,
        1,
    };
    const SlangNVVMValueTypeDesc operandTypes[] = {signedI32, signedI32};
    SlangNVVMValueOperationDesc operationDesc = {
        SlangNVVMValueOperation(SLANG_NVVM_VALUE_OPERATION_COUNT),
        signedI32,
        operandTypes,
        SLANG_COUNT_OF(operandTypes),
    };
    uint32_t supported = 1;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(valueAPI->isOperationSupported(&operationDesc, &supported)));
    SLANG_CHECK(supported == 0);
    output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    const SlangNVVMValueHandle operandValues[] = {nullptr, nullptr};
    SLANG_CHECK(
        valueAPI->emitOperation(
            scope.module,
            &operationDesc,
            operandValues,
            SLANG_COUNT_OF(operandValues),
            &output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);

    ComPtr<ISlangBlob> after;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, after)));
    SLANG_CHECK_ABORT(before != nullptr && after != nullptr);
    SLANG_CHECK(before->getBufferSize() == after->getBufferSize());
    SLANG_CHECK(
        ::memcmp(before->getBufferPointer(), after->getBufferPointer(), before->getBufferSize()) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsAndValidatesCUDAExecutionOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    ScopedNVVMBuilderModule foreignScope;
    scope.builder = &builder;
    foreignScope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("cuda-execution-operations"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("cuda-execution-operations-foreign"), foreignScope.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle i32Type = nullptr;
    SlangNVVMTypeHandle uint3Type = nullptr;
    SlangNVVMTypeHandle foreignI32Type = nullptr;
    SlangNVVMTypeHandle foreignUInt3Type = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, i32Type)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignScope.module, 32, foreignI32Type)));

    SlangNVVMTypeHandle rejectedType = reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
    SLANG_CHECK(builder.getVectorType(nullptr, i32Type, 3, rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getVectorType(scope.module, foreignI32Type, 3, rejectedType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getVectorType(scope.module, i32Type, 1, rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getVectorType(scope.module, i32Type, 5, rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    SLANG_CHECK(
        builder.getConstructionAPI()->getVectorType(scope.module, i32Type, 3, nullptr) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVectorType(scope.module, i32Type, 3, uint3Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getVectorType(foreignScope.module, foreignI32Type, 3, foreignUInt3Type)));

    const SlangNVVMTypeHandle parameterTypes[] = {i32Type};
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        scope.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("cudaExecutionOperations"),
        function)));
    SlangNVVMValueHandle scalarParameter = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, scalarParameter)));
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), entryBlock)));

    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SlangNVVMValueHandle foreignFunction = nullptr;
    SlangNVVMValueHandle foreignVector = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignScope.module, foreignVoidType)));
    const SlangNVVMTypeHandle foreignParameterTypes[] = {foreignUInt3Type};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignScope.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignScope.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignCUDAExecutionOperations"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignScope.module, foreignFunction, 0, foreignVector)));
    SlangNVVMValueHandle foreignIndex = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(foreignScope.module, foreignI32Type, 0, foreignIndex)));

    const SlangNVVMValueOperation executionOperations[] = {
        SLANG_NVVM_VALUE_OP_THREAD_INDEX,
        SLANG_NVVM_VALUE_OP_BLOCK_INDEX,
        SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS,
        SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS,
    };
    auto getOperation = [](SlangNVVMValueOperation operation)
    {
        for (const NVVMSemantics::CatalogEntry& entry : NVVMSemantics::kCatalog)
        {
            if (entry.operation == operation)
                return NVVMSemantics::getOperationDesc(entry);
        }
        SLANG_UNEXPECTED("missing CUDA execution catalog operation");
    };
    const SlangNVVMValueOperationDesc barrierOperation =
        getOperation(SLANG_NVVM_VALUE_OP_WORKGROUP_BARRIER);
    const SlangNVVMValueOperationDesc deviceBarrierOperation =
        getOperation(SLANG_NVVM_VALUE_OP_DEVICE_MEMORY_BARRIER);

    SlangNVVMValueHandle rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitValueOperation(
            scope.module,
            getOperation(SLANG_NVVM_VALUE_OP_THREAD_INDEX),
            nullptr,
            0,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entryBlock)));
    SLANG_CHECK(
        builder.getValueOperationsAPI()
            ->emitOperation(scope.module, &barrierOperation, nullptr, 0, nullptr) ==
        SLANG_E_INVALID_ARG);

    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder
            .emitSequentialElementExtract(scope.module, nullptr, scalarParameter, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitSequentialElementExtract(
            scope.module,
            scalarParameter,
            scalarParameter,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitSequentialElementExtract(
            scope.module,
            foreignVector,
            scalarParameter,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    for (SlangNVVMValueOperation executionOperation : executionOperations)
    {
        SlangNVVMValueHandle vector = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueOperation(
            scope.module,
            getOperation(executionOperation),
            nullptr,
            0,
            vector)));
        SlangNVVMValueHandle axisConstants[4] = {};
        for (uint32_t axis = 0; axis < 4; ++axis)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getIntegerConstant(scope.module, i32Type, axis, axisConstants[axis])));
        }
        for (uint32_t axis = 0; axis < 3; ++axis)
        {
            SlangNVVMValueHandle component = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitSequentialElementExtract(
                scope.module,
                vector,
                axisConstants[axis],
                component)));
        }
        rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitSequentialElementExtract(scope.module, vector, nullptr, rejectedValue) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
        rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder
                .emitSequentialElementExtract(scope.module, vector, foreignIndex, rejectedValue) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
        rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitSequentialElementExtract(
                scope.module,
                vector,
                axisConstants[3],
                rejectedValue) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
    }

    SlangNVVMValueHandle barrierValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitValueOperation(scope.module, barrierOperation, nullptr, 0, barrierValue)));
    SLANG_CHECK(barrierValue == nullptr);
    barrierValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .emitValueOperation(scope.module, deviceBarrierOperation, nullptr, 0, barrierValue)));
    SLANG_CHECK(barrierValue == nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(scope.module, function)));

    for (SlangNVVMValueOperation executionOperation : executionOperations)
    {
        rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitValueOperation(
                scope.module,
                getOperation(executionOperation),
                nullptr,
                0,
                rejectedValue) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
    }
    barrierValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitValueOperation(scope.module, barrierOperation, nullptr, 0, barrierValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(barrierValue == nullptr);

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    static const char* kIntrinsicNames[] = {
        "llvm.nvvm.read.ptx.sreg.tid.",
        "llvm.nvvm.read.ptx.sreg.ctaid.",
        "llvm.nvvm.read.ptx.sreg.ntid.",
        "llvm.nvvm.read.ptx.sreg.nctaid.",
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const String text = _getBlobText(assembly);
        for (const char* intrinsicName : kIntrinsicNames)
        {
            SLANG_CHECK(
                _countOccurrences(text.getUnownedSlice(), UnownedStringSlice(intrinsicName)) == 6);
        }
        SLANG_CHECK(
            _countOccurrences(text.getUnownedSlice(), toSlice("call void @llvm.nvvm.barrier0()")) ==
            1);
        SLANG_CHECK(
            _countOccurrences(
                text.getUnownedSlice(),
                toSlice("call void @llvm.nvvm.membar.gl()")) == 1);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("extractelement")) == 12);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("ret void")) == 1);
        if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
        {
            SLANG_CHECK(text.indexOf("= { nounwind readnone speculatable }") >= 0);
        }
        else
        {
            SLANG_CHECK(text.indexOf("= { nounwind readnone speculatable }") < 0);
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderConstructsAndConvertsIntegerVectors)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule scope;
    ScopedNVVMBuilderModule foreignScope;
    scope.builder = &builder;
    foreignScope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("integer-vectors"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("integer-vectors-foreign"), foreignScope.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle i32Type = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle vectorType = nullptr;
    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignI32Type = nullptr;
    SlangNVVMTypeHandle foreignVectorType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, i32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 32, floatType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVectorType(scope.module, i32Type, 2, vectorType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignScope.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignScope.module, 32, foreignI32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getVectorType(foreignScope.module, foreignI32Type, 2, foreignVectorType)));

    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SlangNVVMValueHandle foreignFunction = nullptr;
    SlangNVVMValueHandle foreignValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignScope.module,
        foreignVoidType,
        &foreignI32Type,
        1,
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignScope.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignIntegerVectorSource"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignScope.module, foreignFunction, 0, foreignValue)));

    const SlangNVVMTypeHandle parameterTypes[] = {i32Type, i32Type, floatType};
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        scope.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("integerVectors"),
        function)));
    SlangNVVMValueHandle first = nullptr;
    SlangNVVMValueHandle second = nullptr;
    SlangNVVMValueHandle wrongType = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, first)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, second)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, wrongType)));
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entryBlock)));

    const SlangNVVMValueHandle elements[] = {first, second};
    const SlangNVVMValueHandle wrongElements[] = {first, wrongType};
    const SlangNVVMValueHandle unavailableElements[] = {first, foreignValue};
    SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitVectorConstruct(nullptr, vectorType, elements, 2, rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitVectorConstruct(scope.module, i32Type, elements, 2, rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitVectorConstruct(scope.module, foreignVectorType, elements, 2, rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitVectorConstruct(scope.module, vectorType, nullptr, 2, rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitVectorConstruct(scope.module, vectorType, elements, 1, rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitVectorConstruct(scope.module, vectorType, wrongElements, 2, rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitVectorConstruct(scope.module, vectorType, unavailableElements, 2, rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    SLANG_CHECK(
        builder.getConstructionAPI()
            ->emitVectorConstruct(scope.module, vectorType, elements, 2, nullptr) ==
        SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle vector = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitVectorConstruct(scope.module, vectorType, elements, 2, vector)));
    SlangNVVMValueHandle firstExtract = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitSequentialElementExtract(scope.module, vector, first, firstExtract)));

    const SlangNVVMValueTypeDesc unsignedI32x2 = {
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
        32,
        2,
    };
    const SlangNVVMValueTypeDesc signedI32x2 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        32,
        2,
    };
    const SlangNVVMValueOperationDesc convertOperation = {
        SLANG_NVVM_VALUE_OP_INTEGER_CONVERT,
        signedI32x2,
        &unsignedI32x2,
        1,
    };
    SlangNVVMValueHandle converted = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitValueOperation(scope.module, convertOperation, &vector, 1, converted)));
    SlangNVVMValueHandle secondExtract = nullptr;
    SlangNVVMValueHandle secondIndex = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerConstant(scope.module, i32Type, 1, secondIndex)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitSequentialElementExtract(scope.module, converted, secondIndex, secondExtract)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(scope.module, function)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("insertelement")) == 2);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("extractelement")) == 2);
        SLANG_CHECK(text.indexOf("insertelement <2 x i32> undef") >= 0);
        SLANG_CHECK(text.indexOf("poison") < 0);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsAndValidatesSharedGlobalStorage)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    ScopedNVVMBuilderModule foreignScope;
    scope.builder = &builder;
    foreignScope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("shared-global-storage"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("shared-global-storage-foreign"), foreignScope.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle i32Type = nullptr;
    SlangNVVMTypeHandle arrayType = nullptr;
    SlangNVVMTypeHandle foreignI32Type = nullptr;
    SlangNVVMTypeHandle foreignArrayType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, i32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getArrayType(scope.module, i32Type, 64, arrayType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignScope.module, 32, foreignI32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getArrayType(foreignScope.module, foreignI32Type, 64, foreignArrayType)));

    SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.declareGlobalStorage(
            nullptr,
            arrayType,
            SLANG_NVVM_LINKAGE_INTERNAL,
            SLANG_NVVM_ADDRESS_SPACE_SHARED,
            4,
            toSlice("rejectedNullModule"),
            rejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.declareGlobalStorage(
            scope.module,
            foreignArrayType,
            SLANG_NVVM_LINKAGE_INTERNAL,
            SLANG_NVVM_ADDRESS_SPACE_SHARED,
            4,
            toSlice("rejectedForeignType"),
            rejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.declareGlobalStorage(
            scope.module,
            arrayType,
            SlangNVVMLinkage(2),
            SLANG_NVVM_ADDRESS_SPACE_SHARED,
            4,
            toSlice("rejectedLinkage"),
            rejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.declareGlobalStorage(
            scope.module,
            arrayType,
            SLANG_NVVM_LINKAGE_INTERNAL,
            SlangNVVMAddressSpace(2),
            4,
            toSlice("rejectedAddressSpace"),
            rejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.declareGlobalStorage(
            scope.module,
            arrayType,
            SLANG_NVVM_LINKAGE_INTERNAL,
            SLANG_NVVM_ADDRESS_SPACE_SHARED,
            3,
            toSlice("rejectedAlignment"),
            rejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);

    SlangNVVMValueHandle storage = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareGlobalStorage(
        scope.module,
        arrayType,
        SLANG_NVVM_LINKAGE_INTERNAL,
        SLANG_NVVM_ADDRESS_SPACE_SHARED,
        4,
        toSlice("sharedValues"),
        storage)));
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.declareGlobalStorage(
            scope.module,
            arrayType,
            SLANG_NVVM_LINKAGE_INTERNAL,
            SLANG_NVVM_ADDRESS_SPACE_SHARED,
            4,
            toSlice("sharedValues"),
            rejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);

    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionType(scope.module, voidType, nullptr, 0, functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("sharedGlobalStorage"),
        function)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entryBlock)));
    SlangNVVMValueHandle index = nullptr;
    SlangNVVMValueHandle elementPointer = nullptr;
    SlangNVVMValueHandle loaded = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerConstant(scope.module, i32Type, 7, index)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitSequentialElementPointer(scope.module, storage, index, elementPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(scope.module, index, elementPointer, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitLoad(scope.module, elementPointer, 4, SLANG_NVVM_LOAD_FLAG_NONE, loaded)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(scope.module, function)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(
            text.indexOf(
                "@sharedValues = internal addrspace(3) global [64 x i32] undef, align 4") >= 0);
        SLANG_CHECK(text.indexOf("rejectedNullModule") < 0);
        SLANG_CHECK(text.indexOf("rejectedForeignType") < 0);
        SLANG_CHECK(text.indexOf("rejectedLinkage") < 0);
        SLANG_CHECK(text.indexOf("rejectedAddressSpace") < 0);
        SLANG_CHECK(text.indexOf("rejectedAlignment") < 0);
        // LLVM folds this constant-index address to a constant expression and prints it once at
        // each load/store use in both dialects.
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("getelementptr")) == 2);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("store i32 7")) == 1);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("load i32")) == 1);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsConventionalGlobalParameterStorage)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule module;
    ScopedNVVMBuilderModule foreignModule;
    module.builder = &builder;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("conventional-global-parameters"), module.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(
        toSlice("conventional-global-parameters-foreign"),
        foreignModule.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle countType = nullptr;
    SlangNVVMTypeHandle dataPointerType = nullptr;
    SlangNVVMTypeHandle resourceType = nullptr;
    SlangNVVMTypeHandle foreignIntegerType = nullptr;
    SlangNVVMTypeHandle foreignCountType = nullptr;
    SlangNVVMTypeHandle foreignDataPointerType = nullptr;
    SlangNVVMTypeHandle foreignResourceType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, countType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        dataPointerType)));
    const SlangNVVMTypeHandle resourceFieldTypes[] = {dataPointerType, countType};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getStructType(
        module.module,
        resourceFieldTypes,
        SLANG_COUNT_OF(resourceFieldTypes),
        resourceType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 64, foreignCountType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignDataPointerType)));
    const SlangNVVMTypeHandle foreignResourceFieldTypes[] = {
        foreignDataPointerType,
        foreignCountType,
    };
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getStructType(
        foreignModule.module,
        foreignResourceFieldTypes,
        SLANG_COUNT_OF(foreignResourceFieldTypes),
        foreignResourceType)));

    SlangNVVMTypeHandle rejectedType = reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getStructType(module.module, nullptr, 1, rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getStructType(module.module, &foreignResourceType, 1, rejectedType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);

    const SlangNVVMTypeHandle fieldTypes[] = {resourceType};
    SlangNVVMTypeHandle parameterStructType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getStructType(
        module.module,
        fieldTypes,
        SLANG_COUNT_OF(fieldTypes),
        parameterStructType)));

    SlangNVVMValueHandle globalParameters = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareGlobalStorage(
        module.module,
        parameterStructType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_ADDRESS_SPACE_CONSTANT,
        8,
        toSlice("SLANG_globalParams"),
        globalParameters)));

    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(module.module, voidType, nullptr, 0, functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("conventionalGlobalParameters"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));

    auto expectRejectedField = [&](SlangNVVMValueHandle base, uint32_t fieldIndex)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitStructFieldPointer(module.module, base, fieldIndex, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    expectRejectedField(nullptr, 0);
    expectRejectedField(globalParameters, 1);

    SlangNVVMValueHandle fieldPointer = nullptr;
    SlangNVVMValueHandle buffer = nullptr;
    SlangNVVMValueHandle index = nullptr;
    SlangNVVMValueHandle elementPointer = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitStructFieldPointer(module.module, globalParameters, 0, fieldPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitLoad(module.module, fieldPointer, 8, SLANG_NVVM_LOAD_FLAG_NONE, buffer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 0, index)));
    SlangNVVMValueHandle dataPointer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitAggregateElementExtract(module.module, buffer, 0, dataPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitPointerOffset(module.module, dataPointer, index, elementPointer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 42, value)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, value, elementPointer, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(module.module, function)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(
            text.indexOf("@SLANG_globalParams = addrspace(4) global { { i32 addrspace(1)*, i64 } } "
                         "undef, align 8") >= 0);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("getelementptr")) == 2);
        SLANG_CHECK(
            _countOccurrences(text.getUnownedSlice(), toSlice("load { i32 addrspace(1)*, i64 }")) ==
            1);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("extractvalue")) == 1);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("store i32 42")) == 1);
        SLANG_CHECK(text.indexOf("!nvvm.annotations") >= 0);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidFloat32Operations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule scope;
    ScopedNVVMBuilderModule foreignScope;
    scope.builder = &builder;
    foreignScope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-float32-main"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-float32-foreign"), foreignScope.module)));

    SlangNVVMTypeHandle invalidType = reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
    SLANG_CHECK(builder.getFloatingPointType(scope.module, 80, invalidType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidType == nullptr);

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle i32Type = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 32, floatType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
    const SlangNVVMTypeHandle parameterTypes[] = {floatType, floatType, integerType};
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        scope.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("invalidFloat32"),
        function)));
    SlangNVVMValueHandle left = nullptr;
    SlangNVVMValueHandle right = nullptr;
    SlangNVVMValueHandle integer = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, left)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, right)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, integer)));
    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle laterBlock = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("later"), laterBlock)));

    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignFloatType = nullptr;
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SlangNVVMValueHandle foreignFunction = nullptr;
    SlangNVVMValueHandle foreignValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignScope.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFloatingPointType(foreignScope.module, 32, foreignFloatType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignScope.module,
        foreignVoidType,
        &foreignFloatType,
        1,
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignScope.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignFloat32"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignScope.module, foreignFunction, 0, foreignValue)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, laterBlock)));
    SlangNVVMValueHandle laterValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .emitFloatingBinary(scope.module, SLANG_NVVM_VALUE_OP_ADD, left, right, laterValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entryBlock)));
    const SlangNVVMValueHandle invalidOperands[][2] = {
        {integer, integer},
        {left, integer},
        {left, foreignValue},
        {laterValue, left},
        {nullptr, right},
    };
    for (const auto& operands : invalidOperands)
    {
        SlangNVVMValueHandle output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitFloatingBinary(
                scope.module,
                SLANG_NVVM_VALUE_OP_ADD,
                operands[0],
                operands[1],
                output) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(output == nullptr);

        output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitFloatingCompare(
                scope.module,
                SLANG_NVVM_VALUE_OP_EQUAL,
                operands[0],
                operands[1],
                output) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(output == nullptr);
    }

    const SlangNVVMValueHandle invalidUnaryOperands[] = {
        integer,
        foreignValue,
        laterValue,
        nullptr,
    };
    for (SlangNVVMValueHandle operand : invalidUnaryOperands)
    {
        SlangNVVMValueHandle output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitFloatingUnary(scope.module, SLANG_NVVM_VALUE_OP_NEGATE, operand, output) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(output == nullptr);
    }

    SlangNVVMValueHandle sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitFloatingBinary(scope.module, SLANG_NVVM_VALUE_OP_ADD, left, right, sum)));
    SlangNVVMValueHandle negated = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitFloatingUnary(scope.module, SLANG_NVVM_VALUE_OP_NEGATE, left, negated)));
    SlangNVVMValueHandle equal = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitFloatingCompare(scope.module, SLANG_NVVM_VALUE_OP_EQUAL, left, right, equal)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    SlangNVVMValueHandle output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitFloatingBinary(scope.module, SLANG_NVVM_VALUE_OP_ADD, left, right, output) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitFloatingCompare(scope.module, SLANG_NVVM_VALUE_OP_EQUAL, left, right, output) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitFloatingUnary(scope.module, SLANG_NVVM_VALUE_OP_NEGATE, left, output) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);

    ComPtr<ISlangBlob> assemblyBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    const UnownedStringSlice assembly(
        static_cast<const char*>(assemblyBlob->getBufferPointer()),
        assemblyBlob->getBufferSize());
    SLANG_CHECK(_countOccurrences(assembly, toSlice("fadd float")) == 2);
    SLANG_CHECK(_countOccurrences(assembly, toSlice("fsub float -0.000000e+00,")) == 1);
    SLANG_CHECK(_countOccurrences(assembly, toSlice("fcmp oeq float")) == 1);
}

static void _runNVVMIRBuilderBuildsFloat32ArithmeticKernel(
    UnitTestContext* unitTestContext,
    NVVMFloat32ArithmeticTestOperation testOperation)
{
    const NVVMFloat32ArithmeticTestCase& testCase =
        _getNVVMFloat32ArithmeticTestCase(testOperation);
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    StringBuilder moduleName;
    moduleName << testCase.diagnosticName << "-module";
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(moduleName.getUnownedSlice(), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateFloat32ArithmeticKernel(
        builder,
        scope.module,
        UnownedStringSlice(testCase.kernelName),
        testCase.operandCount,
        testCase.operation)));

    ComPtr<ISlangBlob> llvmAssembly;
    ComPtr<ISlangBlob> nvvmAssembly;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        llvmAssembly)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        nvvmAssembly)));
    SLANG_CHECK_ABORT(llvmAssembly != nullptr && nvvmAssembly != nullptr);

    const String llvmText(UnownedStringSlice(
        static_cast<const char*>(llvmAssembly->getBufferPointer()),
        llvmAssembly->getBufferSize()));
    const String nvvmText(UnownedStringSlice(
        static_cast<const char*>(nvvmAssembly->getBufferPointer()),
        nvvmAssembly->getBufferSize()));
    const String texts[] = {llvmText, nvvmText};
    for (Index textIndex = 0; textIndex < SLANG_COUNT_OF(texts); ++textIndex)
    {
        const String& text = texts[textIndex];
        StringBuilder signature;
        signature << "define void @" << testCase.kernelName << "(float addrspace(1)*";
        SLANG_CHECK(text.indexOf(signature.getUnownedSlice()) >= 0);
        for (const auto& arithmeticCase : kNVVMFloat32ArithmeticTestCases)
        {
            StringBuilder instruction;
            instruction << arithmeticCase.llvmOpcode << " float";
            Index expectedCount = &arithmeticCase == &testCase ? 1 : 0;
            if (testOperation == NVVMFloat32ArithmeticTestOperation::Negate)
            {
                if (arithmeticCase.testOperation == NVVMFloat32ArithmeticTestOperation::Negate)
                    expectedCount = 0;
                else if (
                    arithmeticCase.testOperation == NVVMFloat32ArithmeticTestOperation::Subtract)
                    expectedCount = 1;
            }
            SLANG_CHECK(
                _countOccurrences(text.getUnownedSlice(), instruction.getUnownedSlice()) ==
                expectedCount);
        }
        SLANG_CHECK(
            _countOccurrences(text.getUnownedSlice(), toSlice("fsub float -0.000000e+00,")) ==
            (testOperation == NVVMFloat32ArithmeticTestOperation::Negate ? 1 : 0));
        SLANG_CHECK(text.indexOf("store float") >= 0);
        SLANG_CHECK(text.indexOf("align 4") >= 0);
        SLANG_CHECK(text.indexOf("fast") < 0);
    }
    SLANG_CHECK(nvvmText.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(nvvmText.indexOf("!\"kernel\", i32 1") >= 0);
}

#define NVVM_FLOAT32_ARITHMETIC_BUILDER_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                     \
    {                                                         \
        _runNVVMIRBuilderBuildsFloat32ArithmeticKernel(       \
            unitTestContext,                                  \
            NVVMFloat32ArithmeticTestOperation::OPERATION);   \
    }

NVVM_FLOAT32_ARITHMETIC_BUILDER_TEST(nvvmIRBuilderBuildsFloat32AddKernel, Add)
NVVM_FLOAT32_ARITHMETIC_BUILDER_TEST(nvvmIRBuilderBuildsFloat32SubtractKernel, Subtract)
NVVM_FLOAT32_ARITHMETIC_BUILDER_TEST(nvvmIRBuilderBuildsFloat32MultiplyKernel, Multiply)
NVVM_FLOAT32_ARITHMETIC_BUILDER_TEST(nvvmIRBuilderBuildsFloat32DivideKernel, Divide)
NVVM_FLOAT32_ARITHMETIC_BUILDER_TEST(nvvmIRBuilderBuildsFloat32NegateKernel, Negate)

#undef NVVM_FLOAT32_ARITHMETIC_BUILDER_TEST

static void _runNVVMIRBuilderBuildsFloat32ComparisonKernel(
    UnitTestContext* unitTestContext,
    NVVMFloat32ComparisonTestOperation testOperation)
{
    const NVVMFloat32ComparisonTestCase& testCase =
        _getNVVMFloat32ComparisonTestCase(testOperation);
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(UnownedStringSlice(testCase.diagnosticName), scope.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_populateFloat32ComparisonKernel(builder, scope.module, testCase)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        StringBuilder signature;
        signature << "define void @" << testCase.kernelName << "(i32 addrspace(1)*";
        SLANG_CHECK(text.indexOf(signature.getUnownedSlice()) >= 0);
        for (const auto& comparisonCase : kNVVMFloat32ComparisonTestCases)
        {
            StringBuilder instruction;
            instruction << comparisonCase.llvmOpcode << " float";
            SLANG_CHECK(
                _countOccurrences(text, instruction.getUnownedSlice()) ==
                (&comparisonCase == &testCase ? 1 : 0));
        }
        SLANG_CHECK(_countOccurrences(text, toSlice("fcmp ")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 2);
        SLANG_CHECK(text.indexOf(toSlice("br i1")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("align 4")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fast")) < 0);
    }
}

#define NVVM_FLOAT32_COMPARISON_BUILDER_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                     \
    {                                                         \
        _runNVVMIRBuilderBuildsFloat32ComparisonKernel(       \
            unitTestContext,                                  \
            NVVMFloat32ComparisonTestOperation::OPERATION);   \
    }

NVVM_FLOAT32_COMPARISON_BUILDER_TEST(nvvmIRBuilderBuildsFloat32EqualKernel, OrderedEqual)
NVVM_FLOAT32_COMPARISON_BUILDER_TEST(nvvmIRBuilderBuildsFloat32NotEqualKernel, UnorderedNotEqual)
NVVM_FLOAT32_COMPARISON_BUILDER_TEST(
    nvvmIRBuilderBuildsFloat32GreaterThanKernel,
    OrderedGreaterThan)
NVVM_FLOAT32_COMPARISON_BUILDER_TEST(nvvmIRBuilderBuildsFloat32LessEqualKernel, OrderedLessEqual)
NVVM_FLOAT32_COMPARISON_BUILDER_TEST(
    nvvmIRBuilderBuildsFloat32GreaterEqualKernel,
    OrderedGreaterEqual)
NVVM_FLOAT32_COMPARISON_BUILDER_TEST(nvvmIRBuilderBuildsFloat32LessThanKernel, OrderedLessThan)

#undef NVVM_FLOAT32_COMPARISON_BUILDER_TEST

SLANG_UNIT_TEST(nvvmIRBuilderBuildsFloat32ConstantKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("float32-constant-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateFloat32ConstantKernel(
        builder,
        scope.module,
        toSlice("float32Constant"),
        UINT32_C(0x3fc00000))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define void @float32Constant(float addrspace(1)*")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("store float 1.500000e+00")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("align 4")) == 1);
        SLANG_CHECK(text.indexOf(toSlice("fadd float")) < 0);
    }

    SlangNVVMValueHandle value = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getFloatingPointConstant(nullptr, nullptr, 32, UINT64_C(0x3fc00000), value) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(value == nullptr);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsFloat32PhiKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("float32-phi-module"), scope.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_populateFloat32PhiKernel(builder, scope.module, toSlice("float32Phi"))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define void @float32Phi(float addrspace(1)*")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("phi float")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store float")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("align 4")) == 1);
        SLANG_CHECK(text.indexOf(toSlice("fadd float")) < 0);
    }

    SlangNVVMValueHandle value = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(builder.emitPhi(scope.module, nullptr, nullptr, value) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(value == nullptr);
    SLANG_CHECK(
        builder.addPhiIncoming(scope.module, nullptr, nullptr, nullptr) == SLANG_E_INVALID_ARG);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsFloat32FunctionKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("float32-function-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateFloat32FunctionKernel(
        builder,
        scope.module,
        toSlice("float32Function"),
        toSlice("addFloat32"))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define float @addFloat32(float")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("define void @float32Function(float addrspace(1)*")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("call float @addFloat32")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret float")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("fadd float")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store float")) == 1);
        SLANG_CHECK(text.indexOf(toSlice("align 4")) >= 0);
    }

    SlangNVVMValueHandle value = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(builder.emitCall(scope.module, nullptr, nullptr, 0, value) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(value == nullptr);
    SLANG_CHECK(builder.emitValueReturn(scope.module, nullptr) == SLANG_E_INVALID_ARG);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveLaneIndexKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("wave-lane-index-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWaveLaneIndexKernel(
        builder,
        scope.module,
        toSlice("waveLaneIndex"),
        toSlice("readWaveLaneIndex"))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define i32 @readWaveLaneIndex()")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("define void @waveLaneIndex(i32 addrspace(1)*")) >= 0);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call i32 @llvm.nvvm.read.ptx.sreg.laneid()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @readWaveLaneIndex()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret i32")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 1);
    }

    const SlangNVVMValueOperationDesc invalidOperation = {
        SlangNVVMValueOperation(99),
        NVVMSemantics::kSignedI32,
        nullptr,
        0,
    };
    SlangNVVMValueHandle value = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitValueOperation(scope.module, invalidOperation, nullptr, 0, value) ==
        SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(value == nullptr);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveLaneCountKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("wave-lane-count-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWaveLaneCountKernel(
        builder,
        scope.module,
        toSlice("waveLaneCount"),
        toSlice("readWaveLaneIndex"),
        toSlice("readWaveLaneCount"))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define i32 @readWaveLaneIndex()")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("define i32 @readWaveLaneCount()")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("define void @waveLaneCount(i32 addrspace(1)*")) >= 0);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call i32 @llvm.nvvm.read.ptx.sreg.laneid()")) == 1);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call i32 @llvm.nvvm.read.ptx.sreg.warpsize()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @readWaveLaneIndex()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @readWaveLaneCount()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret i32")) == 2);
        SLANG_CHECK(_countOccurrences(text, toSlice("getelementptr")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 1);
        const UnownedStringSlice llvm14Attributes =
            toSlice(" = { nofree nosync nounwind readnone speculatable willreturn }");
        const UnownedStringSlice legacyAttributes = toSlice(" = { nounwind readnone }");
        if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
        {
            SLANG_CHECK(_countOccurrences(text, llvm14Attributes) == 1);
            SLANG_CHECK(_countOccurrences(text, legacyAttributes) == 0);
        }
        else
        {
            SLANG_CHECK(_countOccurrences(text, llvm14Attributes) == 0);
            SLANG_CHECK(_countOccurrences(text, legacyAttributes) == 1);
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveReadLaneAtUIntKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("wave-read-lane-at-uint-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWaveReadLaneAtUIntKernel(
        builder,
        scope.module,
        toSlice("waveReadLaneAtUInt"),
        toSlice("readWaveLaneIndex"),
        toSlice("readWaveLaneAtUInt"))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define i32 @readWaveLaneIndex()")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("define i32 @readWaveLaneAtUInt(i32")) >= 0);
        SLANG_CHECK(
            text.indexOf(toSlice("define void @waveReadLaneAtUInt(i32 addrspace(1)*")) >= 0);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call i32 @llvm.nvvm.read.ptx.sreg.laneid()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @llvm.nvvm.shfl.sync.idx.i32")) == 1);
        SLANG_CHECK(text.indexOf(toSlice("i32 31)")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @readWaveLaneIndex()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @readWaveLaneAtUInt")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret i32")) == 2);
        SLANG_CHECK(_countOccurrences(text, toSlice("getelementptr")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 1);
        SLANG_CHECK(
            _countOccurrences(text, toSlice(" = { convergent inaccessiblememonly nounwind }")) ==
            1);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveReadLaneAtIntKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("wave-read-lane-at-int-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWaveReadLaneAtIntKernel(
        builder,
        scope.module,
        toSlice("waveReadLaneAtInt"),
        toSlice("readWaveLaneIndex"),
        toSlice("readWaveLaneAtInt"))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define i32 @readWaveLaneIndex()")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("define i32 @readWaveLaneAtInt(i32")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("define void @waveReadLaneAtInt(i32 addrspace(1)*")) >= 0);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call i32 @llvm.nvvm.read.ptx.sreg.laneid()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @llvm.nvvm.shfl.sync.idx.i32")) == 1);
        SLANG_CHECK(text.indexOf(toSlice("i32 31)")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @readWaveLaneIndex()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @readWaveLaneAtInt")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret i32")) == 2);
        SLANG_CHECK(_countOccurrences(text, toSlice("getelementptr")) == 2);
        SLANG_CHECK(_countOccurrences(text, toSlice("load i32")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 1);
        SLANG_CHECK(
            _countOccurrences(text, toSlice(" = { convergent inaccessiblememonly nounwind }")) ==
            1);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveReadLaneAtFloatKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("wave-read-lane-at-float-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWaveReadLaneAtFloatKernel(
        builder,
        scope.module,
        toSlice("waveReadLaneAtFloat"),
        toSlice("readWaveLaneIndex"),
        toSlice("readWaveLaneAtFloat"))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define i32 @readWaveLaneIndex()")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("define float @readWaveLaneAtFloat(i32")) >= 0);
        SLANG_CHECK(
            text.indexOf(toSlice("define void @waveReadLaneAtFloat(float addrspace(1)*")) >= 0);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call i32 @llvm.nvvm.read.ptx.sreg.laneid()")) == 1);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call float @llvm.nvvm.shfl.sync.idx.f32")) == 1);
        SLANG_CHECK(text.indexOf(toSlice("i32 31)")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @readWaveLaneIndex()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call float @readWaveLaneAtFloat")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret i32")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret float")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("getelementptr")) == 2);
        SLANG_CHECK(_countOccurrences(text, toSlice("load float")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store float")) == 1);
        SLANG_CHECK(
            _countOccurrences(text, toSlice(" = { convergent inaccessiblememonly nounwind }")) ==
            1);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveActiveMaskKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("wave-active-mask-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _populateWaveActiveMaskKernel(builder, scope.module, toSlice("waveActiveMask"))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define void @waveActiveMask(i32 addrspace(1)*")) >= 0);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call i32 @llvm.nvvm.read.ptx.sreg.laneid()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @llvm.nvvm.vote.ballot.sync")) == 1);
        SLANG_CHECK(text.indexOf(toSlice("i32 -1, i1 true")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("getelementptr")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 1);
        SLANG_CHECK(
            _countOccurrences(text, toSlice(" = { convergent inaccessiblememonly nounwind }")) ==
            1);
    }
}

// Checks the shared signless-i32 implementation and exact LLVM 14/LLVM 7 declaration forms for
// one independently negotiated Slang read-first semantic.
static void _checkNVVMIRBuilderBuildsWaveReadLaneFirstKernel(
    UnitTestContext* unitTestContext,
    SlangNVVMValueOperation operation,
    const UnownedStringSlice& moduleName,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& helperName,
    bool usesFloatValue)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(moduleName, scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWaveReadLaneFirstKernel(
        builder,
        scope.module,
        kernelName,
        helperName,
        operation,
        usesFloatValue)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        StringBuilder helperDefinition;
        helperDefinition << (usesFloatValue ? "define float @" : "define i32 @") << helperName
                         << "(i32";
        SLANG_CHECK(text.indexOf(helperDefinition.getUnownedSlice()) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @llvm.cttz.i32")) == 1);
        const UnownedStringSlice shuffleCall =
            usesFloatValue ? toSlice("call float @llvm.nvvm.shfl.sync.idx.f32")
                           : toSlice("call i32 @llvm.nvvm.shfl.sync.idx.i32");
        SLANG_CHECK(_countOccurrences(text, shuffleCall) == 1);
        SLANG_CHECK(text.indexOf(toSlice("i1 true")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("i32 31")) >= 0);
        SLANG_CHECK(
            _countOccurrences(
                text,
                usesFloatValue ? toSlice("store float") : toSlice("store i32")) == 1);
        const UnownedStringSlice llvm14Declaration = toSlice("@llvm.cttz.i32(i32, i1 immarg)");
        const UnownedStringSlice legacyDeclaration = toSlice("@llvm.cttz.i32(i32, i1)");
        const UnownedStringSlice llvm14Attributes =
            toSlice(" = { nofree nosync nounwind readnone speculatable willreturn }");
        const UnownedStringSlice legacyAttributes = toSlice(" = { nounwind readnone }");
        if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
        {
            SLANG_CHECK(_countOccurrences(text, llvm14Declaration) == 1);
            SLANG_CHECK(_countOccurrences(text, legacyDeclaration) == 0);
            SLANG_CHECK(_countOccurrences(text, llvm14Attributes) == 1);
            SLANG_CHECK(_countOccurrences(text, legacyAttributes) == 0);
        }
        else
        {
            SLANG_CHECK(_countOccurrences(text, llvm14Declaration) == 0);
            SLANG_CHECK(_countOccurrences(text, legacyDeclaration) == 1);
            SLANG_CHECK(_countOccurrences(text, llvm14Attributes) == 0);
            SLANG_CHECK(_countOccurrences(text, legacyAttributes) == 1);
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveReadLaneFirstUIntKernel)
{
    _checkNVVMIRBuilderBuildsWaveReadLaneFirstKernel(
        unitTestContext,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        toSlice("wave-read-lane-first-uint-module"),
        toSlice("waveReadLaneFirstUInt"),
        toSlice("readWaveLaneFirstUInt"),
        false);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveReadLaneFirstIntKernel)
{
    _checkNVVMIRBuilderBuildsWaveReadLaneFirstKernel(
        unitTestContext,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        toSlice("wave-read-lane-first-int-module"),
        toSlice("waveReadLaneFirstInt"),
        toSlice("readWaveLaneFirstInt"),
        false);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveReadLaneFirstFloatKernel)
{
    _checkNVVMIRBuilderBuildsWaveReadLaneFirstKernel(
        unitTestContext,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        toSlice("wave-read-lane-first-float-module"),
        toSlice("waveReadLaneFirstFloat"),
        toSlice("readWaveLaneFirstFloat"),
        true);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveMaskIsFirstLaneKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("wave-mask-is-first-lane-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWaveIsFirstLaneKernel(
        builder,
        scope.module,
        toSlice("waveIsFirstLane"),
        toSlice("waveMaskIsFirstLane"))));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("define i1 @waveMaskIsFirstLane(i32")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("sub i32 0,")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("and i32")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("shl i32 1,")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("icmp eq i32")) == 1);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call i32 @llvm.nvvm.read.ptx.sreg.laneid()")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret i1")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i1 @waveMaskIsFirstLane")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @llvm.nvvm.vote.ballot.sync")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 1);
    }
}

static void _checkNVVMIRBuilderBuildsWaveMaskVoteKernel(
    UnitTestContext* unitTestContext,
    SlangNVVMValueOperation operation,
    const UnownedStringSlice& moduleName,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& helperName,
    const UnownedStringSlice& intrinsicName)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(moduleName, scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWavePredicateIntrinsicKernel(
        builder,
        scope.module,
        kernelName,
        helperName,
        operation,
        WavePredicateValueKind::Boolean)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        StringBuilder helperDefinition;
        helperDefinition << "define i1 @" << helperName << "(i32";
        StringBuilder intrinsicCall;
        intrinsicCall << "call i1 @" << intrinsicName << "(i32";
        StringBuilder helperCall;
        helperCall << "call i1 @" << helperName;
        StringBuilder intrinsicDeclaration;
        intrinsicDeclaration << "declare i1 @" << intrinsicName << "(i32, i1)";
        SLANG_CHECK(text.indexOf(helperDefinition.getUnownedSlice()) >= 0);
        SLANG_CHECK(text.indexOf(toSlice(", i1")) >= 0);
        SLANG_CHECK(_countOccurrences(text, intrinsicCall.getUnownedSlice()) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret i1")) == 1);
        SLANG_CHECK(_countOccurrences(text, helperCall.getUnownedSlice()) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @llvm.nvvm.vote.ballot.sync")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 1);
        SLANG_CHECK(_countOccurrences(text, intrinsicDeclaration.getUnownedSlice()) == 1);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveMaskAnyTrueKernel)
{
    _checkNVVMIRBuilderBuildsWaveMaskVoteKernel(
        unitTestContext,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ANY_TRUE,
        toSlice("wave-mask-any-true-module"),
        toSlice("waveActiveAnyTrue"),
        toSlice("waveMaskAnyTrue"),
        toSlice("llvm.nvvm.vote.any.sync"));
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveMaskAllTrueKernel)
{
    _checkNVVMIRBuilderBuildsWaveMaskVoteKernel(
        unitTestContext,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_TRUE,
        toSlice("wave-mask-all-true-module"),
        toSlice("waveActiveAllTrue"),
        toSlice("waveMaskAllTrue"),
        toSlice("llvm.nvvm.vote.all.sync"));
}

static void _checkNVVMIRBuilderBuildsWaveMaskAllEqualKernel(
    UnitTestContext* unitTestContext,
    SlangNVVMValueOperation operation,
    const UnownedStringSlice& moduleName,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& helperName,
    WavePredicateValueKind valueKind)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(moduleName, scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWavePredicateIntrinsicKernel(
        builder,
        scope.module,
        kernelName,
        helperName,
        operation,
        valueKind)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        StringBuilder helperDefinition;
        helperDefinition << "define i1 @" << helperName << "(i32";
        StringBuilder helperCall;
        helperCall << "call i1 @" << helperName;
        SLANG_CHECK(text.indexOf(helperDefinition.getUnownedSlice()) >= 0);
        SLANG_CHECK(
            _countOccurrences(
                text,
                toSlice("call { i32, i1 } @llvm.nvvm.match.all.sync.i32p(i32")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("extractvalue { i32, i1 }")) == 1);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("bitcast float")) ==
            (valueKind == WavePredicateValueKind::Float ? 1 : 0));
        SLANG_CHECK(_countOccurrences(text, toSlice(", 1")) >= 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("ret i1")) == 1);
        SLANG_CHECK(_countOccurrences(text, helperCall.getUnownedSlice()) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i32 @llvm.nvvm.vote.ballot.sync")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 1);
        SLANG_CHECK(
            _countOccurrences(
                text,
                toSlice("declare { i32, i1 } @llvm.nvvm.match.all.sync.i32p(i32, i32)")) == 1);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveMaskAllEqualIntKernel)
{
    _checkNVVMIRBuilderBuildsWaveMaskAllEqualKernel(
        unitTestContext,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        toSlice("wave-mask-all-equal-int-module"),
        toSlice("waveActiveAllEqualInt"),
        toSlice("waveMaskAllEqualInt"),
        WavePredicateValueKind::Integer);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveMaskAllEqualUIntKernel)
{
    _checkNVVMIRBuilderBuildsWaveMaskAllEqualKernel(
        unitTestContext,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        toSlice("wave-mask-all-equal-uint-module"),
        toSlice("waveActiveAllEqualUInt"),
        toSlice("waveMaskAllEqualUInt"),
        WavePredicateValueKind::Integer);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveMaskAllEqualFloatKernel)
{
    _checkNVVMIRBuilderBuildsWaveMaskAllEqualKernel(
        unitTestContext,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        toSlice("wave-mask-all-equal-float-module"),
        toSlice("waveActiveAllEqualFloat"),
        toSlice("waveMaskAllEqualFloat"),
        WavePredicateValueKind::Float);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsFloat32CopyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("float32-copy-module"), scope.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_populateFloat32CopyKernel(builder, scope.module, toSlice("float32Copy"))));

    ComPtr<ISlangBlob> llvmAssembly;
    ComPtr<ISlangBlob> nvvmAssembly;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        llvmAssembly)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        nvvmAssembly)));
    SLANG_CHECK_ABORT(llvmAssembly != nullptr && nvvmAssembly != nullptr);

    const String llvmText(UnownedStringSlice(
        static_cast<const char*>(llvmAssembly->getBufferPointer()),
        llvmAssembly->getBufferSize()));
    const String nvvmText(UnownedStringSlice(
        static_cast<const char*>(nvvmAssembly->getBufferPointer()),
        nvvmAssembly->getBufferSize()));
    const String texts[] = {llvmText, nvvmText};
    for (const String& text : texts)
    {
        SLANG_CHECK(text.indexOf("define void @float32Copy(float addrspace(1)*") >= 0);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("float addrspace(1)*")) >= 4);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("load float")) == 1);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("store float")) == 1);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("align 4")) == 2);
        SLANG_CHECK(text.indexOf("fadd float") < 0);
    }
    SLANG_CHECK(nvvmText.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(nvvmText.indexOf("!\"kernel\", i32 1") >= 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsNumericTypeFamilies)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("numeric-family-module"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateNumericFamilyFunction(builder, scope.module)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        SLANG_CHECK_ABORT(assembly != nullptr);
        const UnownedStringSlice text(
            static_cast<const char*>(assembly->getBufferPointer()),
            assembly->getBufferSize());
        SLANG_CHECK(text.indexOf(toSlice("add i8")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("icmp slt i8")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("icmp ugt i8")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("select i1")) >= 4);
        SLANG_CHECK(text.indexOf(toSlice("declare float @__nv_fminf(float, float)")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("declare double @__nv_fmax(double, double)")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("sext i8")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("zext i8")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("sitofp i8")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fptoui float")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("bitcast float")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("bitcast i32")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("sitofp i8")) < text.indexOf(toSlice("to half")));
        SLANG_CHECK(text.indexOf(toSlice("fadd half")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fsub half")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp olt half")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fpext half")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fptrunc float")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fptosi half")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("sitofp <2 x i32>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fadd <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fsub <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp oge <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fpext <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fptrunc <2 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fptosi <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("add <2 x i32>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("shl <2 x i32>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("lshr <2 x i32>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("icmp eq <2 x i32>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("ashr <2 x i8>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("sdiv <2 x i8>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("srem <2 x i8>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("icmp slt <2 x i8>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fadd <3 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("frem <3 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fmul <3 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp oeq <3 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp une <3 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp olt <3 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp ogt <3 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp ole <3 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp oge <3 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("xor <2 x i1>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("and <2 x i1>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("or <2 x i1>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("icmp eq <2 x i1>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("icmp ne <2 x i1>")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("select <2 x i1>")) >= 2);
        SLANG_CHECK(text.indexOf(toSlice("select i1")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("insertelement <2 x i1>")) >= 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("extractelement <2 x i1>")) >= 2);
        SLANG_CHECK(_countOccurrences(text, toSlice("insertelement <2 x half>")) == 2);
        SLANG_CHECK(_countOccurrences(text, toSlice("extractelement <2 x half>")) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("select i1")) >= 2);
        SLANG_CHECK(_countOccurrences(text, toSlice("insertelement")) >= 20);
        SLANG_CHECK(text.indexOf(toSlice("poison")) < 0);
        SLANG_CHECK(text.indexOf(toSlice("ret <2 x i32>")) >= 0);
    }

    const SlangNVVMValueTypeDesc signedI8 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        8,
        1,
    };
    const SlangNVVMValueTypeDesc unsignedI8 = {
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
        8,
        1,
    };
    const SlangNVVMValueTypeDesc mixedOperandTypes[] = {signedI8, unsignedI8};
    const SlangNVVMValueOperationDesc mixedSignednessAdd = {
        SLANG_NVVM_VALUE_OP_ADD,
        signedI8,
        mixedOperandTypes,
        SLANG_COUNT_OF(mixedOperandTypes),
    };
    SLANG_CHECK(!builder.supportsValueOperation(mixedSignednessAdd));
    SlangNVVMValueHandle invalidOperands[2] = {};
    SlangNVVMValueHandle invalidResult = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitValueOperation(
            scope.module,
            mixedSignednessAdd,
            invalidOperands,
            SLANG_COUNT_OF(invalidOperands),
            invalidResult) == SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(invalidResult == nullptr);

    const SlangNVVMValueTypeDesc signedI24 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        24,
        1,
    };
    const SlangNVVMValueTypeDesc unsupportedWidthOperandTypes[] = {signedI24, signedI24};
    const SlangNVVMValueOperationDesc unsupportedWidthAdd = {
        SLANG_NVVM_VALUE_OP_ADD,
        signedI24,
        unsupportedWidthOperandTypes,
        SLANG_COUNT_OF(unsupportedWidthOperandTypes),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedWidthAdd));

    const SlangNVVMValueTypeDesc signedI32x5 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        32,
        5,
    };
    const SlangNVVMValueTypeDesc vectorOperandTypes[] = {signedI32x5, signedI32x5};
    const SlangNVVMValueOperationDesc unsupportedVectorWidthMultiply = {
        SLANG_NVVM_VALUE_OP_MULTIPLY,
        signedI32x5,
        vectorOperandTypes,
        SLANG_COUNT_OF(vectorOperandTypes),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedVectorWidthMultiply));

    const SlangNVVMValueTypeDesc bool2 = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, 2};
    const SlangNVVMValueTypeDesc boolOperandTypes[] = {bool2, bool2};
    const SlangNVVMValueOperationDesc unsupportedBooleanAdd = {
        SLANG_NVVM_VALUE_OP_ADD,
        bool2,
        boolOperandTypes,
        SLANG_COUNT_OF(boolOperandTypes),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedBooleanAdd));

    const SlangNVVMValueTypeDesc signedI32x2 = NVVMSemantics::kSignedI32x2;
    const SlangNVVMValueTypeDesc signedI32x2Operands[] = {signedI32x2, signedI32x2};
    const SlangNVVMValueOperationDesc unsupportedVectorIntegerMinimum = {
        SLANG_NVVM_VALUE_OP_MIN,
        signedI32x2,
        signedI32x2Operands,
        SLANG_COUNT_OF(signedI32x2Operands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedVectorIntegerMinimum));
    const SlangNVVMValueTypeDesc bool3 = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, 3};
    const SlangNVVMValueOperationDesc mismatchedComparisonLanes = {
        SLANG_NVVM_VALUE_OP_EQUAL,
        bool3,
        signedI32x2Operands,
        SLANG_COUNT_OF(signedI32x2Operands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(mismatchedComparisonLanes));

    const SlangNVVMValueTypeDesc signedI32 = NVVMSemantics::kSignedI32;
    const SlangNVVMValueTypeDesc scalarOperands[] = {signedI32, signedI32};
    const SlangNVVMValueOperationDesc scalarOperandsWithVectorResult = {
        SLANG_NVVM_VALUE_OP_ADD,
        signedI32x2,
        scalarOperands,
        SLANG_COUNT_OF(scalarOperands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(scalarOperandsWithVectorResult));
    invalidResult = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitValueOperation(
            scope.module,
            scalarOperandsWithVectorResult,
            invalidOperands,
            SLANG_COUNT_OF(invalidOperands),
            invalidResult) == SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(invalidResult == nullptr);

    const SlangNVVMValueTypeDesc signedI16 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        16,
        1,
    };
    const SlangNVVMValueTypeDesc mismatchedWidthOperands[] = {signedI32x2, signedI16};
    const SlangNVVMValueOperationDesc mismatchedBroadcastWidth = {
        SLANG_NVVM_VALUE_OP_ADD,
        signedI32x2,
        mismatchedWidthOperands,
        SLANG_COUNT_OF(mismatchedWidthOperands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(mismatchedBroadcastWidth));

    const SlangNVVMValueTypeDesc float64x2 = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        64,
        2,
    };
    const SlangNVVMValueTypeDesc float64x2Operands[] = {float64x2, float64x2};
    const SlangNVVMValueOperationDesc unsupportedFloatRemainder = {
        SLANG_NVVM_VALUE_OP_REMAINDER,
        float64x2,
        float64x2Operands,
        SLANG_COUNT_OF(float64x2Operands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedFloatRemainder));

    const SlangNVVMValueOperationDesc unsupportedVectorFloatMinimum = {
        SLANG_NVVM_VALUE_OP_MIN,
        float64x2,
        float64x2Operands,
        SLANG_COUNT_OF(float64x2Operands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedVectorFloatMinimum));

    const SlangNVVMValueTypeDesc float16 = NVVMSemantics::kFloat16;
    const SlangNVVMValueTypeDesc float32 = NVVMSemantics::kFloat32;
    const SlangNVVMValueTypeDesc float16BinaryOperands[] = {float16, float16};
    const SlangNVVMValueOperationDesc unsupportedHalfMaximum = {
        SLANG_NVVM_VALUE_OP_MAX,
        float16,
        float16BinaryOperands,
        SLANG_COUNT_OF(float16BinaryOperands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedHalfMaximum));
    const SlangNVVMValueTypeDesc sameWidthFloatOperands[] = {float16};
    const SlangNVVMValueOperationDesc sameWidthFloatConvert = {
        SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
        float16,
        sameWidthFloatOperands,
        SLANG_COUNT_OF(sameWidthFloatOperands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(sameWidthFloatConvert));

    const SlangNVVMValueTypeDesc sameBitTypeOperands[] = {float32};
    const SlangNVVMValueOperationDesc sameTypeBitReinterpret = {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        float32,
        sameBitTypeOperands,
        SLANG_COUNT_OF(sameBitTypeOperands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(sameTypeBitReinterpret));

    const SlangNVVMValueTypeDesc float16x2 = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        16,
        2,
    };
    const SlangNVVMValueTypeDesc mismatchedFloatConvertOperands[] = {float16x2};
    const SlangNVVMValueOperationDesc mismatchedFloatConvertLanes = {
        SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
        float32,
        mismatchedFloatConvertOperands,
        SLANG_COUNT_OF(mismatchedFloatConvertOperands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(mismatchedFloatConvertLanes));
    invalidResult = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitValueOperation(
            scope.module,
            mismatchedFloatConvertLanes,
            invalidOperands,
            1,
            invalidResult) == SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(invalidResult == nullptr);

    const SlangNVVMValueTypeDesc float32x2 = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        32,
        2,
    };
    const SlangNVVMValueTypeDesc float32x2Operands[] = {float32x2, float32x2};
    const SlangNVVMValueOperationDesc mismatchedFloatComparisonLanes = {
        SLANG_NVVM_VALUE_OP_EQUAL,
        bool3,
        float32x2Operands,
        SLANG_COUNT_OF(float32x2Operands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(mismatchedFloatComparisonLanes));

    const SlangNVVMValueTypeDesc bool2Operands[] = {bool2, bool2};
    const SlangNVVMValueOperationDesc unsupportedBooleanOrdering = {
        SLANG_NVVM_VALUE_OP_LESS_THAN,
        bool2,
        bool2Operands,
        SLANG_COUNT_OF(bool2Operands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedBooleanOrdering));
    invalidResult = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitValueOperation(
            scope.module,
            unsupportedBooleanOrdering,
            invalidOperands,
            SLANG_COUNT_OF(invalidOperands),
            invalidResult) == SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(invalidResult == nullptr);

    const SlangNVVMValueTypeDesc scalarConditionVectorSelectOperands[] = {
        NVVMSemantics::kBool,
        signedI32x2,
        signedI32x2,
    };
    const SlangNVVMValueOperationDesc scalarConditionVectorSelect = {
        SLANG_NVVM_VALUE_OP_SELECT,
        signedI32x2,
        scalarConditionVectorSelectOperands,
        SLANG_COUNT_OF(scalarConditionVectorSelectOperands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(scalarConditionVectorSelect));

    const SlangNVVMValueTypeDesc unsignedI32x2 = {
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
        32,
        2,
    };
    const SlangNVVMValueTypeDesc mismatchedSelectAlternatives[] = {
        bool2,
        signedI32x2,
        unsignedI32x2,
    };
    const SlangNVVMValueOperationDesc mismatchedSelect = {
        SLANG_NVVM_VALUE_OP_SELECT,
        signedI32x2,
        mismatchedSelectAlternatives,
        SLANG_COUNT_OF(mismatchedSelectAlternatives),
    };
    SLANG_CHECK(!builder.supportsValueOperation(mismatchedSelect));
}

SLANG_UNIT_TEST(nvvmIRBuilderRealProviderPreservesShortBuffers)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("real-short-buffer"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _populateEmptyNVVMKernel(builder, scope.module, toSlice("realShortBufferKernel"))));

    const SlangNVVMBuilderFoundationAPI* foundationAPI = builder.getFoundationAPI();
    SLANG_CHECK_ABORT(foundationAPI != nullptr);
    SLANG_CHECK_ABORT(foundationAPI->serializeModuleWithDiagnostics != nullptr);

    size_t requiredSerializedSize = 0;
    size_t requiredDiagnosticSize = 0;
    SlangNVVMVerificationStatus verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(foundationAPI->serializeModuleWithDiagnostics(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        nullptr,
        0,
        &requiredSerializedSize,
        nullptr,
        0,
        &requiredDiagnosticSize,
        &verificationStatus)));
    SLANG_CHECK(requiredSerializedSize > 8);
    SLANG_CHECK(requiredDiagnosticSize == 0);
    SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);

    // A query has both destinations null. Supplying even an otherwise-unneeded diagnostic
    // destination makes this a write, so omitting the non-empty serialized destination must fail
    // without touching the diagnostic sentinel.
    uint8_t mixedDiagnosticSentinel = 0x3c;
    size_t mixedSerializedSize = 0;
    size_t mixedDiagnosticSize = 1;
    verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK(
        foundationAPI->serializeModuleWithDiagnostics(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            nullptr,
            0,
            &mixedSerializedSize,
            &mixedDiagnosticSentinel,
            1,
            &mixedDiagnosticSize,
            &verificationStatus) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(mixedSerializedSize == requiredSerializedSize);
    SLANG_CHECK(mixedDiagnosticSize == requiredDiagnosticSize);
    SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);
    SLANG_CHECK(mixedDiagnosticSentinel == 0x3c);

    uint8_t serializedSentinels[8];
    ::memset(serializedSentinels, 0x5a, sizeof(serializedSentinels));
    size_t reportedSerializedSize = 0;
    size_t reportedDiagnosticSize = 1;
    verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK(requiredSerializedSize > sizeof(serializedSentinels));
    SLANG_CHECK(
        foundationAPI->serializeModuleWithDiagnostics(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            serializedSentinels,
            sizeof(serializedSentinels),
            &reportedSerializedSize,
            nullptr,
            0,
            &reportedDiagnosticSize,
            &verificationStatus) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(reportedSerializedSize == requiredSerializedSize);
    SLANG_CHECK(reportedDiagnosticSize == requiredDiagnosticSize);
    SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);
    for (const auto value : serializedSentinels)
        SLANG_CHECK(value == 0x5a);

    ComPtr<ISlangBlob> bitcode;
    String diagnostics = "stale diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        bitcode,
        diagnostics)));
    SLANG_CHECK_ABORT(bitcode != nullptr);
    SLANG_CHECK(bitcode->getBufferSize() == requiredSerializedSize);
    SLANG_CHECK(diagnostics.getLength() == 0);
}

// Exercise the module boundary's rejected input shapes with handles produced by the real LLVM 14
// implementation. These checks keep malformed LLVM IR from becoming a libNVVM diagnostic later.
SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule firstModule;
    firstModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("first-module"), firstModule.module)));
    ScopedNVVMBuilderModule secondModule;
    secondModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("second-module"), secondModule.module)));

    SlangNVVMTypeHandle firstVoidType = nullptr;
    SlangNVVMTypeHandle secondVoidType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(firstModule.module, firstVoidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(secondModule.module, secondVoidType)));

    SlangNVVMTypeHandle invalidFunctionType = nullptr;
    SLANG_CHECK(
        builder.getFunctionType(
            firstModule.module,
            firstVoidType,
            &firstVoidType,
            1,
            invalidFunctionType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunctionType == nullptr);
    SLANG_CHECK(
        builder
            .getFunctionType(firstModule.module, secondVoidType, nullptr, 0, invalidFunctionType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunctionType == nullptr);

    SlangNVVMTypeHandle firstFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(firstModule.module, firstVoidType, nullptr, 0, firstFunctionType)));
    SlangNVVMValueHandle firstFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        firstModule.module,
        firstFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("uniqueKernel"),
        firstFunction)));

    SlangNVVMValueHandle invalidFunction = nullptr;
    SLANG_CHECK(
        builder.declareFunction(
            firstModule.module,
            firstFunctionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("uniqueKernel"),
            invalidFunction) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunction == nullptr);
    SLANG_CHECK(
        builder.declareFunction(
            secondModule.module,
            firstFunctionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("foreignType"),
            invalidFunction) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunction == nullptr);

    SlangNVVMBlockHandle invalidBlock = nullptr;
    SLANG_CHECK(
        builder.createBlock(
            secondModule.module,
            firstFunction,
            toSlice("foreignFunction"),
            invalidBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidBlock == nullptr);

    SlangNVVMBlockHandle firstBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("entry"), firstBlock)));
    SLANG_CHECK(builder.setInsertBlock(secondModule.module, firstBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.markFunctionAsKernel(secondModule.module, firstFunction) == SLANG_E_INVALID_ARG);

    const SlangNVVMSerializationFormat unknownFormat =
        SlangNVVMSerializationFormat(SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY + 1);
    const SlangNVVMBuilderFoundationAPI* foundationAPI = builder.getFoundationAPI();
    SLANG_CHECK_ABORT(foundationAPI != nullptr);
    SLANG_CHECK_ABORT(foundationAPI->serializeModuleWithDiagnostics != nullptr);

    size_t compatibleUnknownFormatSerializedSize = 1;
    size_t compatibleUnknownFormatDiagnosticSize = 1;
    SlangNVVMVerificationStatus compatibleUnknownFormatStatus = SLANG_NVVM_VERIFICATION_VALID;
    SLANG_CHECK(
        foundationAPI->serializeModuleWithDiagnostics(
            firstModule.module,
            unknownFormat,
            nullptr,
            0,
            &compatibleUnknownFormatSerializedSize,
            nullptr,
            0,
            &compatibleUnknownFormatDiagnosticSize,
            &compatibleUnknownFormatStatus) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(compatibleUnknownFormatSerializedSize == 0);
    SLANG_CHECK(compatibleUnknownFormatDiagnosticSize == 0);
    SLANG_CHECK(compatibleUnknownFormatStatus == SLANG_NVVM_VERIFICATION_NOT_RUN);

    size_t invalidSerializedSize = 1;
    size_t invalidDiagnosticSize = 0;
    SlangNVVMVerificationStatus invalidStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(foundationAPI->serializeModuleWithDiagnostics(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        nullptr,
        0,
        &invalidSerializedSize,
        nullptr,
        0,
        &invalidDiagnosticSize,
        &invalidStatus)));
    SLANG_CHECK(invalidSerializedSize == 0);
    SLANG_CHECK(invalidDiagnosticSize > 0);
    SLANG_CHECK(invalidStatus == SLANG_NVVM_VERIFICATION_INVALID);

    uint8_t invalidSerializedSentinel = 0xa5;
    uint8_t invalidDiagnosticSentinel = 0x5a;
    size_t reportedInvalidSerializedSize = 1;
    size_t reportedInvalidDiagnosticSize = 0;
    invalidStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK(
        foundationAPI->serializeModuleWithDiagnostics(
            firstModule.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            &invalidSerializedSentinel,
            1,
            &reportedInvalidSerializedSize,
            &invalidDiagnosticSentinel,
            1,
            &reportedInvalidDiagnosticSize,
            &invalidStatus) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(reportedInvalidSerializedSize == 0);
    SLANG_CHECK(reportedInvalidDiagnosticSize == invalidDiagnosticSize);
    SLANG_CHECK(invalidStatus == SLANG_NVVM_VERIFICATION_INVALID);
    SLANG_CHECK(invalidSerializedSentinel == 0xa5);
    SLANG_CHECK(invalidDiagnosticSentinel == 0x5a);

    ComPtr<ISlangBlob> invalidBitcode;
    SLANG_CHECK(
        builder.serializeModule(
            firstModule.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            invalidBitcode) == SLANG_FAIL);
    SLANG_CHECK(invalidBitcode == nullptr);

    String verifierDiagnostics = "stale diagnostics";
    SLANG_CHECK(
        builder.serializeModule(
            firstModule.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            invalidBitcode,
            verifierDiagnostics) == SLANG_FAIL);
    SLANG_CHECK(invalidBitcode == nullptr);
    SLANG_CHECK(verifierDiagnostics.indexOf("does not have terminator") >= 0);
    SLANG_CHECK(verifierDiagnostics.indexOf("uniqueKernel") >= 0);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, firstBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.markFunctionAsKernel(firstModule.module, firstFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        invalidBitcode)));
    SLANG_CHECK(invalidBitcode != nullptr);

    invalidBitcode.setNull();
    verifierDiagnostics = "stale diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        invalidBitcode,
        verifierDiagnostics)));
    SLANG_CHECK(invalidBitcode != nullptr);
    SLANG_CHECK(verifierDiagnostics.getLength() == 0);
}

// Scalar-memory calls reject malformed module-owned shapes before they can insert LLVM IR.
SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule firstModule;
    firstModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-first"), firstModule.module)));
    ScopedNVVMBuilderModule secondModule;
    secondModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-second"), secondModule.module)));

    SlangNVVMTypeHandle firstVoidType = nullptr;
    SlangNVVMTypeHandle firstIntegerType = nullptr;
    SlangNVVMTypeHandle firstGlobalPointerType = nullptr;
    SlangNVVMTypeHandle firstConstantPointerType = nullptr;
    SlangNVVMTypeHandle secondVoidType = nullptr;
    SlangNVVMTypeHandle secondIntegerType = nullptr;
    SlangNVVMTypeHandle secondGlobalPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(firstModule.module, firstVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(firstModule.module, 32, firstIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        firstModule.module,
        firstIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        firstGlobalPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        firstModule.module,
        firstIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_CONSTANT,
        firstConstantPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(secondModule.module, secondVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(secondModule.module, 32, secondIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        secondModule.module,
        secondIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        secondGlobalPointerType)));

    const SlangNVVMBuilderConstructionAPI* scalarAPI = builder.getConstructionAPI();
    SLANG_CHECK_ABORT(scalarAPI != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->getIntegerType != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->getPointerType != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->getFunctionParameter != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->emitLoad != nullptr);
    SLANG_CHECK(scalarAPI->getIntegerType(firstModule.module, 32, nullptr) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        scalarAPI->getPointerType(
            firstModule.module,
            firstIntegerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            nullptr) == SLANG_E_INVALID_ARG);

    SlangNVVMTypeHandle rejectedType = firstVoidType;
    SLANG_CHECK(builder.getIntegerType(firstModule.module, 0, rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    static const uint32_t kMaximumIntegerBitWidth = 1u << 23;
    SlangNVVMTypeHandle maximumIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerType(firstModule.module, kMaximumIntegerBitWidth, maximumIntegerType)));
    SLANG_CHECK(maximumIntegerType != nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getIntegerType(firstModule.module, kMaximumIntegerBitWidth + 1, rejectedType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getPointerType(
            firstModule.module,
            firstVoidType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getPointerType(
            firstModule.module,
            firstIntegerType,
            SlangNVVMAddressSpace(2),
            rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getPointerType(
            firstModule.module,
            secondIntegerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);

    const SlangNVVMTypeHandle firstParameterTypes[] = {
        firstGlobalPointerType,
        firstIntegerType,
        firstConstantPointerType,
    };
    SlangNVVMTypeHandle firstFunctionType = nullptr;
    SlangNVVMValueHandle firstFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        firstModule.module,
        firstVoidType,
        firstParameterTypes,
        SLANG_COUNT_OF(firstParameterTypes),
        firstFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        firstModule.module,
        firstFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("rejectInvalidScalarOperations"),
        firstFunction)));

    SlangNVVMValueHandle firstDestination = nullptr;
    SlangNVVMValueHandle firstValue = nullptr;
    SlangNVVMValueHandle firstConstantDestination = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 0, firstDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 1, firstValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .getFunctionParameter(firstModule.module, firstFunction, 2, firstConstantDestination)));
    SLANG_CHECK(
        scalarAPI->getFunctionParameter(firstModule.module, firstFunction, 0, nullptr) ==
        SLANG_E_INVALID_ARG);

    const SlangNVVMTypeHandle secondParameterTypes[] = {
        secondGlobalPointerType,
        secondIntegerType,
    };
    SlangNVVMTypeHandle secondFunctionType = nullptr;
    SlangNVVMValueHandle secondFunction = nullptr;
    SlangNVVMValueHandle secondDestination = nullptr;
    SlangNVVMValueHandle secondValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        secondModule.module,
        secondVoidType,
        secondParameterTypes,
        SLANG_COUNT_OF(secondParameterTypes),
        secondFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        secondModule.module,
        secondFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignScalarFunction"),
        secondFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(secondModule.module, secondFunction, 0, secondDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(secondModule.module, secondFunction, 1, secondValue)));

    SlangNVVMValueHandle rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.getFunctionParameter(
            firstModule.module,
            firstFunction,
            SLANG_COUNT_OF(firstParameterTypes),
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.getFunctionParameter(firstModule.module, secondFunction, 0, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    // A declared function has no insertion block yet. Both operations must fail without mutation.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(
            firstModule.module,
            firstDestination,
            4,
            SLANG_NVVM_LOAD_FLAG_NONE,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);

    SlangNVVMBlockHandle firstBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("entry"), firstBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, firstBlock)));
    SLANG_CHECK(
        scalarAPI->emitLoad(
            firstModule.module,
            firstDestination,
            4,
            SLANG_NVVM_LOAD_FLAG_NONE,
            nullptr) == SLANG_E_INVALID_ARG);

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(
            firstModule.module,
            firstDestination,
            4,
            SlangNVVMLoadFlags(2u),
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(
            firstModule.module,
            firstValue,
            4,
            SLANG_NVVM_LOAD_FLAG_NONE,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(
            firstModule.module,
            secondDestination,
            4,
            SLANG_NVVM_LOAD_FLAG_NONE,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    static const uint32_t kInvalidAlignments[] = {0u, 3u};
    for (uint32_t invalidAlignment : kInvalidAlignments)
    {
        rejectedValue = firstFunction;
        SLANG_CHECK(
            builder.emitLoad(
                firstModule.module,
                firstDestination,
                invalidAlignment,
                SLANG_NVVM_LOAD_FLAG_NONE,
                rejectedValue) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
        SLANG_CHECK(
            builder.emitStore(firstModule.module, firstValue, firstDestination, invalidAlignment) ==
            SLANG_E_INVALID_ARG);
    }
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstDestination, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstValue, 4) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, secondValue, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, secondDestination, 4) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstConstantDestination, 4) ==
        SLANG_E_INVALID_ARG);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(
            firstModule.module,
            firstDestination,
            4,
            SLANG_NVVM_LOAD_FLAG_NONE,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics = "stale diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf(" = load ") < 0);
    SLANG_CHECK(assembly.indexOf("\n  store ") < 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarControlOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule firstModule;
    firstModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-control-first"), firstModule.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-control-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(firstModule.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(firstModule.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        firstModule.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));
    const SlangNVVMTypeHandle parameterTypes[] = {pointerType, integerType, integerType};
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        firstModule.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));

    auto declareFunction = [&](const char* name, SlangNVVMValueHandle& outFunction)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            firstModule.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            UnownedStringSlice(name),
            outFunction)));
    };
    SlangNVVMValueHandle firstFunction = nullptr;
    SlangNVVMValueHandle secondFunction = nullptr;
    declareFunction("firstControlFunction", firstFunction);
    declareFunction("secondControlFunction", secondFunction);

    SlangNVVMValueHandle firstDestination = nullptr;
    SlangNVVMValueHandle firstX = nullptr;
    SlangNVVMValueHandle firstY = nullptr;
    SlangNVVMValueHandle secondDestination = nullptr;
    SlangNVVMValueHandle secondX = nullptr;
    SlangNVVMValueHandle secondY = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 0, firstDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 1, firstX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 2, firstY)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, secondFunction, 0, secondDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, secondFunction, 1, secondX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, secondFunction, 2, secondY)));

    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    const SlangNVVMTypeHandle foreignParameterTypes[] = {
        foreignIntegerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SlangNVVMValueHandle foreignFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignControlFunction"),
        foreignFunction)));
    SlangNVVMValueHandle foreignX = nullptr;
    SlangNVVMValueHandle foreignY = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignY)));
    SlangNVVMBlockHandle foreignBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createBlock(
        foreignModule.module,
        foreignFunction,
        toSlice("foreign-entry"),
        foreignBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(foreignModule.module, foreignBlock)));
    SlangNVVMValueHandle foreignCondition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(
        foreignModule.module,
        foreignX,
        foreignY,
        foreignCondition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(foreignModule.module)));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle trueBlock = nullptr;
    SlangNVVMBlockHandle falseBlock = nullptr;
    SlangNVVMBlockHandle mergeBlock = nullptr;
    SlangNVVMBlockHandle secondBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("true"), trueBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("false"), falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createBlock(
        firstModule.module,
        secondFunction,
        toSlice("second-entry"),
        secondBlock)));

    SlangNVVMValueHandle rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitBranch(firstModule.module, mergeBlock) == SLANG_E_INVALID_ARG);

    // Produce a live i1 in a second function, then prove that values and blocks from that function
    // cannot be consumed at the first function's insertion point.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, secondBlock)));
    SlangNVVMValueHandle secondCondition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerSignedLessThan(firstModule.module, secondX, secondY, secondCondition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, entryBlock)));
    const SlangNVVMBuilderValueOperationsAPI* valueAPI = builder.getValueOperationsAPI();
    SLANG_CHECK_ABORT(valueAPI != nullptr);
    const SlangNVVMValueTypeDesc boolType = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, 1};
    const SlangNVVMValueTypeDesc signedI32 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        32,
        1,
    };
    const SlangNVVMValueTypeDesc operandTypes[] = {signedI32, signedI32};
    SlangNVVMValueOperationDesc operationDesc = {
        SLANG_NVVM_VALUE_OP_ADD,
        signedI32,
        operandTypes,
        SLANG_COUNT_OF(operandTypes),
    };
    const SlangNVVMValueHandle operands[] = {firstX, firstY};
    SLANG_CHECK(
        valueAPI->emitOperation(
            firstModule.module,
            &operationDesc,
            operands,
            SLANG_COUNT_OF(operands),
            nullptr) == SLANG_E_INVALID_ARG);
    operationDesc.operation = SLANG_NVVM_VALUE_OP_LESS_THAN;
    operationDesc.resultType = boolType;
    SLANG_CHECK(
        valueAPI->emitOperation(
            firstModule.module,
            &operationDesc,
            operands,
            SLANG_COUNT_OF(operands),
            nullptr) == SLANG_E_INVALID_ARG);

    // Context ownership is stricter than function ownership: values, conditions, and blocks from
    // another provider module must be rejected before any first-module instruction is created.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            firstX,
            foreignX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerSignedLessThan(firstModule.module, firstY, foreignY, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder
            .emitConditionalBranch(firstModule.module, foreignCondition, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitBranch(firstModule.module, foreignBlock) == SLANG_E_INVALID_ARG);

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SlangNVVMValueOperation(SLANG_NVVM_VALUE_OP_SUBTRACT + 1),
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            firstX,
            firstDestination,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            firstX,
            secondX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerSignedLessThan(firstModule.module, firstX, secondY, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, secondX, firstDestination, 4) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitLoad(
            firstModule.module,
            secondDestination,
            4,
            SLANG_NVVM_LOAD_FLAG_NONE,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitConditionalBranch(firstModule.module, firstX, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitConditionalBranch(firstModule.module, secondCondition, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitBranch(firstModule.module, secondBlock) == SLANG_E_INVALID_ARG);

    // Every rejected call above must leave the entry block untouched, so one valid graph still
    // verifies and contains exactly the instructions deliberately emitted below.
    SlangNVVMValueHandle condition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerSignedLessThan(firstModule.module, firstX, firstY, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(firstModule.module, condition, trueBlock, falseBlock)));

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitBranch(firstModule.module, mergeBlock) == SLANG_E_INVALID_ARG);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, trueBlock)));
    SlangNVVMValueHandle sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .emitIntegerBinary(firstModule.module, SLANG_NVVM_VALUE_OP_ADD, firstX, firstY, sum)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(firstModule.module, sum, firstDestination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(firstModule.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, falseBlock)));
    // `sum` belongs to the sibling true block and does not dominate this insertion point. Both
    // instruction-producing and side-effecting consumers must reject it without changing the
    // false block.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            sum,
            firstX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, sum, firstDestination, 4) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle difference = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        firstModule.module,
        SLANG_NVVM_VALUE_OP_SUBTRACT,
        firstX,
        firstY,
        difference)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(firstModule.module, difference, firstDestination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(firstModule.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, mergeBlock)));
    // The merge is reachable without executing the true block as well, so the same value remains
    // unavailable here. The final assembly counts below prove these failures added no instructions.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_VALUE_OP_SUBTRACT,
            sum,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, sum, firstDestination, 4) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.markFunctionAsKernel(firstModule.module, firstFunction)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("icmp slt i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("add i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("sub i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("br i1")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarSSAOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-ssa"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-ssa-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle pointerType = nullptr;
    SlangNVVMTypeHandle foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));

    const SlangNVVMBuilderConstructionAPI* ssaAPI = builder.getConstructionAPI();
    SLANG_CHECK_ABORT(ssaAPI != nullptr);
    SLANG_CHECK(
        ssaAPI->getIntegerConstant(module.module, integerType, 0, nullptr) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        ssaAPI->emitPhi(module.module, nullptr, integerType, nullptr) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getIntegerConstant(module.module, voidType, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getIntegerConstant(module.module, foreignIntegerType, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    static const int64_t kOutOfI32Range[] = {INT64_C(2147483648), -INT64_C(2147483649)};
    for (int64_t value : kOutOfI32Range)
    {
        rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.getIntegerConstant(module.module, integerType, value, rejectedValue) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
    }
    SlangNVVMValueHandle minimum = nullptr;
    SlangNVVMValueHandle maximum = nullptr;
    SlangNVVMValueHandle foreignOne = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(module.module, integerType, -INT64_C(2147483647) - 1, minimum)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(module.module, integerType, INT64_C(2147483647), maximum)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(foreignModule.module, foreignIntegerType, 1, foreignOne)));

    const SlangNVVMTypeHandle parameterTypes[] = {pointerType, integerType, integerType};
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle secondFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("latePhiKernel"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("sameModuleForeignFunction"),
        secondFunction)));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle x = nullptr;
    SlangNVVMValueHandle y = nullptr;
    SlangNVVMValueHandle secondX = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, x)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, y)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, secondFunction, 1, secondX)));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle trueBlock = nullptr;
    SlangNVVMBlockHandle falseBlock = nullptr;
    SlangNVVMBlockHandle mergeBlock = nullptr;
    SlangNVVMBlockHandle orphanBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("if.true"), trueBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("if.false"), falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("if.merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("orphan"), orphanBlock)));

    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerPhi(foreignModule.module, mergeBlock, integerType, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerPhi(module.module, mergeBlock, voidType, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, x, y, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, trueBlock, falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, trueBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, orphanBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SlangNVVMValueHandle sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerBinary(module.module, SLANG_NVVM_VALUE_OP_ADD, x, y, sum)));
    SlangNVVMValueHandle phi = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerPhi(module.module, mergeBlock, integerType, phi)));
    // Incoming validation requires the complete CFG; merge has no terminator yet.
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, x, trueBlock) == SLANG_E_INVALID_ARG);
    // The explicit target permits late phi insertion and must preserve the current insertion state.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, phi, destination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    // All blocks in the phi function are now terminated. Invalid incoming calls must not mutate it.
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, condition, trueBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, secondX, trueBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, foreignOne, trueBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, x, orphanBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, sum, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.addIntegerPhiIncoming(module.module, phi, x, trueBlock)));
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, y, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.addIntegerPhiIncoming(module.module, phi, y, falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(module.module, function)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    const Index phiIndex = assembly.indexOf("phi i32");
    const Index addIndex = assembly.indexOf("add i32");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(phiIndex >= 0);
    SLANG_CHECK(addIndex > phiIndex);
    SLANG_CHECK(storeIndex > addIndex);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("phi i32")) == 1);
    UnownedStringSlice phiLine = assembly.getUnownedSlice().tail(phiIndex);
    const Index phiLineEnd = phiLine.indexOf(toSlice("\n"));
    if (phiLineEnd >= 0)
        phiLine = phiLine.head(phiLineEnd);
    SLANG_CHECK(_countOccurrences(phiLine, toSlice("[")) == 2);
    SLANG_CHECK(phiLine.indexOf(toSlice("%if.true")) >= 0);
    SLANG_CHECK(phiLine.indexOf(toSlice("%if.false")) >= 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarFunctionOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-function"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-function-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));

    SlangNVVMTypeHandle helperType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(module.module, integerType, &integerType, 1, helperType)));
    SlangNVVMValueHandle helper = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        helperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("invalidCallHelper"),
        helper)));
    SlangNVVMValueHandle helperValue = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, helper, 0, helperValue)));

    const SlangNVVMTypeHandle callerParameterTypes[] = {integerType, integerType};
    SlangNVVMTypeHandle callerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        integerType,
        callerParameterTypes,
        SLANG_COUNT_OF(callerParameterTypes),
        callerType)));
    SlangNVVMValueHandle caller = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        callerType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("invalidCallCaller"),
        caller)));
    SlangNVVMValueHandle x = nullptr;
    SlangNVVMValueHandle y = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, caller, 0, x)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, caller, 1, y)));

    SlangNVVMTypeHandle voidFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(module.module, voidType, &integerType, 1, voidFunctionType)));
    SlangNVVMValueHandle voidFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        voidFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("invalidCallVoid"),
        voidFunction)));
    SlangNVVMValueHandle voidFunctionValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, voidFunction, 0, voidFunctionValue)));

    SlangNVVMTypeHandle foreignHelperType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignIntegerType,
        &foreignIntegerType,
        1,
        foreignHelperType)));
    SlangNVVMValueHandle foreignHelper = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignHelperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignCallHelper"),
        foreignHelper)));
    SlangNVVMValueHandle foreignValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignHelper, 0, foreignValue)));

    // This module has no insertion block yet. Both operations must reject without creating an
    // instruction or selecting function ownership implicitly.
    const SlangNVVMValueHandle noInsertionArguments[] = {x};
    SlangNVVMValueHandle noInsertionResult = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(
            module.module,
            helper,
            noInsertionArguments,
            SLANG_COUNT_OF(noInsertionArguments),
            noInsertionResult) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(noInsertionResult == nullptr);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, x) == SLANG_E_INVALID_ARG);

    SlangNVVMBlockHandle helperBlock = nullptr;
    SlangNVVMBlockHandle callerEntry = nullptr;
    SlangNVVMBlockHandle callerOther = nullptr;
    SlangNVVMBlockHandle voidBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, helper, toSlice("helper.entry"), helperBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, caller, toSlice("caller.entry"), callerEntry)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, caller, toSlice("caller.other"), callerOther)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, voidFunction, toSlice("void.entry"), voidBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, helperBlock)));
    SlangNVVMValueHandle one = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 1, one)));
    SlangNVVMValueHandle helperResult = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_VALUE_OP_ADD,
        helperValue,
        one,
        helperResult)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerReturn(module.module, helperResult)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, callerOther)));
    SlangNVVMValueHandle nonDominatingValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .emitIntegerBinary(module.module, SLANG_NVVM_VALUE_OP_ADD, x, y, nonDominatingValue)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerReturn(module.module, nonDominatingValue)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, voidBlock)));
    SLANG_CHECK(builder.emitIntegerReturn(module.module, voidFunctionValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, callerEntry)));
    SlangNVVMValueHandle condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, x, y, condition)));

    const SlangNVVMValueHandle xArgument[] = {x};
    const SlangNVVMValueHandle conditionArgument[] = {condition};
    const SlangNVVMValueHandle helperArgument[] = {helperValue};
    const SlangNVVMValueHandle foreignArgument[] = {foreignValue};
    const SlangNVVMValueHandle nonDominatingArgument[] = {nonDominatingValue};
    const SlangNVVMValueHandle tooManyArguments[] = {x, y};
    SlangNVVMValueHandle rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getConstructionAPI()->emitCall(module.module, helper, xArgument, 1, nullptr) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, x, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, foreignHelper, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, nullptr, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, nullptr, 0, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(
            module.module,
            helper,
            tooManyArguments,
            SLANG_COUNT_OF(tooManyArguments),
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, conditionArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, helperArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, foreignArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, nonDominatingArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    // Invalid valued returns must likewise leave caller.entry unterminated for the valid graph.
    SLANG_CHECK(builder.emitIntegerReturn(module.module, condition) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, helperValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, foreignValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitIntegerReturn(module.module, nonDominatingValue) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle callResult = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerCall(module.module, helper, xArgument, 1, callResult)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerReturn(module.module, callResult)));
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, x) == SLANG_E_INVALID_ARG);

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("call i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret i32")) == 3);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidPointerAddressingOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-pointer-offset"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-pointer-offset-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));

    // The opaque ABI has no aggregate/opaque type constructor, and its only unsized exposed type
    // cannot form a pointer. This pins the construction boundary without forging provider handles.
    SlangNVVMTypeHandle rejectedUnsizedPointer =
        reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getPointerType(
            module.module,
            voidType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            rejectedUnsizedPointer) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedUnsizedPointer == nullptr);

    const SlangNVVMTypeHandle parameterTypes[] = {pointerType, pointerType, integerType};
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("invalidPointerOffset"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("otherPointerOffset"),
        otherFunction)));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle source = nullptr;
    SlangNVVMValueHandle index = nullptr;
    SlangNVVMValueHandle otherDestination = nullptr;
    SlangNVVMValueHandle otherIndex = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, source)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, index)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 2, otherIndex)));

    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignIntegerType = nullptr;
    SlangNVVMTypeHandle foreignPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignPointerType)));
    const SlangNVVMTypeHandle foreignParameterTypes[] = {
        foreignPointerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SlangNVVMValueHandle foreignFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignPointerOffset"),
        foreignFunction)));
    SlangNVVMValueHandle foreignPointer = nullptr;
    SlangNVVMValueHandle foreignIndex = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignIndex)));

    auto expectRejectedOffset = [&](SlangNVVMModuleHandle targetModule,
                                    SlangNVVMValueHandle base,
                                    SlangNVVMValueHandle offset)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitPointerOffset(targetModule, base, offset, rejected) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    auto expectRejectedByteOffset = [&](SlangNVVMModuleHandle targetModule,
                                        SlangNVVMValueHandle base,
                                        SlangNVVMValueHandle offset,
                                        SlangNVVMTypeHandle pointeeType)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitByteOffsetPointer(targetModule, base, offset, pointeeType, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    // No insertion point and module ownership failures must be rejected before any instruction is
    // created or a function is inferred from the values.
    expectRejectedOffset(module.module, destination, index);
    expectRejectedOffset(nullptr, destination, index);
    expectRejectedOffset(foreignModule.module, destination, index);
    expectRejectedByteOffset(module.module, destination, index, integerType);
    expectRejectedByteOffset(nullptr, destination, index, integerType);
    expectRejectedByteOffset(foreignModule.module, destination, index, integerType);

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle producerBlock = nullptr;
    SlangNVVMBlockHandle consumerBlock = nullptr;
    SlangNVVMBlockHandle mergeBlock = nullptr;
    SlangNVVMBlockHandle otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, index, index, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle producerPointer = nullptr;
    SlangNVVMValueHandle producerInteger = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitPointerOffset(module.module, source, index, producerPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_VALUE_OP_ADD,
        index,
        index,
        producerInteger)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        builder.getConstructionAPI()
            ->emitPointerOffset(module.module, destination, index, nullptr) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.getConstructionAPI()
            ->emitByteOffsetPointer(module.module, destination, index, integerType, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejectedOffset(module.module, index, index);
    expectRejectedOffset(module.module, destination, source);
    expectRejectedOffset(module.module, foreignPointer, index);
    expectRejectedOffset(module.module, destination, foreignIndex);
    expectRejectedOffset(module.module, otherDestination, index);
    expectRejectedOffset(module.module, destination, otherIndex);
    expectRejectedOffset(module.module, producerPointer, index);
    expectRejectedOffset(module.module, destination, producerInteger);
    expectRejectedByteOffset(module.module, index, index, integerType);
    expectRejectedByteOffset(module.module, destination, source, integerType);
    expectRejectedByteOffset(module.module, foreignPointer, index, integerType);
    expectRejectedByteOffset(module.module, destination, foreignIndex, integerType);
    expectRejectedByteOffset(module.module, otherDestination, index, integerType);
    expectRejectedByteOffset(module.module, destination, otherIndex, integerType);
    expectRejectedByteOffset(module.module, producerPointer, index, integerType);
    expectRejectedByteOffset(module.module, destination, producerInteger, integerType);
    expectRejectedByteOffset(module.module, destination, index, foreignIntegerType);
    expectRejectedByteOffset(module.module, destination, index, voidType);

    SlangNVVMValueHandle consumerPointer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitPointerOffset(module.module, destination, index, consumerPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejectedOffset(module.module, destination, index);
    expectRejectedByteOffset(module.module, destination, index, integerType);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr i32")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidSequentialAddressingOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-sequential-addressing"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(
        toSlice("invalid-sequential-addressing-foreign"),
        foreignModule.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle arrayType = nullptr;
    SlangNVVMTypeHandle arrayPointerType = nullptr;
    SlangNVVMTypeHandle scalarPointerType = nullptr;
    SlangNVVMTypeHandle vectorType = nullptr;
    SlangNVVMTypeHandle vectorPointerType = nullptr;
    SlangNVVMTypeHandle foreignIntegerType = nullptr;
    SlangNVVMTypeHandle foreignArrayType = nullptr;
    SlangNVVMTypeHandle foreignArrayPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getArrayType(module.module, integerType, 4, arrayType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        arrayType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        arrayPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        scalarPointerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getVectorType(module.module, integerType, 4, vectorType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        vectorType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        vectorPointerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getArrayType(foreignModule.module, foreignIntegerType, 4, foreignArrayType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignArrayType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignArrayPointerType)));

    SLANG_CHECK(
        builder.getConstructionAPI()->getArrayType(module.module, integerType, 4, nullptr) ==
        SLANG_E_INVALID_ARG);
    SlangNVVMTypeHandle rawRejectedType = reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getConstructionAPI()->getArrayType(module.module, voidType, 4, &rawRejectedType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejectedType == nullptr);
    auto expectRejectedArrayType =
        [&](SlangNVVMModuleHandle targetModule, SlangNVVMTypeHandle elementType, uint32_t count)
    {
        SlangNVVMTypeHandle rejected = reinterpret_cast<SlangNVVMTypeHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.getArrayType(targetModule, elementType, count, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    expectRejectedArrayType(nullptr, integerType, 4);
    expectRejectedArrayType(foreignModule.module, integerType, 4);
    expectRejectedArrayType(module.module, foreignIntegerType, 4);
    // Void is the only unsized type exposed by this provider ABI.
    expectRejectedArrayType(module.module, voidType, 4);
    expectRejectedArrayType(module.module, integerType, 0);

    const SlangNVVMTypeHandle parameterTypes[] = {
        arrayPointerType,
        arrayPointerType,
        scalarPointerType,
        integerType,
        vectorPointerType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("invalidSequentialAddressing"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("otherSequentialAddressing"),
        otherFunction)));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle source = nullptr;
    SlangNVVMValueHandle scalarPointer = nullptr;
    SlangNVVMValueHandle index = nullptr;
    SlangNVVMValueHandle vectorPointer = nullptr;
    SlangNVVMValueHandle otherDestination = nullptr;
    SlangNVVMValueHandle otherIndex = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, source)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, scalarPointer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 3, index)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 4, vectorPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 3, otherIndex)));

    const SlangNVVMTypeHandle foreignParameterTypes[] = {
        foreignArrayPointerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SlangNVVMValueHandle foreignFunction = nullptr;
    SlangNVVMValueHandle foreignBase = nullptr;
    SlangNVVMValueHandle foreignIndex = nullptr;
    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignSequentialAddressing"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignBase)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignIndex)));

    auto expectRejectedElement = [&](SlangNVVMModuleHandle targetModule,
                                     SlangNVVMValueHandle base,
                                     SlangNVVMValueHandle elementIndex)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitSequentialElementPointer(targetModule, base, elementIndex, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    expectRejectedElement(module.module, destination, index);
    SlangNVVMValueHandle rawRejectedElement = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.getConstructionAPI()->emitSequentialElementPointer(
            module.module,
            destination,
            index,
            &rawRejectedElement) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejectedElement == nullptr);
    expectRejectedElement(nullptr, destination, index);
    expectRejectedElement(foreignModule.module, destination, index);
    expectRejectedElement(module.module, nullptr, index);
    expectRejectedElement(module.module, destination, nullptr);

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle producerBlock = nullptr;
    SlangNVVMBlockHandle consumerBlock = nullptr;
    SlangNVVMBlockHandle mergeBlock = nullptr;
    SlangNVVMBlockHandle otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, index, index, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle nonDominatingIndex = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_VALUE_OP_ADD,
        index,
        index,
        nonDominatingIndex)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        builder.getConstructionAPI()
            ->emitSequentialElementPointer(module.module, destination, index, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejectedElement(module.module, scalarPointer, index);
    expectRejectedElement(module.module, destination, source);
    expectRejectedElement(module.module, foreignBase, index);
    expectRejectedElement(module.module, destination, foreignIndex);
    expectRejectedElement(module.module, otherDestination, index);
    expectRejectedElement(module.module, destination, otherIndex);
    expectRejectedElement(module.module, destination, nonDominatingIndex);

    SlangNVVMValueHandle destinationElement = nullptr;
    SlangNVVMValueHandle sourceElement = nullptr;
    SlangNVVMValueHandle vectorElement = nullptr;
    SlangNVVMValueHandle ordinaryValue = nullptr;
    SlangNVVMValueHandle invariantValue = nullptr;
    SlangNVVMValueHandle vectorValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .emitSequentialElementPointer(module.module, destination, index, destinationElement)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitSequentialElementPointer(module.module, source, index, sourceElement)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitSequentialElementPointer(module.module, vectorPointer, index, vectorElement)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .emitLoad(module.module, sourceElement, 4, SLANG_NVVM_LOAD_FLAG_NONE, ordinaryValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitLoad(
        module.module,
        sourceElement,
        4,
        SLANG_NVVM_LOAD_FLAG_INVARIANT,
        invariantValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitLoad(module.module, vectorElement, 4, SLANG_NVVM_LOAD_FLAG_NONE, vectorValue)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(module.module, invariantValue, destinationElement, 4)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(module.module, vectorValue, destinationElement, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejectedElement(module.module, destination, index);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr [4 x i32]")) == 2);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr <4 x i32>")) == 1);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("i32 0, i32 %slangParameter3")) == 3);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("load i32")) == 3);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("!invariant.load")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsGenericAggregateValues)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule module;
    ScopedNVVMBuilderModule foreignModule;
    module.builder = &builder;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("generic-aggregate-values"), module.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("generic-aggregate-values-foreign"), foreignModule.module)));

    auto getTypes = [&](SlangNVVMModuleHandle targetModule,
                        SlangNVVMTypeHandle& outVoidType,
                        SlangNVVMTypeHandle& outFloatType,
                        SlangNVVMTypeHandle& outFloat2Type,
                        SlangNVVMTypeHandle& outArrayType)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(targetModule, outVoidType)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFloatingPointType(targetModule, 32, outFloatType)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getVectorType(targetModule, outFloatType, 2, outFloat2Type)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getArrayType(targetModule, outFloat2Type, 2, outArrayType)));
    };

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle i32Type = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle float2Type = nullptr;
    SlangNVVMTypeHandle arrayType = nullptr;
    getTypes(module.module, voidType, floatType, float2Type, arrayType);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, i32Type)));
    const SlangNVVMTypeHandle parameterTypes[] = {float2Type, float2Type, floatType, i32Type};
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("genericAggregateValues"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("otherGenericAggregateValues"),
        otherFunction)));

    SlangNVVMValueHandle firstRow = nullptr;
    SlangNVVMValueHandle secondRow = nullptr;
    SlangNVVMValueHandle scalar = nullptr;
    SlangNVVMValueHandle dynamicIndex = nullptr;
    SlangNVVMValueHandle otherFirstRow = nullptr;
    SlangNVVMValueHandle otherSecondRow = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, firstRow)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, secondRow)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, scalar)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 3, dynamicIndex)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherFirstRow)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 1, otherSecondRow)));

    SlangNVVMBlockHandle otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    const SlangNVVMValueHandle otherRows[] = {otherFirstRow, otherSecondRow};
    SlangNVVMValueHandle otherAggregate = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAggregateConstruct(
        module.module,
        arrayType,
        otherRows,
        SLANG_COUNT_OF(otherRows),
        otherAggregate)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignFloatType = nullptr;
    SlangNVVMTypeHandle foreignFloat2Type = nullptr;
    SlangNVVMTypeHandle foreignArrayType = nullptr;
    getTypes(
        foreignModule.module,
        foreignVoidType,
        foreignFloatType,
        foreignFloat2Type,
        foreignArrayType);
    const SlangNVVMTypeHandle foreignParameterTypes[] = {foreignFloat2Type, foreignFloat2Type};
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SlangNVVMValueHandle foreignFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignGenericAggregateValues"),
        foreignFunction)));
    SlangNVVMValueHandle foreignFirstRow = nullptr;
    SlangNVVMValueHandle foreignSecondRow = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignFirstRow)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignSecondRow)));
    SlangNVVMBlockHandle foreignBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .createBlock(foreignModule.module, foreignFunction, toSlice("entry"), foreignBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(foreignModule.module, foreignBlock)));
    const SlangNVVMValueHandle foreignRows[] = {foreignFirstRow, foreignSecondRow};
    SlangNVVMValueHandle foreignAggregate = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAggregateConstruct(
        foreignModule.module,
        foreignArrayType,
        foreignRows,
        SLANG_COUNT_OF(foreignRows),
        foreignAggregate)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(foreignModule.module)));

    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(module.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));
    const SlangNVVMValueHandle rows[] = {firstRow, secondRow};
    const SlangNVVMValueHandle wrongRows[] = {firstRow, scalar};

    auto expectRejectedConstruction = [&](SlangNVVMModuleHandle targetModule,
                                          SlangNVVMTypeHandle targetType,
                                          const SlangNVVMValueHandle* elements,
                                          size_t elementCount)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitAggregateConstruct(
                targetModule,
                targetType,
                elements,
                elementCount,
                rejected) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    SLANG_CHECK(
        builder.getConstructionAPI()->emitAggregateConstruct(
            module.module,
            arrayType,
            rows,
            SLANG_COUNT_OF(rows),
            nullptr) == SLANG_E_INVALID_ARG);
    expectRejectedConstruction(nullptr, arrayType, rows, SLANG_COUNT_OF(rows));
    expectRejectedConstruction(foreignModule.module, arrayType, rows, SLANG_COUNT_OF(rows));
    expectRejectedConstruction(module.module, nullptr, rows, SLANG_COUNT_OF(rows));
    expectRejectedConstruction(module.module, float2Type, rows, SLANG_COUNT_OF(rows));
    expectRejectedConstruction(module.module, arrayType, nullptr, SLANG_COUNT_OF(rows));
    expectRejectedConstruction(module.module, arrayType, rows, 1);
    expectRejectedConstruction(module.module, arrayType, wrongRows, SLANG_COUNT_OF(wrongRows));
    expectRejectedConstruction(module.module, foreignArrayType, rows, SLANG_COUNT_OF(rows));
    expectRejectedConstruction(module.module, arrayType, foreignRows, SLANG_COUNT_OF(foreignRows));
    expectRejectedConstruction(module.module, arrayType, otherRows, SLANG_COUNT_OF(otherRows));

    SlangNVVMValueHandle aggregate = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAggregateConstruct(
        module.module,
        arrayType,
        rows,
        SLANG_COUNT_OF(rows),
        aggregate)));

    auto expectRejectedExtraction = [&](SlangNVVMModuleHandle targetModule,
                                        SlangNVVMValueHandle targetValue,
                                        uint32_t elementIndex)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder
                .emitAggregateElementExtract(targetModule, targetValue, elementIndex, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    SLANG_CHECK(
        builder.getConstructionAPI()
            ->emitAggregateElementExtract(module.module, aggregate, 0, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejectedExtraction(nullptr, aggregate, 0);
    expectRejectedExtraction(foreignModule.module, aggregate, 0);
    expectRejectedExtraction(module.module, nullptr, 0);
    expectRejectedExtraction(module.module, firstRow, 0);
    expectRejectedExtraction(module.module, aggregate, 2);
    expectRejectedExtraction(module.module, otherAggregate, 0);
    expectRejectedExtraction(module.module, foreignAggregate, 0);

    SlangNVVMValueHandle extractedRow = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitAggregateElementExtract(module.module, aggregate, 1, extractedRow)));
    SlangNVVMValueHandle dynamicallyExtractedRow = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitSequentialElementExtract(
        module.module,
        aggregate,
        dynamicIndex,
        dynamicallyExtractedRow)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    String diagnostics;
    ComPtr<ISlangBlob> assemblyBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("insertvalue")) == 4);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("extractvalue")) == 3);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("select")) == 2);
    SLANG_CHECK(assembly.indexOf("poison") < 0);

    expectRejectedConstruction(module.module, arrayType, rows, SLANG_COUNT_OF(rows));
    expectRejectedExtraction(module.module, aggregate, 0);
    ComPtr<ISlangBlob> afterTerminationBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        afterTerminationBlob,
        diagnostics)));
    SLANG_CHECK(_getBlobText(afterTerminationBlob) == assembly);

    ComPtr<ISlangBlob> compatibleBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        compatibleBlob,
        diagnostics)));
    const String compatible = _getBlobText(compatibleBlob);
    SLANG_CHECK(_countOccurrences(compatible.getUnownedSlice(), toSlice("insertvalue")) == 4);
    SLANG_CHECK(_countOccurrences(compatible.getUnownedSlice(), toSlice("extractvalue")) == 3);
    SLANG_CHECK(_countOccurrences(compatible.getUnownedSlice(), toSlice("select")) == 2);
    SLANG_CHECK(compatible.indexOf("poison") < 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidAggregateElementOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule module;
    ScopedNVVMBuilderModule foreignModule;
    module.builder = &builder;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-aggregate-element"), module.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-aggregate-element-foreign"), foreignModule.module)));

    auto makeResourceType = [&](SlangNVVMModuleHandle targetModule,
                                SlangNVVMTypeHandle& outVoidType,
                                SlangNVVMTypeHandle& outIntegerType,
                                SlangNVVMTypeHandle& outResourceType)
    {
        SlangNVVMTypeHandle countType = nullptr;
        SlangNVVMTypeHandle dataPointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(targetModule, outVoidType)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getIntegerType(targetModule, 32, outIntegerType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(targetModule, 64, countType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            targetModule,
            outIntegerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            dataPointerType)));
        const SlangNVVMTypeHandle resourceFieldTypes[] = {dataPointerType, countType};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getStructType(
            targetModule,
            resourceFieldTypes,
            SLANG_COUNT_OF(resourceFieldTypes),
            outResourceType)));
    };

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle resourceType = nullptr;
    makeResourceType(module.module, voidType, integerType, resourceType);
    const SlangNVVMTypeHandle parameterTypes[] = {resourceType, integerType};
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("invalidAggregateElement"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("otherAggregateElement"),
        otherFunction)));

    SlangNVVMValueHandle buffer = nullptr;
    SlangNVVMValueHandle index = nullptr;
    SlangNVVMValueHandle otherBuffer = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, buffer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, index)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherBuffer)));

    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignIntegerType = nullptr;
    SlangNVVMTypeHandle foreignResourceType = nullptr;
    makeResourceType(
        foreignModule.module,
        foreignVoidType,
        foreignIntegerType,
        foreignResourceType);
    const SlangNVVMTypeHandle foreignParameterTypes[] = {
        foreignResourceType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SlangNVVMValueHandle foreignFunction = nullptr;
    SlangNVVMValueHandle foreignBuffer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignAggregateElement"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignBuffer)));

    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(module.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));

    auto expectRejected = [&](SlangNVVMModuleHandle targetModule,
                              SlangNVVMValueHandle targetValue,
                              uint32_t fieldIndex)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitAggregateElementExtract(targetModule, targetValue, fieldIndex, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    SLANG_CHECK(
        builder.getConstructionAPI()
            ->emitAggregateElementExtract(module.module, buffer, 0, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejected(nullptr, buffer, 0);
    expectRejected(foreignModule.module, buffer, 0);
    expectRejected(module.module, nullptr, 0);
    expectRejected(module.module, index, 0);
    expectRejected(module.module, buffer, 2);
    expectRejected(module.module, otherBuffer, 0);
    expectRejected(module.module, foreignBuffer, 0);

    SlangNVVMValueHandle dataPointer = nullptr;
    SlangNVVMValueHandle elementPointer = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitAggregateElementExtract(module.module, buffer, 0, dataPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitPointerOffset(module.module, dataPointer, index, elementPointer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 42, value)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, value, elementPointer, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    String diagnostics;
    ComPtr<ISlangBlob> completeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        completeBlob,
        diagnostics)));
    const String complete = _getBlobText(completeBlob);
    expectRejected(module.module, buffer, 0);
    ComPtr<ISlangBlob> afterTerminatedBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        afterTerminatedBlob,
        diagnostics)));
    SLANG_CHECK(_getBlobText(afterTerminatedBlob) == complete);

    SLANG_CHECK(
        complete.indexOf("define void @invalidAggregateElement({ i32 addrspace(1)*, i64 } "
                         "%slangParameter0, i32 %slangParameter1)") >= 0);
    SLANG_CHECK(_countOccurrences(complete.getUnownedSlice(), toSlice("extractvalue")) == 1);
    SLANG_CHECK(_countOccurrences(complete.getUnownedSlice(), toSlice("getelementptr i32")) == 1);
    SLANG_CHECK(complete.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(_countOccurrences(complete.getUnownedSlice(), toSlice("store i32 42")) == 1);
}

static SlangResult _emitRawNVVMScalarBuilderOperation(
    const SlangNVVMBuilderValueOperationsAPI* api,
    NVVMScalarTestOperation operation,
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    if (!api)
        return SLANG_E_INVALID_ARG;
    const SlangNVVMValueTypeDesc boolType = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, 1};
    const SlangNVVMValueTypeDesc signedI32 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        32,
        1,
    };
    const SlangNVVMValueTypeDesc operandTypes[] = {signedI32, signedI32};
    SlangNVVMValueOperationDesc operationDesc = {
        SLANG_NVVM_VALUE_OP_ADD,
        signedI32,
        operandTypes,
        2,
    };
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_MULTIPLY;
        break;
    case NVVMScalarTestOperation::BitAnd:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_BIT_AND;
        break;
    case NVVMScalarTestOperation::BitOr:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_BIT_OR;
        break;
    case NVVMScalarTestOperation::BitXor:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_BIT_XOR;
        break;
    case NVVMScalarTestOperation::BitNot:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_BIT_NOT;
        operationDesc.operandCount = 1;
        break;
    case NVVMScalarTestOperation::Negate:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_NEGATE;
        operationDesc.operandCount = 1;
        break;
    case NVVMScalarTestOperation::Equal:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_EQUAL;
        operationDesc.resultType = boolType;
        break;
    case NVVMScalarTestOperation::NotEqual:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_NOT_EQUAL;
        operationDesc.resultType = boolType;
        break;
    case NVVMScalarTestOperation::SignedGreaterThan:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_GREATER_THAN;
        operationDesc.resultType = boolType;
        break;
    case NVVMScalarTestOperation::SignedLessEqual:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_LESS_EQUAL;
        operationDesc.resultType = boolType;
        break;
    case NVVMScalarTestOperation::SignedGreaterEqual:
        operationDesc.operation = SLANG_NVVM_VALUE_OP_GREATER_EQUAL;
        operationDesc.resultType = boolType;
        break;
    }
    const SlangNVVMValueHandle operands[] = {left, right};
    return api
        ->emitOperation(module, &operationDesc, operands, operationDesc.operandCount, outValue);
}

static void _runNVVMScalarInvalidOperations(
    UnitTestContext* unitTestContext,
    NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    const bool isUnary = testCase.key.family == FakeNVVMBuilderScalarFamily::Unary;
    const bool isCompare = testCase.key.family == FakeNVVMBuilderScalarFamily::Compare;

    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(_supportsNVVMScalarBuilderOperation(builder, operation));

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-operation"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-operation-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle wideIntegerType = nullptr;
    SlangNVVMTypeHandle pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, wideIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));

    const SlangNVVMTypeHandle parameterTypes[] = {
        pointerType,
        integerType,
        integerType,
        wideIntegerType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("invalidScalarOperation"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("otherScalarOperation"),
        otherFunction)));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle left = nullptr;
    SlangNVVMValueHandle right = nullptr;
    SlangNVVMValueHandle wide = nullptr;
    SlangNVVMValueHandle otherLeft = nullptr;
    SlangNVVMValueHandle otherRight = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, left)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, right)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 3, wide)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 1, otherLeft)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 2, otherRight)));

    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    const SlangNVVMTypeHandle foreignParameterTypes[] = {
        foreignIntegerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SlangNVVMValueHandle foreignFunction = nullptr;
    SlangNVVMValueHandle foreignLeft = nullptr;
    SlangNVVMValueHandle foreignRight = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignScalarOperation"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignLeft)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignRight)));

    auto expectRejected = [&](SlangNVVMModuleHandle targetModule,
                              SlangNVVMValueHandle candidateLeft,
                              SlangNVVMValueHandle candidateRight)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            _emitNVVMScalarBuilderOperation(
                builder,
                operation,
                targetModule,
                candidateLeft,
                candidateRight,
                rejected) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    expectRejected(module.module, left, right);
    SlangNVVMValueHandle rawRejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        _emitRawNVVMScalarBuilderOperation(
            builder.getValueOperationsAPI(),
            operation,
            module.module,
            left,
            right,
            &rawRejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejected == nullptr);
    expectRejected(nullptr, left, right);
    expectRejected(foreignModule.module, left, right);

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle producerBlock = nullptr;
    SlangNVVMBlockHandle consumerBlock = nullptr;
    SlangNVVMBlockHandle trueBlock = nullptr;
    SlangNVVMBlockHandle falseBlock = nullptr;
    SlangNVVMBlockHandle mergeBlock = nullptr;
    SlangNVVMBlockHandle otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    if (isCompare)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(module.module, function, toSlice("true"), trueBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(module.module, function, toSlice("false"), falseBlock)));
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SlangNVVMValueHandle zero = nullptr;
    SlangNVVMValueHandle one = nullptr;
    if (isCompare)
    {
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 0, zero)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 1, one)));
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle scaffoldCondition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerSignedLessThan(module.module, left, right, scaffoldCondition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitConditionalBranch(
        module.module,
        scaffoldCondition,
        producerBlock,
        consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle nonDominating = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_VALUE_OP_ADD,
        left,
        right,
        nonDominating)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        _emitRawNVVMScalarBuilderOperation(
            builder.getValueOperationsAPI(),
            operation,
            module.module,
            left,
            right,
            nullptr) == SLANG_E_INVALID_ARG);
    expectRejected(module.module, nullptr, right);
    if (!isUnary)
        expectRejected(module.module, left, nullptr);
    expectRejected(module.module, destination, right);
    if (!isUnary)
        expectRejected(module.module, left, destination);
    if (!isUnary)
        expectRejected(module.module, wide, right);
    if (!isUnary)
        expectRejected(module.module, left, wide);
    expectRejected(module.module, foreignLeft, right);
    if (!isUnary)
        expectRejected(module.module, left, foreignRight);
    expectRejected(module.module, otherLeft, right);
    if (!isUnary)
        expectRejected(module.module, left, otherRight);
    expectRejected(module.module, nonDominating, right);
    if (!isUnary)
        expectRejected(module.module, left, nonDominating);

    SlangNVVMValueHandle value = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _emitNVVMScalarBuilderOperation(builder, operation, module.module, left, right, value)));
    if (isCompare)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitConditionalBranch(module.module, value, trueBlock, falseBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, trueBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, one, destination, 4)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, falseBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, zero, destination, 4)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    }
    else
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, value, destination, 4)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejected(module.module, left, right);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    StringBuilder instruction32;
    instruction32 << testCase.llvmOpcode << " i32";
    StringBuilder instruction64;
    instruction64 << testCase.llvmOpcode << " i64";
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), instruction32.getUnownedSlice()) == 1);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), instruction64.getUnownedSlice()) == 0);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == (isCompare ? 2 : 1));
    const Index operationIndex = assembly.indexOf(instruction32.getUnownedSlice());
    SLANG_CHECK_ABORT(operationIndex >= 0);
    if (isCompare)
    {
        const Index conditionalIndex =
            assembly.getUnownedSlice().tail(operationIndex).indexOf(toSlice("br i1"));
        SLANG_CHECK(conditionalIndex > 0);
    }
    else
    {
        const Index storeIndex = assembly.indexOf("store i32");
        SLANG_CHECK(storeIndex > operationIndex);
    }
}

#define NVVM_SCALAR_INVALID_TEST(NAME, OPERATION)                                             \
    SLANG_UNIT_TEST(NAME)                                                                     \
    {                                                                                         \
        _runNVVMScalarInvalidOperations(unitTestContext, NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerMultiplyOperations, Multiply)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerBitAndOperations, BitAnd)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerBitOrOperations, BitOr)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerBitXorOperations, BitXor)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerBitNotOperations, BitNot)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerNegateOperations, Negate)
SLANG_UNIT_TEST(nvvmIRBuilderValidatesAtomicOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-relaxed-global-i32-atomic-add"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(
        toSlice("invalid-relaxed-global-i32-atomic-add-foreign"),
        foreignModule.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle i32Type = nullptr;
    SlangNVVMTypeHandle i64Type = nullptr;
    SlangNVVMTypeHandle globalI32PointerType = nullptr;
    SlangNVVMTypeHandle sharedI32PointerType = nullptr;
    SlangNVVMTypeHandle constantI32PointerType = nullptr;
    SlangNVVMTypeHandle genericI32PointerType = nullptr;
    SlangNVVMTypeHandle localI32PointerType = nullptr;
    SlangNVVMTypeHandle globalI64PointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, i32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, i64Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_SHARED,
        sharedI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_CONSTANT,
        constantI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_GENERIC,
        genericI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_LOCAL,
        localI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i64Type,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalI64PointerType)));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalI32PointerType,
        globalI32PointerType,
        i32Type,
        sharedI32PointerType,
        constantI32PointerType,
        genericI32PointerType,
        localI32PointerType,
        globalI64PointerType,
        i64Type,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("rejectInvalidRelaxedGlobalI32AtomicAdd"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("otherRelaxedGlobalI32AtomicAdd"),
        otherFunction)));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle oldValueDestination = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SlangNVVMValueHandle sharedDestination = nullptr;
    SlangNVVMValueHandle constantDestination = nullptr;
    SlangNVVMValueHandle genericDestination = nullptr;
    SlangNVVMValueHandle localDestination = nullptr;
    SlangNVVMValueHandle wideDestination = nullptr;
    SlangNVVMValueHandle wideValue = nullptr;
    SlangNVVMValueHandle otherDestination = nullptr;
    SlangNVVMValueHandle otherValue = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 1, oldValueDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, value)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 3, sharedDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 4, constantDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 5, genericDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 6, localDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 7, wideDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 8, wideValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 2, otherValue)));

    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignI32Type = nullptr;
    SlangNVVMTypeHandle foreignGlobalI32PointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignI32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignI32Type,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignGlobalI32PointerType)));
    const SlangNVVMTypeHandle foreignParameterTypes[] = {
        foreignGlobalI32PointerType,
        foreignI32Type,
    };
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SlangNVVMValueHandle foreignFunction = nullptr;
    SlangNVVMValueHandle foreignDestination = nullptr;
    SlangNVVMValueHandle foreignValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignRelaxedGlobalI32AtomicAdd"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignValue)));
    SlangNVVMBlockHandle foreignBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .createBlock(foreignModule.module, foreignFunction, toSlice("entry"), foreignBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(foreignModule.module, foreignBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(foreignModule.module)));

    const SlangNVVMAtomicOperationDesc atomicOperation = {
        SLANG_NVVM_ATOMIC_OP_ADD,
        NVVMSemantics::kSignedI32,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        SLANG_NVVM_MEMORY_ORDER_RELAXED,
    };
    SLANG_CHECK(builder.supportsAtomicOperation(atomicOperation));
    SlangNVVMAtomicOperationDesc unsignedAtomicOperation = atomicOperation;
    unsignedAtomicOperation.valueType = NVVMSemantics::kUnsignedI32;
    SLANG_CHECK(builder.supportsAtomicOperation(unsignedAtomicOperation));
    SlangNVVMAtomicOperationDesc sharedAtomicOperation = atomicOperation;
    sharedAtomicOperation.addressSpace = SLANG_NVVM_ADDRESS_SPACE_SHARED;
    SLANG_CHECK(builder.supportsAtomicOperation(sharedAtomicOperation));
    SlangNVVMAtomicOperationDesc unsignedSharedAtomicOperation = unsignedAtomicOperation;
    unsignedSharedAtomicOperation.addressSpace = SLANG_NVVM_ADDRESS_SPACE_SHARED;
    SLANG_CHECK(builder.supportsAtomicOperation(unsignedSharedAtomicOperation));
    const SlangNVVMAtomicOperationDesc unsignedWideMaxOperation = {
        SLANG_NVVM_ATOMIC_OP_MAX,
        NVVMSemantics::kUnsignedI64,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        SLANG_NVVM_MEMORY_ORDER_RELAXED,
    };
    SLANG_CHECK(builder.supportsAtomicOperation(unsignedWideMaxOperation));
    SlangNVVMAtomicOperationDesc unsupportedWideMaxOperation = unsignedWideMaxOperation;
    unsupportedWideMaxOperation.valueType.kind = SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedWideMaxOperation));
    unsupportedWideMaxOperation = unsignedWideMaxOperation;
    unsupportedWideMaxOperation.valueType.bitWidth = 32;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedWideMaxOperation));
    unsupportedWideMaxOperation = unsignedWideMaxOperation;
    unsupportedWideMaxOperation.addressSpace = SLANG_NVVM_ADDRESS_SPACE_SHARED;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedWideMaxOperation));
    unsupportedWideMaxOperation = unsignedWideMaxOperation;
    unsupportedWideMaxOperation.memoryOrder = SLANG_NVVM_MEMORY_ORDER_ACQUIRE;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedWideMaxOperation));

    SlangNVVMAtomicOperationDesc unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.operation = SLANG_NVVM_ATOMIC_OP_SUBTRACT;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.operation =
        SlangNVVMAtomicOperation(SLANG_NVVM_ATOMIC_OPERATION_COUNT);
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.valueType.bitWidth = 64;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.valueType.laneCount = 2;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.valueType.kind = SLANG_NVVM_VALUE_TYPE_FLOATING_POINT;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.addressSpace = SlangNVVMAddressSpace(99);
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.memoryOrder = SLANG_NVVM_MEMORY_ORDER_ACQUIRE;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.memoryOrder = SlangNVVMMemoryOrder(SLANG_NVVM_MEMORY_ORDER_COUNT);
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));

    auto expectRejected = [&](SlangNVVMModuleHandle targetModule,
                              SlangNVVMValueHandle pointer,
                              SlangNVVMValueHandle addend)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitAtomicOperation(targetModule, atomicOperation, pointer, addend, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    // No insertion point exists for the selected function. Rejections must not infer ownership or
    // create an atomic instruction in some other function's current block.
    expectRejected(module.module, destination, value);
    expectRejected(nullptr, destination, value);
    expectRejected(foreignModule.module, destination, value);

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle producerBlock = nullptr;
    SlangNVVMBlockHandle consumerBlock = nullptr;
    SlangNVVMBlockHandle mergeBlock = nullptr;
    SlangNVVMBlockHandle otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, value, value, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle nonDominatingValue = nullptr;
    SlangNVVMValueHandle nonDominatingPointer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_VALUE_OP_ADD,
        value,
        value,
        nonDominatingValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitPointerOffset(module.module, destination, value, nonDominatingPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    const SlangNVVMBuilderAtomicOperationsAPI* api = builder.getAtomicOperationsAPI();
    SLANG_CHECK_ABORT(api != nullptr);
    SLANG_CHECK(
        api->emitOperation(module.module, &atomicOperation, destination, value, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejected(module.module, nullptr, value);
    expectRejected(module.module, destination, nullptr);
    expectRejected(module.module, value, value);
    expectRejected(module.module, destination, destination);
    expectRejected(module.module, sharedDestination, value);
    expectRejected(module.module, constantDestination, value);
    expectRejected(module.module, genericDestination, value);
    expectRejected(module.module, localDestination, value);
    expectRejected(module.module, wideDestination, value);
    expectRejected(module.module, destination, wideValue);
    expectRejected(module.module, otherDestination, value);
    expectRejected(module.module, destination, otherValue);
    expectRejected(module.module, foreignDestination, value);
    expectRejected(module.module, destination, foreignValue);
    expectRejected(module.module, nonDominatingPointer, value);
    expectRejected(module.module, destination, nonDominatingValue);

    SlangNVVMValueHandle oldValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitAtomicOperation(module.module, atomicOperation, destination, value, oldValue)));
    SlangNVVMValueHandle sharedOldValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        sharedAtomicOperation,
        sharedDestination,
        value,
        sharedOldValue)));
    SlangNVVMValueHandle wideOldValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        unsignedWideMaxOperation,
        wideDestination,
        wideValue,
        wideOldValue)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(module.module, oldValue, oldValueDestination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejected(module.module, destination, value);

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    const UnownedStringSlice assemblySlice = assembly.getUnownedSlice();
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw add i32 addrspace(1)*")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw add i32 addrspace(3)*")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw umax i64 addrspace(1)*")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw add i64")) == 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("monotonic")) == 3);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("align 4")) == 3);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("align 8")) == 1);
    SLANG_CHECK(assembly.indexOf("monotonic, align 4") >= 0);
    SLANG_CHECK(assembly.indexOf("syncscope(") < 0);
    SLANG_CHECK(assembly.indexOf("acquire") < 0);
    SLANG_CHECK(assembly.indexOf("release") < 0);
    SLANG_CHECK(assembly.indexOf("seq_cst") < 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("store i32")) == 1);
    const Index atomicIndex = assembly.indexOf("atomicrmw add i32 addrspace(1)*");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(atomicIndex >= 0);
    SLANG_CHECK(storeIndex > atomicIndex);

    ComPtr<ISlangBlob> compatibleAssemblyBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        compatibleAssemblyBlob,
        diagnostics)));
    const String compatibleAssembly = _getBlobText(compatibleAssemblyBlob);
    SLANG_CHECK(compatibleAssembly.indexOf("atomicrmw umax i64 addrspace(1)*") >= 0);
    SLANG_CHECK(compatibleAssembly.indexOf("monotonic, align 4") < 0);
    SLANG_CHECK(compatibleAssembly.indexOf("monotonic, align 8") < 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsIntegerBitOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    const uint32_t bitWidths[] = {8, 16, 32, 64};
    SlangNVVMValueTypeDesc integerTypes[SLANG_COUNT_OF(bitWidths)] = {};
    for (Index i = 0; i < SLANG_COUNT_OF(bitWidths); ++i)
    {
        integerTypes[i] = {
            SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
            bitWidths[i],
            1,
        };
        const SlangNVVMValueTypeDesc operandTypes[] = {integerTypes[i]};
        const SlangNVVMValueOperation operations[] = {
            SLANG_NVVM_VALUE_OP_COUNT_BITS,
            SLANG_NVVM_VALUE_OP_REVERSE_BITS,
            SLANG_NVVM_VALUE_OP_FIRST_BIT_HIGH,
            SLANG_NVVM_VALUE_OP_FIRST_BIT_LOW,
        };
        for (SlangNVVMValueOperation operation : operations)
        {
            const SlangNVVMValueOperationDesc desc = {
                operation,
                operation == SLANG_NVVM_VALUE_OP_REVERSE_BITS ? integerTypes[i]
                                                              : NVVMSemantics::kUnsignedI32,
                operandTypes,
                SLANG_COUNT_OF(operandTypes),
            };
            SLANG_CHECK(builder.supportsValueOperation(desc));
        }
    }

    SlangNVVMValueTypeDesc vectorInteger = integerTypes[2];
    vectorInteger.laneCount = 2;
    const SlangNVVMValueTypeDesc vectorOperandTypes[] = {vectorInteger};
    const SlangNVVMValueOperationDesc unsupportedVectorCount = {
        SLANG_NVVM_VALUE_OP_COUNT_BITS,
        NVVMSemantics::kUnsignedI32,
        vectorOperandTypes,
        SLANG_COUNT_OF(vectorOperandTypes),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedVectorCount));
    const SlangNVVMValueTypeDesc wrongResultOperandTypes[] = {integerTypes[2]};
    const SlangNVVMValueOperationDesc unsupportedWrongCountResult = {
        SLANG_NVVM_VALUE_OP_COUNT_BITS,
        integerTypes[2],
        wrongResultOperandTypes,
        SLANG_COUNT_OF(wrongResultOperandTypes),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedWrongCountResult));
    const SlangNVVMValueTypeDesc signedI24 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        24,
        1,
    };
    const SlangNVVMValueTypeDesc unsupportedWidthOperands[] = {signedI24};
    const SlangNVVMValueOperationDesc unsupportedWidthReverse = {
        SLANG_NVVM_VALUE_OP_REVERSE_BITS,
        signedI24,
        unsupportedWidthOperands,
        SLANG_COUNT_OF(unsupportedWidthOperands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedWidthReverse));

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("integer-bit-operations"), module.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle parameterTypes[SLANG_COUNT_OF(bitWidths)] = {};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    for (Index i = 0; i < SLANG_COUNT_OF(bitWidths); ++i)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getIntegerType(module.module, bitWidths[i], parameterTypes[i])));
    }
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("useIntegerBits"),
        function)));
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));

    for (Index i = 0; i < SLANG_COUNT_OF(bitWidths); ++i)
    {
        SlangNVVMValueHandle parameter = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, i, parameter)));
        const SlangNVVMValueTypeDesc operandTypes[] = {integerTypes[i]};
        const SlangNVVMValueOperation operations[] = {
            SLANG_NVVM_VALUE_OP_COUNT_BITS,
            SLANG_NVVM_VALUE_OP_REVERSE_BITS,
            SLANG_NVVM_VALUE_OP_FIRST_BIT_HIGH,
            SLANG_NVVM_VALUE_OP_FIRST_BIT_LOW,
        };
        for (SlangNVVMValueOperation operation : operations)
        {
            const SlangNVVMValueOperationDesc desc = {
                operation,
                operation == SLANG_NVVM_VALUE_OP_REVERSE_BITS ? integerTypes[i]
                                                              : NVVMSemantics::kUnsignedI32,
                operandTypes,
                SLANG_COUNT_OF(operandTypes),
            };
            SlangNVVMValueHandle result = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.emitValueOperation(module.module, desc, &parameter, 1, result)));
            SLANG_CHECK_ABORT(result != nullptr);
        }
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (SlangNVVMSerializationFormat format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assembly)));
        const String text = _getBlobText(assembly);
        for (uint32_t bitWidth : bitWidths)
        {
            StringBuilder suffix;
            suffix << ".i" << bitWidth;
            SLANG_CHECK(text.indexOf((String("@llvm.ctpop") + suffix).getBuffer()) >= 0);
            SLANG_CHECK(text.indexOf((String("@llvm.bitreverse") + suffix).getBuffer()) >= 0);
            SLANG_CHECK(text.indexOf((String("@llvm.ctlz") + suffix).getBuffer()) >= 0);
            SLANG_CHECK(text.indexOf((String("@llvm.cttz") + suffix).getBuffer()) >= 0);
        }
        if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
        {
            SLANG_CHECK(text.indexOf("i1 immarg") >= 0);
        }
        else
        {
            SLANG_CHECK(text.indexOf("immarg") < 0);
        }
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("icmp slt")) == 4);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsExactLibdeviceUnaryOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    const SlangNVVMValueTypeDesc float32Operands[] = {NVVMSemantics::kFloat32};
    const SlangNVVMValueTypeDesc float64Operands[] = {NVVMSemantics::kFloat64};
    const SlangNVVMValueOperationDesc operations[] = {
        {
            SLANG_NVVM_VALUE_OP_SIN,
            NVVMSemantics::kFloat32,
            float32Operands,
            1,
        },
        {
            SLANG_NVVM_VALUE_OP_COS,
            NVVMSemantics::kFloat32,
            float32Operands,
            1,
        },
        {
            SLANG_NVVM_VALUE_OP_SIN,
            NVVMSemantics::kFloat64,
            float64Operands,
            1,
        },
        {
            SLANG_NVVM_VALUE_OP_COS,
            NVVMSemantics::kFloat64,
            float64Operands,
            1,
        },
        {
            SLANG_NVVM_VALUE_OP_TRUNC,
            NVVMSemantics::kFloat32,
            float32Operands,
            1,
        },
    };
    for (const auto& operation : operations)
        SLANG_CHECK(builder.supportsValueOperation(operation));

    SlangNVVMValueTypeDesc vectorFloat32 = NVVMSemantics::kFloat32;
    vectorFloat32.laneCount = 2;
    const SlangNVVMValueTypeDesc vectorFloat32Operands[] = {vectorFloat32};
    SlangNVVMValueOperationDesc unsupported = operations[0];
    unsupported.resultType = vectorFloat32;
    unsupported.operandTypes = vectorFloat32Operands;
    SLANG_CHECK(!builder.supportsValueOperation(unsupported));
    unsupported = operations[0];
    unsupported.operandTypes = float64Operands;
    SLANG_CHECK(!builder.supportsValueOperation(unsupported));

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("exact-libdevice-transcendentals"), module.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle float32Type = nullptr;
    SlangNVVMTypeHandle float64Type = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, 32, float32Type)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, 64, float64Type)));
    const SlangNVVMTypeHandle parameterTypes[] = {float32Type, float64Type};
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("useTranscendentals"),
        function)));

    SlangNVVMValueHandle float32Value = nullptr;
    SlangNVVMValueHandle float64Value = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, float32Value)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, float64Value)));
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));

    SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitValueOperation(module.module, operations[0], &float64Value, 1, rejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejected == nullptr);
    rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitValueOperation(module.module, unsupported, &float32Value, 1, rejected) ==
        SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(rejected == nullptr);

    const SlangNVVMValueHandle operands[] =
        {float32Value, float32Value, float64Value, float64Value, float32Value};
    for (Index i = 0; i < SLANG_COUNT_OF(operations); ++i)
    {
        SlangNVVMValueHandle result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitValueOperation(module.module, operations[i], &operands[i], 1, result)));
        SLANG_CHECK_ABORT(result != nullptr);
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (const auto format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assembly)));
        const String text = _getBlobText(assembly);
        const UnownedStringSlice textSlice = text.getUnownedSlice();
        SLANG_CHECK(_countOccurrences(textSlice, toSlice("declare float @__nv_sinf(float)")) == 1);
        SLANG_CHECK(_countOccurrences(textSlice, toSlice("declare float @__nv_cosf(float)")) == 1);
        SLANG_CHECK(_countOccurrences(textSlice, toSlice("declare double @__nv_sin(double)")) == 1);
        SLANG_CHECK(_countOccurrences(textSlice, toSlice("declare double @__nv_cos(double)")) == 1);
        SLANG_CHECK(
            _countOccurrences(textSlice, toSlice("declare float @__nv_truncf(float)")) == 1);
        SLANG_CHECK(_countOccurrences(textSlice, toSlice("call float @__nv_sinf")) == 1);
        SLANG_CHECK(_countOccurrences(textSlice, toSlice("call float @__nv_cosf")) == 1);
        SLANG_CHECK(_countOccurrences(textSlice, toSlice("call double @__nv_sin")) == 1);
        SLANG_CHECK(_countOccurrences(textSlice, toSlice("call double @__nv_cos")) == 1);
        SLANG_CHECK(_countOccurrences(textSlice, toSlice("call float @__nv_truncf")) == 1);
    }
}

NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerEqualOperations, Equal)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerNotEqualOperations, NotEqual)
NVVM_SCALAR_INVALID_TEST(
    nvvmIRBuilderRejectsInvalidIntegerSignedGreaterThanOperations,
    SignedGreaterThan)
NVVM_SCALAR_INVALID_TEST(
    nvvmIRBuilderRejectsInvalidIntegerSignedLessEqualOperations,
    SignedLessEqual)
NVVM_SCALAR_INVALID_TEST(
    nvvmIRBuilderRejectsInvalidIntegerSignedGreaterEqualOperations,
    SignedGreaterEqual)

#undef NVVM_SCALAR_INVALID_TEST
SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarReferenceKernels)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarReferenceModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("target triple = \"nvptx64-nvidia-cuda\"") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @writeScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @copyScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf(", i32 addrspace(1)*") >= 0);
    SLANG_CHECK(
        assembly.indexOf(
            "store i32 %slangParameter1, i32 addrspace(1)* %slangParameter0, align 4") >= 0);
    SLANG_CHECK(assembly.indexOf("load i32, i32 addrspace(1)* %slangParameter1, align 4") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("load i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 3);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 2);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(assembly.indexOf("!nvvm.annotations") >= 0);
    SLANG_CHECK(assembly.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(assembly.indexOf("@writeScalar, !\"kernel\", i32 1") >= 0);
    SLANG_CHECK(assembly.indexOf("@copyScalar, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarConditionalKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarConditionalModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("define void @chooseScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("icmp slt i32") >= 0);
    SLANG_CHECK(assembly.indexOf("add i32") >= 0);
    SLANG_CHECK(assembly.indexOf("sub i32") >= 0);
    SLANG_CHECK(assembly.indexOf("br i1") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@chooseScalar, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarSSALoopKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarSSALoopModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("define void @sumToLimit(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("phi i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("icmp slt i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("add i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("br i1")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(assembly.indexOf("i32 0") >= 0);
    SLANG_CHECK(assembly.indexOf("i32 1") >= 0);
    SLANG_CHECK(assembly.indexOf("%entry") >= 0);
    SLANG_CHECK(assembly.indexOf("%loop.body") >= 0);
    SLANG_CHECK(assembly.indexOf("%loop.continue") >= 0);
    SLANG_CHECK(assembly.indexOf("@sumToLimit, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarFunctionKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarFunctionModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("define i32 @incrementScalar(i32") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @callScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("call i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(assembly.indexOf("@callScalar, !\"kernel\", i32 1") >= 0);
    SLANG_CHECK(assembly.indexOf("@incrementScalar, !\"kernel\"") < 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsPointerOffsetKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildPointerOffsetModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        assembly.indexOf(
            "define void @copyIndexed(i32 addrspace(1)* %slangParameter0, i32 addrspace(1)* "
            "%slangParameter1, i32 %slangParameter2)") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr i32")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("load i32, i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("store i32") >= 0 && assembly.indexOf("i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@copyIndexed, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsByteOffsetPointerKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> nvvmAssemblyBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String nvvmAssemblyDiagnostics = "stale NVVM assembly diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildByteOffsetPointerModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        nvvmAssemblyBlob,
        nvvmAssemblyDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(nvvmAssemblyBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(nvvmAssemblyDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    const String nvvmAssembly = _getBlobText(nvvmAssemblyBlob);
    SLANG_CHECK(assembly.indexOf("define void @copyByteOffset(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("to i8 addrspace(1)*")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr i8")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("to <4 x i32> addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("load <4 x i32>") >= 0);
    SLANG_CHECK(assembly.indexOf("!invariant.load") >= 0);
    SLANG_CHECK(assembly.indexOf("store i32") >= 0);
    SLANG_CHECK(assembly.indexOf("align 16") >= 0);
    SLANG_CHECK(assembly.indexOf("align 4") >= 0);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(nvvmAssembly.indexOf("getelementptr i8") >= 0);
    SLANG_CHECK(nvvmAssembly.indexOf("i32 addrspace(1)*") >= 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsArrayElementKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildArrayElementModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        assembly.indexOf(
            "define void @copyArrayElement([4 x i32] addrspace(1)* %slangParameter0, [4 x i32] "
            "addrspace(1)* %slangParameter1, i32 %slangParameter2)") >= 0);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr [4 x i32]")) == 2);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("i32 0, i32 %slangParameter2")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("load i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@copyArrayElement, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

static void _runNVVMScalarBuilderKernel(
    UnitTestContext* unitTestContext,
    NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    const bool isCompare = testCase.key.family == FakeNVVMBuilderScalarFamily::Compare;

    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(_supportsNVVMScalarBuilderOperation(builder, operation));

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildNVVMScalarTestModule(
        builder,
        operation,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    StringBuilder instruction32;
    instruction32 << testCase.llvmOpcode << " i32";
    StringBuilder instruction64;
    instruction64 << testCase.llvmOpcode << " i64";
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), instruction32.getUnownedSlice()) == 1);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), instruction64.getUnownedSlice()) == 0);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == (isCompare ? 2 : 1));
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("br i1")) == (isCompare ? 1 : 0));
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    StringBuilder kernelMetadata;
    kernelMetadata << "@" << testCase.kernelName << ", !\"kernel\", i32 1";
    SLANG_CHECK(assembly.indexOf(kernelMetadata.getUnownedSlice()) >= 0);

    const Index operationIndex = assembly.indexOf(instruction32.getUnownedSlice());
    SLANG_CHECK_ABORT(operationIndex >= 0);
    if (!isCompare)
    {
        const Index storeIndex = assembly.indexOf("store i32");
        SLANG_CHECK(storeIndex > operationIndex);
        SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 1);
    }
    if (operation == NVVMScalarTestOperation::BitNot)
    {
        SLANG_CHECK(assembly.indexOf("-1") >= 0);
    }
    if (operation == NVVMScalarTestOperation::Negate)
    {
        SLANG_CHECK(assembly.indexOf("sub i32 0, %slangParameter1") >= 0);
        SLANG_CHECK(assembly.indexOf("sub nsw") < 0);
        SLANG_CHECK(assembly.indexOf("sub nuw") < 0);
    }

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

#define NVVM_SCALAR_BUILDER_KERNEL_TEST(NAME, OPERATION)                                  \
    SLANG_UNIT_TEST(NAME)                                                                 \
    {                                                                                     \
        _runNVVMScalarBuilderKernel(unitTestContext, NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerMultiplyKernel, Multiply)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerBitAndKernel, BitAnd)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerBitOrKernel, BitOr)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerBitXorKernel, BitXor)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerBitNotKernel, BitNot)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerNegateKernel, Negate)
SLANG_UNIT_TEST(nvvmIRBuilderBuildsRelaxedGlobalI32AtomicAddKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildRelaxedGlobalI32AtomicAddModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    const UnownedStringSlice assemblySlice = assembly.getUnownedSlice();
    SLANG_CHECK(
        assembly.indexOf(
            "define void @relaxedGlobalI32AtomicAdd(i32 addrspace(1)* %slangParameter0, i32 "
            "addrspace(1)* %slangParameter1, i32 %slangParameter2)") >= 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw add i32 addrspace(1)*")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw add i64")) == 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("monotonic")) == 1);
    SLANG_CHECK(assembly.indexOf("syncscope(") < 0);
    SLANG_CHECK(assembly.indexOf("acquire") < 0);
    SLANG_CHECK(assembly.indexOf("release") < 0);
    SLANG_CHECK(assembly.indexOf("seq_cst") < 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("store i32")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("align 4")) == 1);
    SLANG_CHECK(assembly.indexOf("monotonic, align") < 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@relaxedGlobalI32AtomicAdd, !\"kernel\", i32 1") >= 0);
    const Index atomicIndex = assembly.indexOf("atomicrmw add i32 addrspace(1)*");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(atomicIndex >= 0);
    SLANG_CHECK(storeIndex > atomicIndex);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerEqualKernel, Equal)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerNotEqualKernel, NotEqual)
NVVM_SCALAR_BUILDER_KERNEL_TEST(
    nvvmIRBuilderBuildsIntegerSignedGreaterThanKernel,
    SignedGreaterThan)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerSignedLessEqualKernel, SignedLessEqual)
NVVM_SCALAR_BUILDER_KERNEL_TEST(
    nvvmIRBuilderBuildsIntegerSignedGreaterEqualKernel,
    SignedGreaterEqual)

#undef NVVM_SCALAR_BUILDER_KERNEL_TEST
SLANG_UNIT_TEST(nvvmIRBuilderDifferentialScalarPTX)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

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
        String(),
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        nvvmArtifact);
    if (nvvmResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar PTX differential test because libNVVM was not found.");
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
            "Ignoring scalar PTX differential test because NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcArtifact != nullptr);

    String nvvmPTX;
    String nvrtcPTX;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_loadPTXText(nvvmArtifact, nvvmPTX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_loadPTXText(nvrtcArtifact, nvrtcPTX)));
    SLANG_CHECK(nvvmPTX.indexOf(".address_size 64") >= 0);
    SLANG_CHECK(nvrtcPTX.indexOf(".address_size 64") >= 0);

    PTXEntrySummary nvvmWrite;
    PTXEntrySummary nvvmCopy;
    PTXEntrySummary nvrtcWrite;
    PTXEntrySummary nvrtcCopy;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvvmPTX.getUnownedSlice(), toSlice(kWriteScalarKernelName), nvvmWrite)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvvmPTX.getUnownedSlice(), toSlice(kCopyScalarKernelName), nvvmCopy)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        nvrtcPTX.getUnownedSlice(),
        toSlice(kWriteScalarKernelName),
        nvrtcWrite)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvrtcPTX.getUnownedSlice(), toSlice(kCopyScalarKernelName), nvrtcCopy)));

    static const uint32_t kWriteParameterWidths[] = {64, 32};
    static const uint32_t kCopyParameterWidths[] = {64, 64};
    SLANG_CHECK(_hasPTXParameterWidths(
        nvvmWrite,
        kWriteParameterWidths,
        SLANG_COUNT_OF(kWriteParameterWidths)));
    SLANG_CHECK(_hasPTXParameterWidths(
        nvrtcWrite,
        kWriteParameterWidths,
        SLANG_COUNT_OF(kWriteParameterWidths)));
    SLANG_CHECK(_hasPTXParameterWidths(
        nvvmCopy,
        kCopyParameterWidths,
        SLANG_COUNT_OF(kCopyParameterWidths)));
    SLANG_CHECK(_hasPTXParameterWidths(
        nvrtcCopy,
        kCopyParameterWidths,
        SLANG_COUNT_OF(kCopyParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmWrite, nvrtcWrite));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmCopy, nvrtcCopy));

    SLANG_CHECK(nvvmWrite.hasGlobalStore32);
    SLANG_CHECK(nvrtcWrite.hasGlobalStore32);
    SLANG_CHECK(nvvmCopy.hasGlobalLoad32);
    SLANG_CHECK(nvvmCopy.hasGlobalStore32);
    SLANG_CHECK(nvrtcCopy.hasGlobalLoad32);
    SLANG_CHECK(nvrtcCopy.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmIRBuilderCompilesScalarBitcodeThroughRegistry)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

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

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    auto session = static_cast<Slang::Session*>(globalSession.get());
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring registered NVVM handoff test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }
    IDownstreamCompiler* compiler = session->m_downstreamCompilers[int(PassThroughMode::NVVM)];
    SLANG_CHECK_ABORT(compiler != nullptr);
    SLANG_CHECK_ABORT(compiler->getDesc().type == SLANG_PASS_THROUGH_NVVM);

    ComPtr<IArtifact> sourceArtifact =
        _createNVVMBitcodeArtifact(bitcodeBlob->getBufferPointer(), bitcodeBlob->getBufferSize());
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    const SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
        _reportArtifactDiagnostics(outputArtifact);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(diagnostics->getResult()));
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kWriteScalarKernelName)));
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kCopyScalarKernelName)));
}

SLANG_UNIT_TEST(nvvmIRBuilderCompilesEmptyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    static const char kKernelName[] = "slangSlice3aCompiledEmpty";
    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _buildEmptyNVVMKernel(builder, toSlice(kKernelName), assemblyBlob, bitcodeBlob)));

    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMBitcode(
        String(),
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        outputArtifact);
    if (compileResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring generated-bitcode compile test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kKernelName)));
}

SLANG_UNIT_TEST(nvvmIRBuilderCoexistsWithLLVM21)
{
    StringBuilder childOrderBuilder;
    if (SLANG_SUCCEEDED(PlatformUtil::getEnvironmentVariable(
            toSlice(kNVVMCoexistenceChildEnv),
            childOrderBuilder)) &&
        childOrderBuilder.getLength())
    {
        const String childOrder = childOrderBuilder.produceString();
        NVVMLLVMLoadOrder order = NVVMLLVMLoadOrder::LLVMFirst;
        if (childOrder == "llvm-first")
            order = NVVMLLVMLoadOrder::LLVMFirst;
        else if (childOrder == "nvvm-first")
            order = NVVMLLVMLoadOrder::NVVMFirst;
        else
        {
            getTestReporter()->message(
                TestMessageType::TestFailure,
                "Unknown NVVM/LLVM coexistence child order.");
            SLANG_CHECK_ABORT(false);
        }

        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_exerciseNVVMLLVMCoexistence(unitTestContext, order)));
        return;
    }

    // Establish availability in the parent so dependency absence ignores this test instead of
    // becoming an apparently successful ignored child. Loading here cannot affect either worker:
    // each child invocation below executes the probe in its own fully isolated test-server.
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    const SlangResult llvmResult = _queryLLVM21(unitTestContext);
    if (llvmResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring LLVM coexistence test because slang-llvm was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(llvmResult));

    struct OrderCase
    {
        const char* name;
        const char* environmentValue;
    };
    static const OrderCase kOrders[] = {
        {"LLVM 21 then LLVM 14 NVVM", "llvm-first"},
        {"LLVM 14 NVVM then LLVM 21", "nvvm-first"},
    };

    for (const auto& order : kOrders)
    {
        CommandLine commandLine;
        commandLine.setExecutableLocation(
            ExecutableLocation(unitTestContext->executableDirectory, "slang-test"));
        commandLine.addArg("-use-fully-isolated-test-server");
        commandLine.addArg("-server-count");
        commandLine.addArg("1");
        commandLine.addArg("-disable-retries");
        commandLine.addArg("-skip-api-detection");
        commandLine.addArg(kNVVMCoexistenceTestName);

        ExecuteResult childResult;
        childResult.init();
        SlangResult executeResult = SLANG_FAIL;
        {
            SlangUnitTest::ScopedEnvVar childOrder(
                kNVVMCoexistenceChildEnv,
                order.environmentValue);
            executeResult = ProcessUtil::execute(commandLine, childResult);
        }
        const bool reportedOnePassingTest =
            childResult.standardOutput.indexOf("100% of tests passed (1/1)") >= 0;
        if (SLANG_FAILED(executeResult) || childResult.resultCode != 0 || !reportedOnePassingTest)
        {
            _reportCoexistenceChildFailure(order.name, executeResult, childResult);
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeResult));
        SLANG_CHECK(childResult.resultCode == 0);
        SLANG_CHECK(reportedOnePassingTest);
    }
}
