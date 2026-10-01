// unit-test-nvvm-builder.cpp

#include "unit-test-nvvm-library-signature-fixtures.h"
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
        return _emitNVVMTestIntegerBinary(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_MULTIPLY,
            left,
            right,
            outValue);
    case NVVMScalarTestOperation::BitAnd:
        return _emitNVVMTestIntegerBinary(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_BIT_AND,
            left,
            right,
            outValue);
    case NVVMScalarTestOperation::BitOr:
        return _emitNVVMTestIntegerBinary(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_BIT_OR,
            left,
            right,
            outValue);
    case NVVMScalarTestOperation::BitXor:
        return _emitNVVMTestIntegerBinary(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_BIT_XOR,
            left,
            right,
            outValue);
    case NVVMScalarTestOperation::BitNot:
        return _emitNVVMTestIntegerUnary(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_BIT_NOT,
            left,
            outValue);
    case NVVMScalarTestOperation::Negate:
        return _emitNVVMTestIntegerUnary(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_NEGATE,
            left,
            outValue);
    case NVVMScalarTestOperation::Equal:
        return _emitNVVMTestIntegerCompare(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_EQUAL,
            left,
            right,
            outValue);
    case NVVMScalarTestOperation::NotEqual:
        return _emitNVVMTestIntegerCompare(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_NOT_EQUAL,
            left,
            right,
            outValue);
    case NVVMScalarTestOperation::SignedGreaterThan:
        return _emitNVVMTestIntegerCompare(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_GREATER_THAN,
            left,
            right,
            outValue);
    case NVVMScalarTestOperation::SignedLessEqual:
        return _emitNVVMTestIntegerCompare(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_LESS_EQUAL,
            left,
            right,
            outValue);
    case NVVMScalarTestOperation::SignedGreaterEqual:
        return _emitNVVMTestIntegerCompare(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_GREATER_EQUAL,
            left,
            right,
            outValue);
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
        SLANG_CHECK(builder.getConstructionAPI()->emitBitCast != nullptr);
        SLANG_CHECK(builder.getConstructionAPI()->emitPointerAddressSpaceCast != nullptr);
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
    const SlangNVVMTextureOperation sampleOperations[] = {
        SLANG_NVVM_TEXTURE_OP_SAMPLE,
        SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL,
    };
    for (const auto sampleOperation : sampleOperations)
    {
        for (const auto shape : shapes)
        {
            const SlangNVVMTextureOperationDesc operation = {
                sampleOperation,
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
    unsupported = {
        SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL,
        SLANG_NVVM_TEXTURE_SHAPE_2D,
        0,
        floatType,
        1,
    };
    SLANG_CHECK(!builder.supportsTextureOperation(unsupported));

    const SlangNVVMValueTypeKind queryKinds[] = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    };
    const uint32_t queryLaneCounts[] = {1, 2, 4};
    for (const auto kind : queryKinds)
        for (const auto laneCount : queryLaneCounts)
            for (const auto shape : shapes)
            {
                SlangNVVMTextureOperationDesc query = {
                    SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH,
                    shape,
                    0,
                    {kind, 32, laneCount},
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
                    builder.supportsTextureOperation(query) ==
                    (shape != SLANG_NVVM_TEXTURE_SHAPE_1D));
                query.isArray = 1;
                SLANG_CHECK(
                    builder.supportsTextureOperation(query) ==
                    (shape != SLANG_NVVM_TEXTURE_SHAPE_3D));
                query.isArray = 0;
                query.operation = SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH;
                SLANG_CHECK(
                    builder.supportsTextureOperation(query) ==
                    (shape == SLANG_NVVM_TEXTURE_SHAPE_3D));
                query.isArray = 1;
                SLANG_CHECK(
                    builder.supportsTextureOperation(query) ==
                    (shape == SLANG_NVVM_TEXTURE_SHAPE_2D));
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

    const SlangNVVMValueTypeKind gatherKinds[] = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    };
    for (const auto kind : gatherKinds)
    {
        for (uint32_t component = 0; component < 4; ++component)
        {
            const SlangNVVMTextureOperationDesc gather = {
                SLANG_NVVM_TEXTURE_OP_GATHER,
                SLANG_NVVM_TEXTURE_SHAPE_2D,
                0,
                {kind, 32, 4},
                component,
            };
            SLANG_CHECK(builder.supportsTextureOperation(gather));
        }
    }
    SlangNVVMTextureOperationDesc unsupportedGather = {
        SLANG_NVVM_TEXTURE_OP_GATHER,
        SLANG_NVVM_TEXTURE_SHAPE_2D,
        0,
        {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, 4},
        4,
    };
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedGather));
    unsupportedGather.component = 0;
    unsupportedGather.isArray = 1;
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedGather));
    unsupportedGather.isArray = 0;
    unsupportedGather.shape = SLANG_NVVM_TEXTURE_SHAPE_CUBE;
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedGather));
    unsupportedGather.shape = SLANG_NVVM_TEXTURE_SHAPE_2D;
    unsupportedGather.elementType.laneCount = 3;
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedGather));
    unsupportedGather.operation = SLANG_NVVM_TEXTURE_OP_SAMPLE + 1;
    SLANG_CHECK(!builder.supportsTextureOperation(unsupportedGather));
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
    SlangNVVMTextureOperationDesc implicitOperation = operation;
    implicitOperation.operation = SLANG_NVVM_TEXTURE_OP_SAMPLE;
    SlangNVVMValueHandle implicitResult = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .emitTextureOperation(module.module, implicitOperation, operands, 2, implicitResult)));
    SLANG_CHECK_ABORT(implicitResult != nullptr);
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
        SLANG_CHECK(assembly.indexOf("@llvm.nvvm.tex.unified.2d.v4f32.f32") >= 0);
        SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("insertelement")) == 8);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderEmitsTexture2DGathers)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("texture2d-gathers"), module.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle int64Type = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle float2Type = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, int64Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, 32, floatType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getVectorType(module.module, floatType, 2, float2Type)));

    const SlangNVVMTypeHandle parameterTypes[] = {int64Type, float2Type};
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
        toSlice("gather2D"),
        function)));
    SlangNVVMValueHandle operands[2] = {};
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

    const SlangNVVMValueTypeKind kinds[] = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    };
    for (const auto kind : kinds)
    {
        for (uint32_t component = 0; component < 4; ++component)
        {
            const SlangNVVMTextureOperationDesc operation = {
                SLANG_NVVM_TEXTURE_OP_GATHER,
                SLANG_NVVM_TEXTURE_SHAPE_2D,
                0,
                {kind, 32, 4},
                component,
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

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    const char* componentNames[] = {"r", "g", "b", "a"};
    const char* dataTypeNames[] = {"f32", "s32", "u32"};
    for (const auto format : formats)
    {
        ComPtr<ISlangBlob> assemblyBlob;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assemblyBlob)));
        const String assembly = _getBlobText(assemblyBlob);
        for (const char* componentName : componentNames)
        {
            for (const char* dataTypeName : dataTypeNames)
            {
                StringBuilder instruction;
                instruction << "tld4." << componentName << ".2d.v4." << dataTypeName << ".f32";
                SLANG_CHECK(
                    _countOccurrences(assembly.getUnownedSlice(), instruction.getUnownedSlice()) ==
                    1);
            }
        }
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

    String control[2];
    for (bool injectFailure : {false, true})
    {
        ScopedNVVMBuilderModule module;
        module.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("switch-texture-queries"), module.module)));

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
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.setInsertBlock(module.module, caseBlocks[i])));
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
            if (operations[i] == SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT ||
                operations[i] == SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH)
            {
                auto arrayCount = operation;
                arrayCount.shape = operations[i] == SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT
                                       ? SLANG_NVVM_TEXTURE_SHAPE_1D
                                       : SLANG_NVVM_TEXTURE_SHAPE_2D;
                arrayCount.isArray = 1;
                SlangNVVMValueHandle layerCount = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder
                        .emitTextureOperation(module.module, arrayCount, &texture, 1, layerCount)));
                SLANG_CHECK(layerCount != nullptr);
                if (injectFailure)
                {
                    // Reject the non-array role while the insertion block is live. Final complete
                    // module bytes must match the control, not just the successful call count.
                    arrayCount.isArray = 0;
                    SlangNVVMValueHandle rejected = layerCount;
                    SLANG_CHECK(SLANG_FAILED(builder.emitTextureOperation(
                        module.module,
                        arrayCount,
                        &texture,
                        1,
                        rejected)));
                    SLANG_CHECK(rejected == nullptr);
                }
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, defaultBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitUnreachable(module.module)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(module.module, function)));

        const SlangNVVMSerializationFormat formats[] = {
            SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
            SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        };
        Index formatIndex = 0;
        for (const auto format : formats)
        {
            ComPtr<ISlangBlob> assemblyBlob;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assemblyBlob)));
            const String assembly = _getBlobText(assemblyBlob);
            const UnownedStringSlice assemblySlice = assembly.getUnownedSlice();
            SLANG_CHECK(assembly.indexOf("switch i32 ") >= 0);
            SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("unreachable")) == 1);
            SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("call i32 @llvm.nvvm.txq.")) == 5);
            SLANG_CHECK(
                _countOccurrences(assemblySlice, toSlice("call i32 @llvm.nvvm.txq.height(")) == 2);
            SLANG_CHECK(assembly.indexOf("@llvm.nvvm.txq.width(i64") >= 0);
            SLANG_CHECK(assembly.indexOf("@llvm.nvvm.txq.height(i64") >= 0);
            SLANG_CHECK(assembly.indexOf("@llvm.nvvm.txq.depth(i64") >= 0);
            SLANG_CHECK(assembly.indexOf("nounwind readnone") >= 0);
            if (injectFailure)
            {
                SLANG_CHECK(assembly == control[formatIndex]);
            }
            else
            {
                control[formatIndex] = assembly;
            }
            ++formatIndex;
        }
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
    const SlangNVVMSurfaceOperationDesc physicalStore2D = {
        SLANG_NVVM_SURFACE_OP_STORE,
        SLANG_NVVM_TEXTURE_SHAPE_2D,
        0,
        {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 16, 4},
        SLANG_NVVM_SURFACE_BOUNDARY_ZERO,
    };
    SLANG_CHECK(builder.supportsSurfaceOperation(physicalStore2D));
    unsupported = physicalStore2D;
    unsupported.elementType.bitWidth = 8;
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
                    };
                    SLANG_CHECK(builder.supportsSurfaceOperation(native32));
                    native32.isArray = 1;
                    SLANG_CHECK(
                        builder.supportsSurfaceOperation(native32) ==
                        (shape != SLANG_NVVM_TEXTURE_SHAPE_3D));
                }
            }
        }
    }

    unsupported = load2D;
    unsupported.elementType = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, 3};
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = physicalStore2D;
    unsupported.shape = SLANG_NVVM_TEXTURE_SHAPE_3D;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = physicalStore2D;
    unsupported.isArray = 1;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = load2D;
    unsupported.shape = SLANG_NVVM_TEXTURE_SHAPE_CUBE;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported = load2D;
    unsupported.isArray = 1;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
    unsupported.shape = SLANG_NVVM_TEXTURE_SHAPE_1D;
    SLANG_CHECK(!builder.supportsSurfaceOperation(unsupported));
}

SLANG_UNIT_TEST(nvvmIRBuilderEmitsArraySurfaceCoordinatesWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    String control[2];
    for (bool injectFailure : {false, true})
    {
        ScopedNVVMBuilderModule module;
        module.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("array-surface-coordinates"), module.module)));
        SlangNVVMTypeHandle voidType = nullptr, i32 = nullptr, i64 = nullptr,
                            functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, i32)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, i64)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(module.module, voidType, nullptr, 0, functionType)));
        SlangNVVMValueHandle function = nullptr, surface = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            module.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("arrayCoordinates"),
            function)));
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(module.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, i64, 101, surface)));
        for (uint32_t dimensions : {1u, 2u})
        {
            SlangNVVMTypeHandle coordinateType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getVectorType(module.module, i32, dimensions + 1, coordinateType)));
            SlangNVVMValueHandle coordinates[3] = {};
            const uint32_t values[] = {16, dimensions == 1 ? 3u : 7u, 3};
            for (uint32_t i = 0; i <= dimensions; ++i)
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getIntegerConstant(module.module, i32, values[i], coordinates[i])));
            SlangNVVMValueHandle coordinate = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitVectorConstruct(
                module.module,
                coordinateType,
                coordinates,
                dimensions + 1,
                coordinate)));
            for (uint32_t lanes : {1u, 2u, 4u})
            {
                SlangNVVMSurfaceOperationDesc operation = {
                    SLANG_NVVM_SURFACE_OP_LOAD,
                    SlangNVVMTextureShape(dimensions),
                    1,
                    {SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER, 32, lanes},
                    SLANG_NVVM_SURFACE_BOUNDARY_ZERO};
                SlangNVVMValueHandle operands[] = {surface, coordinate, nullptr};
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder
                        .emitSurfaceOperation(module.module, operation, operands, 2, operands[2])));
                SLANG_CHECK_ABORT(operands[2] != nullptr);
                if (injectFailure)
                {
                    // The block is live, so these failures exercise actual descriptor/value checks.
                    auto invalid = operation;
                    invalid.elementType.laneCount = 3;
                    SlangNVVMValueHandle rejected = operands[2];
                    SLANG_CHECK(SLANG_FAILED(
                        builder
                            .emitSurfaceOperation(module.module, invalid, operands, 2, rejected)));
                    SLANG_CHECK(rejected == nullptr);
                    invalid = operation;
                    invalid.elementType = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 16, lanes};
                    SLANG_CHECK(SLANG_FAILED(
                        builder
                            .emitSurfaceOperation(module.module, invalid, operands, 2, rejected)));
                    SLANG_CHECK(rejected == nullptr);
                    SlangNVVMValueHandle wrongCoordinate[] = {surface, coordinates[0]};
                    SLANG_CHECK(SLANG_FAILED(builder.emitSurfaceOperation(
                        module.module,
                        operation,
                        wrongCoordinate,
                        2,
                        rejected)));
                    SLANG_CHECK(rejected == nullptr);
                }
                operation.operation = SLANG_NVVM_SURFACE_OP_STORE;
                SlangNVVMValueHandle unused = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.emitSurfaceOperation(module.module, operation, operands, 3, unused)));
            }
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
        Index formatIndex = 0;
        for (auto format :
             {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
              SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
        {
            ComPtr<ISlangBlob> blob;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.serializeModule(module.module, format, blob)));
            const String assembly = _getBlobText(blob);
            for (uint32_t dimensions : {1u, 2u})
                for (uint32_t lanes : {1u, 2u, 4u})
                    for (bool store : {false, true})
                    {
                        StringBuilder call;
                        call << "@llvm.nvvm." << (store ? "sust.b." : "suld.") << dimensions
                             << "d.array.";
                        if (lanes != 1)
                            call << "v" << lanes;
                        call << "i32.zero(i64 101, i32 3, i32 16";
                        if (dimensions == 2)
                            call << ", i32 7";
                        call << (store ? "," : ")");
                        SLANG_CHECK(assembly.indexOf(call.getBuffer()) >= 0);
                    }
            if (injectFailure)
            {
                SLANG_CHECK(assembly == control[formatIndex]);
            }
            else
            {
                control[formatIndex] = assembly;
            }
            ++formatIndex;
        }
    }
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

    for (uint32_t incompatibleRevision :
         {SLANG_NVVM_BUILDER_ABI_REVISION - 1, SLANG_NVVM_BUILDER_ABI_REVISION + 1})
    {
        _resetDirectNVVMFakes();
        gFakeNVVMBuilder.acceptedABIRevision = incompatibleRevision;
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
    gFakeNVVMBuilder.construction.emitBitCast = nullptr;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }

    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.construction.emitPointerAddressSpaceCast = nullptr;
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

// Canonical nested offsets must survive the provider store operation even when the root
// pointer promises less alignment than the type's ABI. Flat child stores keep their own shape.
SLANG_UNIT_TEST(nvvmIRBuilderPreservesNestedStructStoreLayout)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (uint32_t alignment : {8u, 1u})
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("nested-struct-stores"), scope.module)));
        SlangNVVMTypeHandle voidType = nullptr;
        SlangNVVMTypeHandle i8 = nullptr;
        SlangNVVMTypeHandle i16 = nullptr;
        SlangNVVMTypeHandle i32 = nullptr;
        SlangNVVMTypeHandle i64 = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 8, i8)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 16, i16)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, i32)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 64, i64)));
        SlangNVVMTypeHandle inner = nullptr;
        SlangNVVMTypeHandle middle = nullptr;
        SlangNVVMTypeHandle outer = nullptr;
        const SlangNVVMTypeHandle innerFields[] = {i8, i32};
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getStructType(scope.module, innerFields, 2, inner)));
        const SlangNVVMTypeHandle middleFields[] = {i8, inner, i16};
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getStructType(scope.module, middleFields, 3, middle)));
        const SlangNVVMTypeHandle outerFields[] = {i8, middle, i64};
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getStructType(scope.module, outerFields, 3, outer)));
        SlangNVVMTypeHandle pointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            outer,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            pointerType)));
        SlangNVVMTypeHandle functionType = nullptr;
        const SlangNVVMTypeHandle parameters[] = {outer, pointerType};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, voidType, parameters, 2, functionType)));
        SlangNVVMValueHandle function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_INTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("storeNested"),
            function)));
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        SlangNVVMValueHandle value = nullptr;
        SlangNVVMValueHandle pointer = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, value)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, pointer)));
        SLANG_CHECK(builder.emitStore(scope.module, value, pointer, 3) == SLANG_E_INVALID_ARG);
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitStore(scope.module, value, pointer, alignment)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
            assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("  store ")) == 5);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("extractvalue")) == 6);
        SLANG_CHECK(
            _countOccurrences(text.getUnownedSlice(), toSlice("getelementptr inbounds")) == 6);
        SLANG_CHECK(text.indexOf("store { i8, i32 }") >= 0);
        SLANG_CHECK(text.indexOf("store { i8, { i8, i32 }, i16 }") < 0);
        SLANG_CHECK(text.indexOf("store { i8, { i8, { i8, i32 }, i16 }, i64 }") < 0);
        if (alignment == 1)
        {
            SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice(", align 1\n")) == 5);
        }
        else
        {
            // Outer offsets zero and 24 retain eight. Middle starts at four; its fields at
            // relative offsets zero, four and twelve retain only the inherited four-byte proof.
            SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice(", align 8\n")) == 2);
            SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice(", align 4\n")) == 3);
        }
    }
}

// Arrays are explicit store boundaries, even when their elements are structs. This keeps the
// correction bounded and prevents a large array store from turning into thousands of instructions.
SLANG_UNIT_TEST(nvvmIRBuilderNestedStructStoresKeepArrayBoundaries)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("nested-struct-array-boundaries"), scope.module)));
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle i8 = nullptr;
    SlangNVVMTypeHandle i32 = nullptr;
    SlangNVVMTypeHandle vectorType = nullptr;
    SlangNVVMTypeHandle flatType = nullptr;
    SlangNVVMTypeHandle arrayType = nullptr;
    SlangNVVMTypeHandle mixedType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 8, i8)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, i32)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVectorType(scope.module, i32, 2, vectorType)));
    const SlangNVVMTypeHandle fields[] = {i8, i32};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getStructType(scope.module, fields, 2, flatType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getArrayType(scope.module, flatType, 65536, arrayType)));
    const SlangNVVMTypeHandle mixedFields[] = {i8, flatType, arrayType};
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getStructType(scope.module, mixedFields, 3, mixedType)));
    const SlangNVVMTypeHandle types[] = {i32, vectorType, flatType, arrayType, mixedType};
    const char* names[] = {"scalar", "vector", "flat", "array", "mixed"};
    for (Index i = 0; i < SLANG_COUNT_OF(types); ++i)
    {
        SlangNVVMTypeHandle pointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            types[i],
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            pointerType)));
        const SlangNVVMTypeHandle parameters[] = {types[i], pointerType};
        SlangNVVMTypeHandle functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, voidType, parameters, 2, functionType)));
        SlangNVVMValueHandle function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_INTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            UnownedStringSlice(names[i]),
            function)));
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        SlangNVVMValueHandle value = nullptr;
        SlangNVVMValueHandle pointer = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, value)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, pointer)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(scope.module, value, pointer, 1)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    }
    ComPtr<ISlangBlob> assembly;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        assembly)));
    const String text = _getBlobText(assembly);
    SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("  store ")) == 7);
    SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("store i32 ")) == 1);
    SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("store <2 x i32> ")) == 1);
    SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("store { i8, i32 } ")) == 2);
    SLANG_CHECK(
        _countOccurrences(text.getUnownedSlice(), toSlice("store [65536 x { i8, i32 }] ")) == 2);
    SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("extractvalue")) == 3);
    SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("getelementptr inbounds")) == 3);
    SLANG_CHECK(text.indexOf("getelementptr inbounds [65536") < 0);
}

// Whole arrays retain one store regardless of their size or dimensions. Only a nested struct
// boundary inside that stored value weakens alignment; pointer pointees and flat arrays do not.
SLANG_UNIT_TEST(nvvmIRBuilderPreservesNestedArrayStoreLayout)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (uint32_t alignment : {1u, 4u, 8u})
    {
        for (Index typeIndex = 0; typeIndex < 13; ++typeIndex)
        {
            ScopedNVVMBuilderModule scope;
            scope.builder = &builder;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createModule(toSlice("nested-array-store"), scope.module)));
            SlangNVVMTypeHandle voidType = nullptr;
            SlangNVVMTypeHandle i16 = nullptr;
            SlangNVVMTypeHandle i32 = nullptr;
            SlangNVVMTypeHandle vectorType = nullptr;
            SlangNVVMTypeHandle child = nullptr;
            SlangNVVMTypeHandle cell = nullptr;
            SlangNVVMTypeHandle smallArray = nullptr;
            SlangNVVMTypeHandle largeArray = nullptr;
            SlangNVVMTypeHandle matrix = nullptr;
            SlangNVVMTypeHandle wrapper = nullptr;
            SlangNVVMTypeHandle wrapperArray = nullptr;
            SlangNVVMTypeHandle nestedPointer = nullptr;
            SlangNVVMTypeHandle pointerArray = nullptr;
            SlangNVVMTypeHandle flatArray = nullptr;
            SlangNVVMTypeHandle flatWrapper = nullptr;
            SlangNVVMTypeHandle scalarArray = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 16, i16)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, i32)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getVectorType(scope.module, i32, 2, vectorType)));
            const SlangNVVMTypeHandle childFields[] = {i16, i32};
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getStructType(scope.module, childFields, 2, child)));
            const SlangNVVMTypeHandle cellFields[] = {i16, child};
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getStructType(scope.module, cellFields, 2, cell)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getArrayType(scope.module, cell, 3, smallArray)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getArrayType(scope.module, cell, 65536, largeArray)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getArrayType(scope.module, smallArray, 2, matrix)));
            const SlangNVVMTypeHandle wrapperFields[] = {i32, smallArray, i32};
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getStructType(scope.module, wrapperFields, 3, wrapper)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getArrayType(scope.module, wrapper, 2, wrapperArray)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
                scope.module,
                cell,
                SLANG_NVVM_ADDRESS_SPACE_GENERIC,
                nestedPointer)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getArrayType(scope.module, nestedPointer, 3, pointerArray)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getArrayType(scope.module, child, 3, flatArray)));
            const SlangNVVMTypeHandle flatWrapperFields[] = {i32, flatArray, i32};
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getStructType(scope.module, flatWrapperFields, 3, flatWrapper)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getArrayType(scope.module, i32, 65536, scalarArray)));
            const struct
            {
                SlangNVVMTypeHandle type;
                bool containsNestedStruct;
            } cases[] = {
                {smallArray, true},
                {largeArray, true},
                {matrix, true},
                {wrapper, true},
                {wrapperArray, true},
                {nestedPointer, false},
                {pointerArray, false},
                {flatArray, false},
                {flatWrapper, false},
                {scalarArray, false},
                {i32, false},
                {vectorType, false},
                {child, false},
            };
            SLANG_RELEASE_ASSERT(SLANG_COUNT_OF(cases) == 13);
            const SlangNVVMTypeHandle valueType = cases[typeIndex].type;
            SlangNVVMTypeHandle pointerType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
                scope.module,
                valueType,
                SLANG_NVVM_ADDRESS_SPACE_GENERIC,
                pointerType)));
            const SlangNVVMTypeHandle parameters[] = {valueType, pointerType};
            SlangNVVMTypeHandle functionType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionType(scope.module, voidType, parameters, 2, functionType)));
            SlangNVVMValueHandle function = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                scope.module,
                functionType,
                SLANG_NVVM_LINKAGE_INTERNAL,
                SLANG_NVVM_FUNCTION_FLAG_NONE,
                toSlice("storeValue"),
                function)));
            SlangNVVMBlockHandle block = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.createBlock(scope.module, function, toSlice("entry"), block)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
            SlangNVVMValueHandle value = nullptr;
            SlangNVVMValueHandle pointer = nullptr;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, value)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, pointer)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.emitStore(scope.module, value, pointer, alignment)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
            ComPtr<ISlangBlob> assembly;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
                assembly)));
            const String text = _getBlobText(assembly);
            SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("  store ")) == 1);
            SLANG_CHECK(text.indexOf("extractvalue") < 0);
            SLANG_CHECK(text.indexOf("getelementptr") < 0);
            SLANG_CHECK(text.indexOf("call ") < 0);
            const bool containsNestedStruct = cases[typeIndex].containsNestedStruct;
            const char* expectedAlignment = containsNestedStruct || alignment == 1 ? ", align 1\n"
                                            : alignment == 4                       ? ", align 4\n"
                                                                                   : ", align 8\n";
            SLANG_CHECK(text.indexOf(expectedAlignment) >= 0);
            if (valueType == largeArray)
                SLANG_CHECK(text.indexOf("store [65536 x { i16, { i16, i32 } }] ") >= 0);
        }
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
    SlangNVVMValueHandle output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
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

    const char* executionRegisters[] = {"tid", "ctaid", "ntid", "nctaid"};
    const char* firstName = "llvm.nvvm.read.ptx.sreg.tid.x";
    SlangNVVMNamedIntrinsicDesc firstIntrinsic =
        {firstName, strlen(firstName), NVVMSemantics::kUnsignedI32, nullptr, 0};
    const SlangNVVMNamedIntrinsicDesc barrierOperation =
        {"llvm.nvvm.barrier0", strlen("llvm.nvvm.barrier0"), NVVMSemantics::kVoid, nullptr, 0};
    const SlangNVVMNamedIntrinsicDesc deviceBarrierOperation =
        {"llvm.nvvm.membar.gl", strlen("llvm.nvvm.membar.gl"), NVVMSemantics::kVoid, nullptr, 0};
    const SlangNVVMNamedIntrinsicDesc workgroupMemoryBarrierOperation =
        {"llvm.nvvm.membar.cta", strlen("llvm.nvvm.membar.cta"), NVVMSemantics::kVoid, nullptr, 0};

    SlangNVVMValueHandle rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitNamedIntrinsic(scope.module, firstIntrinsic, nullptr, 0, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entryBlock)));
    SLANG_CHECK(
        builder.getValueOperationsAPI()
            ->emitNamedIntrinsic(scope.module, &barrierOperation, nullptr, 0, nullptr) ==
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

    for (const char* executionRegister : executionRegisters)
    {
        SlangNVVMValueHandle components[3] = {};
        for (uint32_t axis = 0; axis < 3; ++axis)
        {
            StringBuilder name;
            name << "llvm.nvvm.read.ptx.sreg." << executionRegister << "." << char('x' + axis);
            SlangNVVMNamedIntrinsicDesc intrinsic = {
                name.getBuffer(),
                size_t(name.getLength()),
                NVVMSemantics::kUnsignedI32,
                nullptr,
                0};
            SLANG_CHECK_ABORT(builder.supportsNamedIntrinsic(intrinsic));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.emitNamedIntrinsic(scope.module, intrinsic, nullptr, 0, components[axis])));
        }
        SlangNVVMValueHandle vector = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitVectorConstruct(scope.module, uint3Type, components, 3, vector)));
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
        builder.emitNamedIntrinsic(scope.module, barrierOperation, nullptr, 0, barrierValue)));
    SLANG_CHECK(barrierValue == nullptr);
    barrierValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .emitNamedIntrinsic(scope.module, deviceBarrierOperation, nullptr, 0, barrierValue)));
    SLANG_CHECK(barrierValue == nullptr);
    barrierValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitNamedIntrinsic(
        scope.module,
        workgroupMemoryBarrierOperation,
        nullptr,
        0,
        barrierValue)));
    SLANG_CHECK(barrierValue == nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(scope.module, function)));

    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitNamedIntrinsic(scope.module, firstIntrinsic, nullptr, 0, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    barrierValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitNamedIntrinsic(scope.module, barrierOperation, nullptr, 0, barrierValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(barrierValue == nullptr);
    barrierValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitNamedIntrinsic(
            scope.module,
            workgroupMemoryBarrierOperation,
            nullptr,
            0,
            barrierValue) == SLANG_E_INVALID_ARG);
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
        SLANG_CHECK(
            _countOccurrences(
                text.getUnownedSlice(),
                toSlice("call void @llvm.nvvm.membar.cta()")) == 1);
        SLANG_CHECK(text.contains("= { convergent nounwind }"));
        SLANG_CHECK(text.contains("= { nounwind }"));
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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestFloatingBinary(
        builder,
        scope.module,
        SLANG_NVVM_VALUE_OP_ADD,
        left,
        right,
        laterValue)));
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
            _emitNVVMTestFloatingBinary(
                builder,
                scope.module,
                SLANG_NVVM_VALUE_OP_ADD,
                operands[0],
                operands[1],
                output) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(output == nullptr);

        output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            _emitNVVMTestFloatingCompare(
                builder,
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
            _emitNVVMTestFloatingUnary(
                builder,
                scope.module,
                SLANG_NVVM_VALUE_OP_NEGATE,
                operand,
                output) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(output == nullptr);
    }

    SlangNVVMValueHandle sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestFloatingBinary(
        builder,
        scope.module,
        SLANG_NVVM_VALUE_OP_ADD,
        left,
        right,
        sum)));
    SlangNVVMValueHandle negated = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestFloatingUnary(
        builder,
        scope.module,
        SLANG_NVVM_VALUE_OP_NEGATE,
        left,
        negated)));
    SlangNVVMValueHandle equal = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestFloatingCompare(
        builder,
        scope.module,
        SLANG_NVVM_VALUE_OP_EQUAL,
        left,
        right,
        equal)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    SlangNVVMValueHandle output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        _emitNVVMTestFloatingBinary(
            builder,
            scope.module,
            SLANG_NVVM_VALUE_OP_ADD,
            left,
            right,
            output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        _emitNVVMTestFloatingCompare(
            builder,
            scope.module,
            SLANG_NVVM_VALUE_OP_EQUAL,
            left,
            right,
            output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        _emitNVVMTestFloatingUnary(
            builder,
            scope.module,
            SLANG_NVVM_VALUE_OP_NEGATE,
            left,
            output) == SLANG_E_INVALID_ARG);
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

// Rejected typed shuffles must leave the module identical to an empty control module, even
// when the semantic descriptor is valid but the actual LLVM values have the wrong types.


// Hardware mask snapshots must remain distinct and control-dependent in both LLVM dialects.
SLANG_UNIT_TEST(nvvmIRBuilderBuildsHardwareActiveMask)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    const SlangNVVMValueOperationDesc operation = {
        SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK,
        NVVMSemantics::kUnsignedI32,
        nullptr,
        0,
    };
    SLANG_CHECK_ABORT(builder.supportsValueOperation(operation));
    SlangNVVMValueOperationDesc unsupported = operation;
    unsupported.resultType = NVVMSemantics::kSignedI32;
    SLANG_CHECK(!builder.supportsValueOperation(unsupported));
    unsupported = operation;
    unsupported.operandTypes = &NVVMSemantics::kUnsignedI32;
    unsupported.operandCount = 1;
    SLANG_CHECK(!builder.supportsValueOperation(unsupported));

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("hardware-active-mask"), scope.module)));
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(scope.module, integerType, nullptr, 0, functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("readHardwareMask"),
        function)));
    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
    SlangNVVMValueHandle first = nullptr;
    SlangNVVMValueHandle second = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitValueOperation(scope.module, operation, nullptr, 0, first)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitValueOperation(scope.module, operation, nullptr, 0, second)));
    SLANG_CHECK(first != second);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, second)));

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
            _countOccurrences(
                text.getUnownedSlice(),
                toSlice("asm sideeffect \"activemask.b32 $0;\", \"=r\"()")) == 2);
        SLANG_CHECK(text.indexOf("convergent") >= 0);
        SLANG_CHECK(text.indexOf("vote.ballot") < 0);
    }
}

// BF16's physical i16 must not accidentally select IEEE Half or integer conversion semantics.
SLANG_UNIT_TEST(nvvmIRBuilderBFloat16Contract)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(toSlice("bf16"), scope.module)));
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle boolType = nullptr;
    SlangNVVMTypeHandle bitsType = nullptr;
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 32, floatType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 1, boolType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 16, bitsType)));
    const SlangNVVMTypeHandle parameterTypes[] = {floatType, boolType, bitsType};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(scope.module, floatType, parameterTypes, 3, functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("convert"),
        function)));
    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
    SlangNVVMValueHandle value = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, value)));
    SlangNVVMValueOperationDesc narrow =
        {SLANG_NVVM_VALUE_OP_FLOAT_CONVERT, NVVMSemantics::kBFloat16, &NVVMSemantics::kFloat32, 1};
    SlangNVVMValueOperationDesc widen =
        {SLANG_NVVM_VALUE_OP_FLOAT_CONVERT, NVVMSemantics::kFloat32, &NVVMSemantics::kBFloat16, 1};
    SLANG_CHECK(builder.supportsValueOperation(narrow));
    SLANG_CHECK(builder.supportsValueOperation(widen));
    SlangNVVMValueHandle narrowed = nullptr;
    SlangNVVMValueHandle widened = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitValueOperation(scope.module, narrow, &value, 1, narrowed)));
    // core.meta.slang's generic select(bool,T,T) deliberately retains a typed scalar
    // select. Use dynamic parameters so LLVM cannot fold away this transport contract.
    SlangNVVMValueHandle condition = nullptr;
    SlangNVVMValueHandle alternate = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, condition)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, alternate)));
    SlangNVVMValueTypeDesc selectionTypes[] = {
        NVVMSemantics::kBool,
        NVVMSemantics::kBFloat16,
        NVVMSemantics::kBFloat16};
    SlangNVVMValueOperationDesc selection =
        {SLANG_NVVM_VALUE_OP_SELECT, NVVMSemantics::kBFloat16, selectionTypes, 3};
    SlangNVVMValueHandle selectionValues[] = {condition, narrowed, alternate};
    SlangNVVMValueHandle selected = nullptr;
    SLANG_CHECK_ABORT(builder.supportsValueOperation(selection));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitValueOperation(scope.module, selection, selectionValues, 3, selected)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitValueOperation(scope.module, widen, &selected, 1, widened)));
    selectionTypes[1] = NVVMSemantics::kFloat16;
    SLANG_CHECK(!builder.supportsValueOperation(selection));
    SlangNVVMValueHandle invalidSelection = nullptr;
    SLANG_CHECK(SLANG_FAILED(
        builder.emitValueOperation(scope.module, selection, selectionValues, 3, invalidSelection)));
    SLANG_CHECK(invalidSelection == nullptr);
    selectionTypes[1] = NVVMSemantics::kBFloat16;
    selection.operandCount = 2;
    SLANG_CHECK(!builder.supportsValueOperation(selection));
    SLANG_CHECK(SLANG_FAILED(
        builder.emitValueOperation(scope.module, selection, selectionValues, 2, invalidSelection)));
    SLANG_CHECK(invalidSelection == nullptr);

    const SlangNVVMValueTypeDesc excludedOperands[] = {
        NVVMSemantics::kSignedI32,
        NVVMSemantics::kUnsignedI32,
        NVVMSemantics::kFloat16,
        NVVMSemantics::kFloat64,
        NVVMSemantics::kBFloat16,
        {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, 2},
    };
    for (const auto& operand : excludedOperands)
    {
        auto rejected = narrow;
        rejected.operandTypes = &operand;
        if (operand.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
            operand.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER)
            rejected.operation = SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT;
        SLANG_CHECK(!builder.supportsValueOperation(rejected));
        SlangNVVMValueHandle invalid = nullptr;
        SLANG_CHECK(
            SLANG_FAILED(builder.emitValueOperation(scope.module, rejected, &value, 1, invalid)));
        SLANG_CHECK(invalid == nullptr);
    }
    const SlangNVVMValueTypeDesc malformed[] = {
        {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 32, 1},
        {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 2},
    };
    for (const auto& result : malformed)
    {
        auto rejected = narrow;
        rejected.resultType = result;
        SLANG_CHECK(!builder.supportsValueOperation(rejected));
        SlangNVVMValueHandle invalid = nullptr;
        SLANG_CHECK(
            SLANG_FAILED(builder.emitValueOperation(scope.module, rejected, &value, 1, invalid)));
        SLANG_CHECK(invalid == nullptr);
    }
    SlangNVVMValueOperationDesc toInteger = {
        SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER,
        NVVMSemantics::kSignedI32,
        &NVVMSemantics::kBFloat16,
        1};
    SLANG_CHECK(!builder.supportsValueOperation(toInteger));
    SlangNVVMValueHandle invalidInteger = nullptr;
    SLANG_CHECK(SLANG_FAILED(
        builder.emitValueOperation(scope.module, toInteger, &narrowed, 1, invalidInteger)));
    SLANG_CHECK(invalidInteger == nullptr);
    SlangNVVMValueTypeDesc binaryOperands[] = {NVVMSemantics::kBFloat16, NVVMSemantics::kBFloat16};
    SlangNVVMValueOperationDesc arithmetic =
        {SLANG_NVVM_VALUE_OP_ADD, NVVMSemantics::kBFloat16, binaryOperands, 2};
    SLANG_CHECK(!builder.supportsValueOperation(arithmetic));
    auto wrongArity = narrow;
    wrongArity.operandCount = 0;
    SLANG_CHECK(!builder.supportsValueOperation(wrongArity));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, widened)));
    for (auto format :
         {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
          SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        String text = _getBlobText(assembly);
        SLANG_CHECK(text.indexOf("cvt.rn.bf16.f32") >= 0);
        SLANG_CHECK(text.indexOf("select i1") >= 0);
        SLANG_CHECK(text.indexOf(", i16 %") >= 0);
        SLANG_CHECK(text.indexOf("zext i16") >= 0);
        SLANG_CHECK(text.indexOf("shl i32") >= 0);
        SLANG_CHECK(text.indexOf("fptrunc") < 0);
        SLANG_CHECK(text.indexOf("fpext") < 0);
        SLANG_CHECK(text.indexOf("bfloat") < 0);
    }
}

// BF16 remains physically i16; the primitive owns one rounding boundary and core owns dot order.
SLANG_UNIT_TEST(nvvmIRBuilderBFloat16FmaContract)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    String control;
    for (bool injectFailures : {false, true})
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(toSlice("bf16-fma"), scope.module)));
        SlangNVVMTypeHandle scalar = nullptr, half = nullptr, functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 16, scalar)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 16, half)));
        const SlangNVVMTypeHandle parameters[] = {scalar, scalar, scalar, half};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, scalar, parameters, 4, functionType)));
        SlangNVVMValueHandle function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("fma"),
            function)));
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        SlangNVVMValueHandle inputs[4] = {};
        for (uint32_t i = 0; i < 4; ++i)
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionParameter(scope.module, function, i, inputs[i])));
        const SlangNVVMValueTypeDesc bf = NVVMSemantics::kBFloat16;
        const SlangNVVMValueTypeDesc operands[] = {bf, bf, bf};
        const SlangNVVMValueOperationDesc fma = {SLANG_NVVM_VALUE_OP_FMA, bf, operands, 3};
        SLANG_CHECK_ABORT(builder.supportsValueOperation(fma));
        SlangNVVMValueHandle result = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitValueOperation(scope.module, fma, inputs, 3, result)));
        if (injectFailures)
        {
            SlangNVVMValueHandle invalid = nullptr;
            const SlangNVVMValueHandle wrongPhysical[] = {inputs[0], inputs[1], inputs[3]};
            SLANG_CHECK(SLANG_FAILED(
                builder.emitValueOperation(scope.module, fma, wrongPhysical, 3, invalid)));
            SLANG_CHECK(invalid == nullptr);
            const SlangNVVMValueTypeDesc excluded[] = {
                NVVMSemantics::kFloat16,
                NVVMSemantics::kFloat32,
                NVVMSemantics::kUnsignedI16,
                {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 32, 1},
                {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 0},
                {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 2},
            };
            for (auto type : excluded)
            {
                for (uint32_t slot = 0; slot < 4; ++slot)
                {
                    auto rejected = fma;
                    SlangNVVMValueTypeDesc wrongTypes[] = {bf, bf, bf};
                    if (slot == 3)
                        rejected.resultType = type;
                    else
                        wrongTypes[slot] = type;
                    rejected.operandTypes = wrongTypes;
                    SLANG_CHECK(!builder.supportsValueOperation(rejected));
                    SLANG_CHECK(SLANG_FAILED(
                        builder.emitValueOperation(scope.module, rejected, inputs, 3, invalid)));
                    SLANG_CHECK(invalid == nullptr);
                }
            }
            for (uint32_t arity : {0u, 1u, 2u})
            {
                auto rejected = fma;
                rejected.operandCount = arity;
                SLANG_CHECK(!builder.supportsValueOperation(rejected));
                SLANG_CHECK(SLANG_FAILED(
                    builder.emitValueOperation(scope.module, rejected, inputs, arity, invalid)));
                SLANG_CHECK(invalid == nullptr);
            }
            const SlangNVVMValueTypeDesc vectors[] = {
                {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 2},
                {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 2}};
            const SlangNVVMValueOperationDesc retiredDot = {82, bf, vectors, 2};
            SLANG_CHECK(!builder.supportsValueOperation(retiredDot));
            SLANG_CHECK(SLANG_FAILED(
                builder.emitValueOperation(scope.module, retiredDot, inputs, 2, invalid)));
            SLANG_CHECK(invalid == nullptr);
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, result)));
        for (auto format :
             {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
              SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
        {
            ComPtr<ISlangBlob> assembly;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
            String text = _getBlobText(assembly);
            if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
            {
                if (injectFailures)
                {
                    SLANG_CHECK(text == control);
                }
                else
                    control = text;
            }
            SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("fma.rn.bf16")) == 1);
            SLANG_CHECK(text.contains("=h,h,h,h"));
            SLANG_CHECK(!text.contains("extractelement"));
            SLANG_CHECK(!text.contains("fadd"));
            SLANG_CHECK(!text.contains("fmul"));
            SLANG_CHECK(!text.contains("bfloat"));
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBFloat16VectorContract)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (uint32_t lanes = 2; lanes <= 4; ++lanes)
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("bf16-vector"), scope.module)));
        SlangNVVMTypeHandle scalar = nullptr;
        SlangNVVMTypeHandle vector = nullptr;
        SlangNVVMTypeHandle functionType = nullptr;
        SlangNVVMValueHandle function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 32, scalar)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getVectorType(scope.module, scalar, lanes, vector)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, vector, &vector, 1, functionType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("convert"),
            function)));
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        SlangNVVMValueHandle input = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, input)));
        const SlangNVVMValueTypeDesc bf = {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, lanes};
        const SlangNVVMValueTypeDesc fp = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, lanes};
        const SlangNVVMValueOperationDesc narrow = {SLANG_NVVM_VALUE_OP_FLOAT_CONVERT, bf, &fp, 1};
        const SlangNVVMValueOperationDesc widen = {SLANG_NVVM_VALUE_OP_FLOAT_CONVERT, fp, &bf, 1};
        SLANG_CHECK_ABORT(builder.supportsValueOperation(narrow));
        SLANG_CHECK_ABORT(builder.supportsValueOperation(widen));
        SlangNVVMValueHandle narrowed = nullptr;
        SlangNVVMValueHandle widened = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitValueOperation(scope.module, narrow, &input, 1, narrowed)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitValueOperation(scope.module, widen, &narrowed, 1, widened)));
        // Both the descriptor and actual physical operand must match; i16 is not IEEE Half.
        SlangNVVMValueHandle invalid = nullptr;
        SLANG_CHECK(
            SLANG_FAILED(builder.emitValueOperation(scope.module, narrow, &narrowed, 1, invalid)));
        SLANG_CHECK(invalid == nullptr);
        const SlangNVVMValueTypeDesc excluded[] = {
            {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 32, lanes},
            {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 0},
            {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 5},
            {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, lanes == 2 ? 3u : 2u},
        };
        for (auto type : excluded)
        {
            auto rejected = narrow;
            rejected.resultType = type;
            SLANG_CHECK(!builder.supportsValueOperation(rejected));
            SLANG_CHECK(SLANG_FAILED(
                builder.emitValueOperation(scope.module, rejected, &input, 1, invalid)));
            SLANG_CHECK(invalid == nullptr);
            rejected = widen;
            rejected.operandTypes = &type;
            SLANG_CHECK(!builder.supportsValueOperation(rejected));
        }
        for (uint32_t bits : {16u, 64u})
        {
            SlangNVVMValueTypeDesc type = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, bits, lanes};
            auto rejected = narrow;
            rejected.operandTypes = &type;
            SLANG_CHECK(!builder.supportsValueOperation(rejected));
            rejected = widen;
            rejected.resultType = type;
            SLANG_CHECK(!builder.supportsValueOperation(rejected));
        }
        const SlangNVVMValueTypeDesc operands[] = {bf, bf};
        for (auto operation : {SLANG_NVVM_VALUE_OP_ADD, SLANG_NVVM_VALUE_OP_MULTIPLY})
        {
            SlangNVVMValueOperationDesc rejected = {operation, bf, operands, 2};
            SLANG_CHECK(!builder.supportsValueOperation(rejected));
        }
        SlangNVVMValueTypeDesc condition = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, lanes};
        SlangNVVMValueOperationDesc compare = {SLANG_NVVM_VALUE_OP_EQUAL, condition, operands, 2};
        SLANG_CHECK(!builder.supportsValueOperation(compare));
        const SlangNVVMValueTypeDesc selectOperands[] = {NVVMSemantics::kBool, bf, bf};
        SlangNVVMValueOperationDesc select = {SLANG_NVVM_VALUE_OP_SELECT, bf, selectOperands, 3};
        SLANG_CHECK(!builder.supportsValueOperation(select));
        const SlangNVVMValueTypeDesc integer = {SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER, 32, lanes};
        SlangNVVMValueOperationDesc fromInteger =
            {SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT, bf, &integer, 1};
        SlangNVVMValueOperationDesc toInteger =
            {SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER, integer, &bf, 1};
        SLANG_CHECK(!builder.supportsValueOperation(fromInteger));
        SLANG_CHECK(!builder.supportsValueOperation(toInteger));
        auto wrongArity = narrow;
        wrongArity.operandCount = 0;
        SLANG_CHECK(!builder.supportsValueOperation(wrongArity));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, widened)));
        for (auto format :
             {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
              SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
        {
            ComPtr<ISlangBlob> assembly;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
            String text = _getBlobText(assembly);
            SLANG_CHECK(text.indexOf("cvt.rn.bf16.f32") >= 0);
            SLANG_CHECK(text.indexOf("extractelement") >= 0);
            SLANG_CHECK(text.indexOf("insertelement") >= 0);
            SLANG_CHECK(text.indexOf("zext i16") >= 0);
            SLANG_CHECK(text.indexOf("fptrunc") < 0);
            SLANG_CHECK(text.indexOf("fpext") < 0);
            SLANG_CHECK(text.indexOf("bfloat") < 0);
            SLANG_CHECK(text.indexOf("poison") < 0);
        }
    }
}

// Counter observations must remain distinct and side-effecting in both LLVM dialects.
SLANG_UNIT_TEST(nvvmIRBuilderBuildsClock)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    const SlangNVVMNamedIntrinsicDesc operation = {
        "llvm.nvvm.read.ptx.sreg.clock",
        sizeof("llvm.nvvm.read.ptx.sreg.clock") - 1,
        NVVMSemantics::kUnsignedI32,
        nullptr,
        0,
    };
    SLANG_CHECK_ABORT(builder.supportsNamedIntrinsic(operation));
    SlangNVVMNamedIntrinsicDesc unsupported = operation;
    unsupported.resultType = NVVMSemantics::kSignedI64;
    SLANG_CHECK(!builder.supportsNamedIntrinsic(unsupported));
    unsupported = operation;
    const SlangNVVMNamedIntrinsicOperandDesc operand = {
        NVVMSemantics::kUnsignedI32,
        SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
    unsupported.operands = &operand;
    unsupported.operandCount = 1;
    SLANG_CHECK(!builder.supportsNamedIntrinsic(unsupported));

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(toSlice("live-clock"), scope.module)));
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(scope.module, integerType, nullptr, 0, functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("readClock"),
        function)));
    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
    SlangNVVMValueHandle invalid = nullptr;
    SLANG_CHECK(
        SLANG_FAILED(builder.emitNamedIntrinsic(scope.module, unsupported, nullptr, 0, invalid)));
    SLANG_CHECK(invalid == nullptr);
    unsupported = operation;
    unsupported.resultType = NVVMSemantics::kSignedI64;
    SLANG_CHECK(
        SLANG_FAILED(builder.emitNamedIntrinsic(scope.module, unsupported, nullptr, 0, invalid)));
    SLANG_CHECK(invalid == nullptr);
    const SlangNVVMValueOperationDesc retired = {80, operation.resultType, nullptr, 0};
    SLANG_CHECK(!builder.supportsValueOperation(retired));
    SLANG_CHECK(
        SLANG_FAILED(builder.emitValueOperation(scope.module, retired, nullptr, 0, invalid)));
    SLANG_CHECK(invalid == nullptr);
    SlangNVVMValueHandle first = nullptr;
    SlangNVVMValueHandle second = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitNamedIntrinsic(scope.module, operation, nullptr, 0, first)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitNamedIntrinsic(scope.module, operation, nullptr, 0, second)));
    SLANG_CHECK(first != second);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, second)));

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
            _countOccurrences(
                text.getUnownedSlice(),
                toSlice("asm sideeffect \"mov.u32 $0, %clock;\", \"=r\"()")) == 2);
        SLANG_CHECK(text.indexOf("convergent") < 0);
        SLANG_CHECK(text.indexOf("llvm.nvvm.read.ptx.sreg.clock") < 0);
    }
}

// Counter observations must remain distinct and side-effecting in both LLVM dialects.
SLANG_UNIT_TEST(nvvmIRBuilderBuildsClock64)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    const SlangNVVMNamedIntrinsicDesc operation = {
        "llvm.nvvm.read.ptx.sreg.clock64",
        sizeof("llvm.nvvm.read.ptx.sreg.clock64") - 1,
        NVVMSemantics::kSignedI64,
        nullptr,
        0,
    };
    SLANG_CHECK_ABORT(builder.supportsNamedIntrinsic(operation));
    SlangNVVMNamedIntrinsicDesc unsupported = operation;
    unsupported.resultType = NVVMSemantics::kUnsignedI32;
    SLANG_CHECK(!builder.supportsNamedIntrinsic(unsupported));
    unsupported = operation;
    const SlangNVVMNamedIntrinsicOperandDesc operand = {
        NVVMSemantics::kUnsignedI32,
        SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
    unsupported.operands = &operand;
    unsupported.operandCount = 1;
    SLANG_CHECK(!builder.supportsNamedIntrinsic(unsupported));

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(toSlice("live-clock"), scope.module)));
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 64, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(scope.module, integerType, nullptr, 0, functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("readClock"),
        function)));
    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
    SlangNVVMValueHandle invalid = nullptr;
    SLANG_CHECK(
        SLANG_FAILED(builder.emitNamedIntrinsic(scope.module, unsupported, nullptr, 0, invalid)));
    SLANG_CHECK(invalid == nullptr);
    unsupported = operation;
    unsupported.resultType = NVVMSemantics::kUnsignedI32;
    SLANG_CHECK(
        SLANG_FAILED(builder.emitNamedIntrinsic(scope.module, unsupported, nullptr, 0, invalid)));
    SLANG_CHECK(invalid == nullptr);
    const SlangNVVMValueOperationDesc retired = {81, operation.resultType, nullptr, 0};
    SLANG_CHECK(!builder.supportsValueOperation(retired));
    SLANG_CHECK(
        SLANG_FAILED(builder.emitValueOperation(scope.module, retired, nullptr, 0, invalid)));
    SLANG_CHECK(invalid == nullptr);
    SlangNVVMValueHandle first = nullptr;
    SlangNVVMValueHandle second = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitNamedIntrinsic(scope.module, operation, nullptr, 0, first)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitNamedIntrinsic(scope.module, operation, nullptr, 0, second)));
    SLANG_CHECK(first != second);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, second)));

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
            _countOccurrences(
                text.getUnownedSlice(),
                toSlice("asm sideeffect \"mov.u64 $0, %clock64;\", \"=l\"()")) == 2);
        SLANG_CHECK(text.indexOf("convergent") < 0);
        SLANG_CHECK(text.indexOf("llvm.nvvm.read.ptx.sreg.clock") < 0);
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

static void _checkNVVMIRBuilderBuildsWaveMaskMatchKernel(
    UnitTestContext* unitTestContext,
    const UnownedStringSlice& moduleName,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& helperName,
    WavePredicateValueKind valueKind)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    const SlangNVVMValueTypeDesc unsupportedOperandTypes[] = {
        NVVMSemantics::kUnsignedI32,
        NVVMSemantics::kUnsignedI64,
    };
    const SlangNVVMValueOperationDesc unsupportedOperation = {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH,
        NVVMSemantics::kUnsignedI32,
        unsupportedOperandTypes,
        SLANG_COUNT_OF(unsupportedOperandTypes),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedOperation));

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(moduleName, scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateWavePredicateIntrinsicKernel(
        builder,
        scope.module,
        kernelName,
        helperName,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH,
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
        helperDefinition << "define i32 @" << helperName << "(i32";
        StringBuilder helperCall;
        helperCall << "call i32 @" << helperName;
        SLANG_CHECK(text.indexOf(helperDefinition.getUnownedSlice()) >= 0);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("call i32 @llvm.nvvm.match.any.sync.i32(i32")) == 1);
        SLANG_CHECK(
            _countOccurrences(text, toSlice("bitcast float")) ==
            (valueKind == WavePredicateValueKind::Float ? 1 : 0));
        SLANG_CHECK(_countOccurrences(text, toSlice("ret i32")) == 1);
        SLANG_CHECK(_countOccurrences(text, helperCall.getUnownedSlice()) == 1);
        SLANG_CHECK(_countOccurrences(text, toSlice("store i32")) == 1);
        SLANG_CHECK(
            _countOccurrences(
                text,
                toSlice("declare i32 @llvm.nvvm.match.any.sync.i32(i32, i32)")) == 1);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveMaskMatchIntKernel)
{
    _checkNVVMIRBuilderBuildsWaveMaskMatchKernel(
        unitTestContext,
        toSlice("wave-mask-match-int-module"),
        toSlice("waveMaskMatchIntKernel"),
        toSlice("waveMaskMatchInt"),
        WavePredicateValueKind::Integer);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveMaskMatchUIntKernel)
{
    _checkNVVMIRBuilderBuildsWaveMaskMatchKernel(
        unitTestContext,
        toSlice("wave-mask-match-uint-module"),
        toSlice("waveMaskMatchUIntKernel"),
        toSlice("waveMaskMatchUInt"),
        WavePredicateValueKind::UnsignedInteger);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsWaveMaskMatchFloatKernel)
{
    _checkNVVMIRBuilderBuildsWaveMaskMatchKernel(
        unitTestContext,
        toSlice("wave-mask-match-float-module"),
        toSlice("waveMaskMatchFloatKernel"),
        toSlice("waveMaskMatchFloat"),
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
        SLANG_CHECK(_countOccurrences(text, toSlice("select i1")) >= 2);
        SLANG_CHECK(text.indexOf(toSlice("@__nv_fminf")) < 0);
        SLANG_CHECK(text.indexOf(toSlice("@__nv_fmax")) < 0);
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
        SLANG_CHECK(text.indexOf(toSlice("fptrunc float")) < 0);
        SLANG_CHECK(text.indexOf(toSlice("call i16 @llvm.nvvm.f2h.rn(float")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fptosi half")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("sitofp <2 x i32>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fadd <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fsub <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp oge <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fpext <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fptrunc <2 x float>")) < 0);
        SLANG_CHECK(_countOccurrences(text, toSlice("call i16 @llvm.nvvm.f2h.rn(float")) == 3);
        SLANG_CHECK(text.indexOf(toSlice("fptosi <2 x half>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("to <2 x double>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fadd <2 x double>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fsub <2 x double>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fmul <2 x double>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fdiv <2 x double>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("frem <2 x double>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fcmp olt <2 x double>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fptosi <2 x double>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fpext <2 x float>")) >= 0);
        SLANG_CHECK(text.indexOf(toSlice("fptrunc <2 x double>")) >= 0);
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
        SLANG_CHECK(_countOccurrences(text, toSlice("insertelement <2 x half>")) == 0);
        // Both the explicit Half2 construction and the narrowed Half2 use integer lane assembly.
        SLANG_CHECK(_countOccurrences(text, toSlice("insertelement <2 x i16>")) == 4);
        SLANG_CHECK(text.indexOf(toSlice("bitcast <2 x i16>")) >= 0);
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
    const SlangNVVMValueOperationDesc supportedFloatRemainder = {
        SLANG_NVVM_VALUE_OP_REMAINDER,
        float64x2,
        float64x2Operands,
        SLANG_COUNT_OF(float64x2Operands),
    };
    SLANG_CHECK(builder.supportsValueOperation(supportedFloatRemainder));


    const SlangNVVMValueTypeDesc float64x5 = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        64,
        5,
    };
    const SlangNVVMValueTypeDesc float64x5Operands[] = {float64x5, float64x5};
    const SlangNVVMValueOperationDesc unsupportedFloat64LaneCount = {
        SLANG_NVVM_VALUE_OP_ADD,
        float64x5,
        float64x5Operands,
        SLANG_COUNT_OF(float64x5Operands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(unsupportedFloat64LaneCount));

    const SlangNVVMValueTypeDesc float16 = NVVMSemantics::kFloat16;
    const SlangNVVMValueTypeDesc float32 = NVVMSemantics::kFloat32;
    const SlangNVVMValueTypeDesc float16BinaryOperands[] = {float16, float16};
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

    const SlangNVVMValueTypeDesc uint32x2 = {
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
        32,
        2,
    };
    const SlangNVVMValueTypeDesc uint64Operands[] = {NVVMSemantics::kUnsignedI64};
    const SlangNVVMValueOperationDesc scalarToVectorBitReinterpret = {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        uint32x2,
        uint64Operands,
        SLANG_COUNT_OF(uint64Operands),
    };
    SLANG_CHECK(builder.supportsValueOperation(scalarToVectorBitReinterpret));
    const SlangNVVMValueTypeDesc uint32x2Operands[] = {uint32x2};
    const SlangNVVMValueOperationDesc vectorToScalarBitReinterpret = {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        NVVMSemantics::kUnsignedI64,
        uint32x2Operands,
        SLANG_COUNT_OF(uint32x2Operands),
    };
    SLANG_CHECK(builder.supportsValueOperation(vectorToScalarBitReinterpret));

    SlangNVVMValueTypeDesc uint32x3 = uint32x2;
    uint32x3.laneCount = 3;
    const SlangNVVMValueTypeDesc uint32x3Operands[] = {uint32x3};
    const SlangNVVMValueOperationDesc mismatchedWidthBitReinterpret = {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        NVVMSemantics::kUnsignedI64,
        uint32x3Operands,
        SLANG_COUNT_OF(uint32x3Operands),
    };
    SLANG_CHECK(!builder.supportsValueOperation(mismatchedWidthBitReinterpret));

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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerSignedLessThan(
        builder,
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
        _emitNVVMTestIntegerBinary(
            builder,
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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerSignedLessThan(
        builder,
        firstModule.module,
        secondX,
        secondY,
        secondCondition)));
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
        _emitNVVMTestIntegerBinary(
            builder,
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            firstX,
            foreignX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        _emitNVVMTestIntegerSignedLessThan(
            builder,
            firstModule.module,
            firstY,
            foreignY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder
            .emitConditionalBranch(firstModule.module, foreignCondition, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitBranch(firstModule.module, foreignBlock) == SLANG_E_INVALID_ARG);

    rejectedValue = firstFunction;
    SLANG_CHECK(
        _emitNVVMTestIntegerBinary(
            builder,
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            firstX,
            firstDestination,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        _emitNVVMTestIntegerBinary(
            builder,
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            firstX,
            secondX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        _emitNVVMTestIntegerSignedLessThan(
            builder,
            firstModule.module,
            firstX,
            secondY,
            rejectedValue) == SLANG_E_INVALID_ARG);
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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerSignedLessThan(
        builder,
        firstModule.module,
        firstX,
        firstY,
        condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(firstModule.module, condition, trueBlock, falseBlock)));

    rejectedValue = firstFunction;
    SLANG_CHECK(
        _emitNVVMTestIntegerBinary(
            builder,
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitBranch(firstModule.module, mergeBlock) == SLANG_E_INVALID_ARG);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, trueBlock)));
    SlangNVVMValueHandle sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerBinary(
        builder,
        firstModule.module,
        SLANG_NVVM_VALUE_OP_ADD,
        firstX,
        firstY,
        sum)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(firstModule.module, sum, firstDestination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(firstModule.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, falseBlock)));
    // `sum` belongs to the sibling true block and does not dominate this insertion point. Both
    // instruction-producing and side-effecting consumers must reject it without changing the
    // false block.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        _emitNVVMTestIntegerBinary(
            builder,
            firstModule.module,
            SLANG_NVVM_VALUE_OP_ADD,
            sum,
            firstX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, sum, firstDestination, 4) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle difference = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerBinary(
        builder,
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
        _emitNVVMTestIntegerBinary(
            builder,
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
        builder.emitPhi(foreignModule.module, mergeBlock, integerType, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitPhi(module.module, mergeBlock, voidType, rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle condition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _emitNVVMTestIntegerSignedLessThan(builder, module.module, x, y, condition)));
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
        _emitNVVMTestIntegerBinary(builder, module.module, SLANG_NVVM_VALUE_OP_ADD, x, y, sum)));
    SlangNVVMValueHandle phi = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitPhi(module.module, mergeBlock, integerType, phi)));
    // Incoming validation requires the complete CFG; merge has no terminator yet.
    SLANG_CHECK(builder.addPhiIncoming(module.module, phi, x, trueBlock) == SLANG_E_INVALID_ARG);
    // The explicit target permits late phi insertion and must preserve the current insertion state.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, phi, destination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    // All blocks in the phi function are now terminated. Invalid incoming calls must not mutate it.
    SLANG_CHECK(
        builder.addPhiIncoming(module.module, phi, condition, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addPhiIncoming(module.module, phi, secondX, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addPhiIncoming(module.module, phi, foreignOne, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.addPhiIncoming(module.module, phi, x, orphanBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.addPhiIncoming(module.module, phi, sum, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.addPhiIncoming(module.module, phi, x, trueBlock)));
    SLANG_CHECK(builder.addPhiIncoming(module.module, phi, y, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.addPhiIncoming(module.module, phi, y, falseBlock)));
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
        builder.emitCall(
            module.module,
            helper,
            noInsertionArguments,
            SLANG_COUNT_OF(noInsertionArguments),
            noInsertionResult) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(noInsertionResult == nullptr);
    SLANG_CHECK(builder.emitValueReturn(module.module, x) == SLANG_E_INVALID_ARG);

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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerBinary(
        builder,
        module.module,
        SLANG_NVVM_VALUE_OP_ADD,
        helperValue,
        one,
        helperResult)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(module.module, helperResult)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, callerOther)));
    SlangNVVMValueHandle nonDominatingValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerBinary(
        builder,
        module.module,
        SLANG_NVVM_VALUE_OP_ADD,
        x,
        y,
        nonDominatingValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(module.module, nonDominatingValue)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, voidBlock)));
    SLANG_CHECK(builder.emitValueReturn(module.module, voidFunctionValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, callerEntry)));
    SlangNVVMValueHandle condition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _emitNVVMTestIntegerSignedLessThan(builder, module.module, x, y, condition)));

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
        builder.emitCall(module.module, x, xArgument, 1, rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitCall(module.module, foreignHelper, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitCall(module.module, helper, nullptr, 1, rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitCall(module.module, helper, nullptr, 0, rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitCall(
            module.module,
            helper,
            tooManyArguments,
            SLANG_COUNT_OF(tooManyArguments),
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitCall(module.module, helper, conditionArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitCall(module.module, helper, helperArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitCall(module.module, helper, foreignArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitCall(module.module, helper, nonDominatingArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    // Invalid valued returns must likewise leave caller.entry unterminated for the valid graph.
    SLANG_CHECK(builder.emitValueReturn(module.module, condition) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitValueReturn(module.module, helperValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitValueReturn(module.module, foreignValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitValueReturn(module.module, nonDominatingValue) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle callResult = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitCall(module.module, helper, xArgument, 1, callResult)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(module.module, callResult)));
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitCall(module.module, helper, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitValueReturn(module.module, x) == SLANG_E_INVALID_ARG);

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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _emitNVVMTestIntegerSignedLessThan(builder, module.module, index, index, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle producerPointer = nullptr;
    SlangNVVMValueHandle producerInteger = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitPointerOffset(module.module, source, index, producerPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerBinary(
        builder,
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

SLANG_UNIT_TEST(nvvmIRBuilderBuildsAndValidatesPointerBitTransport)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.isInitialized());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("pointer-bit-transport"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("pointer-bit-transport-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle int32Type = nullptr;
    SlangNVVMTypeHandle int64Type = nullptr;
    SlangNVVMTypeHandle uint2Type = nullptr;
    SlangNVVMTypeHandle globalPointerType = nullptr;
    SlangNVVMTypeHandle genericPointerType = nullptr;
    SlangNVVMTypeHandle differentPointeePointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, int32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, int64Type)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getVectorType(module.module, int32Type, 2, uint2Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        int32Type,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        int32Type,
        SLANG_NVVM_ADDRESS_SPACE_GENERIC,
        genericPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        int64Type,
        SLANG_NVVM_ADDRESS_SPACE_GENERIC,
        differentPointeePointerType)));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalPointerType,
        globalPointerType,
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
        toSlice("pointerBitTransport"),
        function)));
    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle source = nullptr;
    SlangNVVMValueHandle int32Value = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, source)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, int32Value)));

    SlangNVVMTypeHandle foreignVoidType = nullptr;
    SlangNVVMTypeHandle foreignInt32Type = nullptr;
    SlangNVVMTypeHandle foreignInt64Type = nullptr;
    SlangNVVMTypeHandle foreignGlobalPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignInt32Type)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 64, foreignInt64Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignInt32Type,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignGlobalPointerType)));
    SlangNVVMTypeHandle foreignFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        &foreignGlobalPointerType,
        1,
        foreignFunctionType)));
    SlangNVVMValueHandle foreignFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("foreignPointerBitTransport"),
        foreignFunction)));
    SlangNVVMValueHandle foreignPointer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignPointer)));

    auto expectRejectedBitCast = [&](SlangNVVMModuleHandle targetModule,
                                     SlangNVVMTypeHandle resultType,
                                     SlangNVVMValueHandle value)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitBitCast(targetModule, resultType, value, rejected) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    auto expectRejectedAddressSpaceCast = [&](SlangNVVMModuleHandle targetModule,
                                              SlangNVVMTypeHandle resultType,
                                              SlangNVVMValueHandle value)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitPointerAddressSpaceCast(targetModule, resultType, value, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    // A value and type do not establish an insertion point. Foreign handles must also be rejected
    // before the provider mutates either module.
    expectRejectedBitCast(module.module, uint2Type, source);
    expectRejectedAddressSpaceCast(module.module, genericPointerType, source);
    expectRejectedBitCast(module.module, foreignInt64Type, source);
    expectRejectedAddressSpaceCast(module.module, genericPointerType, foreignPointer);

    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));

    expectRejectedAddressSpaceCast(module.module, globalPointerType, source);
    expectRejectedAddressSpaceCast(module.module, differentPointeePointerType, source);
    expectRejectedAddressSpaceCast(module.module, int64Type, source);
    expectRejectedAddressSpaceCast(module.module, foreignGlobalPointerType, source);
    expectRejectedBitCast(module.module, int32Type, source);
    expectRejectedBitCast(module.module, genericPointerType, int32Value);
    expectRejectedBitCast(module.module, genericPointerType, source);
    expectRejectedBitCast(foreignModule.module, uint2Type, source);

    SlangNVVMValueHandle genericSource = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitPointerAddressSpaceCast(
        module.module,
        genericPointerType,
        source,
        genericSource)));
    SlangNVVMValueHandle pointerBits = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitBitCast(module.module, uint2Type, genericSource, pointerBits)));
    SlangNVVMValueHandle roundTripPointer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitBitCast(module.module, genericPointerType, pointerBits, roundTripPointer)));
    SlangNVVMValueHandle loaded = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitLoad(module.module, roundTripPointer, 4, SLANG_NVVM_LOAD_FLAG_NONE, loaded)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, loaded, destination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(module.module, function)));

    expectRejectedBitCast(module.module, int64Type, genericSource);
    expectRejectedAddressSpaceCast(module.module, genericPointerType, source);

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
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("addrspacecast i32 addrspace(1)*")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("ptrtoint i32*")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("bitcast i64")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("bitcast <2 x i32>")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("inttoptr i64")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("load i32, i32*")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("store i32")) == 1);
    SLANG_CHECK(assembly.indexOf("@pointerBitTransport, !\"kernel\", i32 1") >= 0);
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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _emitNVVMTestIntegerSignedLessThan(builder, module.module, index, index, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle nonDominatingIndex = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerBinary(
        builder,
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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerSignedLessThan(
        builder,
        module.module,
        left,
        right,
        scaffoldCondition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitConditionalBranch(
        module.module,
        scaffoldCondition,
        producerBlock,
        consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle nonDominating = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerBinary(
        builder,
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
    SlangNVVMAtomicOperationDesc signedWideMaxOperation = unsignedWideMaxOperation;
    signedWideMaxOperation.valueType.kind = SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER;
    SLANG_CHECK(builder.supportsAtomicOperation(signedWideMaxOperation));
    SlangNVVMAtomicOperationDesc unsignedI32MaxOperation = unsignedWideMaxOperation;
    unsignedI32MaxOperation.valueType.bitWidth = 32;
    SLANG_CHECK(builder.supportsAtomicOperation(unsignedI32MaxOperation));
    SlangNVVMAtomicOperationDesc sharedWideMaxOperation = unsignedWideMaxOperation;
    sharedWideMaxOperation.addressSpace = SLANG_NVVM_ADDRESS_SPACE_SHARED;
    SLANG_CHECK(builder.supportsAtomicOperation(sharedWideMaxOperation));
    SlangNVVMAtomicOperationDesc unsupportedWideMaxOperation = unsignedWideMaxOperation;
    unsupportedWideMaxOperation.memoryOrder = SLANG_NVVM_MEMORY_ORDER_ACQUIRE;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedWideMaxOperation));

    SlangNVVMAtomicOperationDesc unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.operation = SLANG_NVVM_ATOMIC_OP_SUBTRACT;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.operation =
        SlangNVVMAtomicOperation(SLANG_NVVM_ATOMIC_OPERATION_COUNT);
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    SlangNVVMAtomicOperationDesc selectedWideAddOperation = atomicOperation;
    selectedWideAddOperation.valueType.bitWidth = 64;
    SLANG_CHECK(builder.supportsAtomicOperation(selectedWideAddOperation));
    unsupportedAtomicOperation = atomicOperation;
    unsupportedAtomicOperation.valueType.laneCount = 2;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    SlangNVVMAtomicOperationDesc selectedFloatingAddOperation = atomicOperation;
    selectedFloatingAddOperation.valueType.kind = SLANG_NVVM_VALUE_TYPE_FLOATING_POINT;
    SLANG_CHECK(builder.supportsAtomicOperation(selectedFloatingAddOperation));
    selectedFloatingAddOperation.valueType.bitWidth = 64;
    SLANG_CHECK(builder.supportsAtomicOperation(selectedFloatingAddOperation));
    unsupportedAtomicOperation = selectedFloatingAddOperation;
    unsupportedAtomicOperation.operation = SLANG_NVVM_ATOMIC_OP_MAX;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    SlangNVVMAtomicOperationDesc selectedHalfAddOperation = selectedFloatingAddOperation;
    selectedHalfAddOperation.valueType.bitWidth = 16;
    SLANG_CHECK(builder.supportsAtomicOperation(selectedHalfAddOperation));
    SlangNVVMAtomicOperationDesc selectedHalf2AddOperation = selectedHalfAddOperation;
    selectedHalf2AddOperation.valueType.laneCount = 2;
    SLANG_CHECK(builder.supportsAtomicOperation(selectedHalf2AddOperation));
    SlangNVVMAtomicOperationDesc unsupportedSharedHalf2AddOperation = selectedHalf2AddOperation;
    unsupportedSharedHalf2AddOperation.addressSpace = SLANG_NVVM_ADDRESS_SPACE_SHARED;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedSharedHalf2AddOperation));
    SlangNVVMAtomicOperationDesc unsupportedHalf3AddOperation = selectedHalf2AddOperation;
    unsupportedHalf3AddOperation.valueType.laneCount = 3;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedHalf3AddOperation));
    unsupportedAtomicOperation = selectedHalfAddOperation;
    unsupportedAtomicOperation.valueType.bitWidth = 8;
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
    SlangNVVMAtomicOperationDesc loadOperation = atomicOperation;
    loadOperation.operation = SLANG_NVVM_ATOMIC_OP_LOAD;
    SLANG_CHECK(builder.supportsAtomicOperation(loadOperation));
    SlangNVVMAtomicOperationDesc storeOperation = atomicOperation;
    storeOperation.operation = SLANG_NVVM_ATOMIC_OP_STORE;
    SLANG_CHECK(builder.supportsAtomicOperation(storeOperation));
    SlangNVVMAtomicOperationDesc exchangeOperation = atomicOperation;
    exchangeOperation.operation = SLANG_NVVM_ATOMIC_OP_EXCHANGE;
    SLANG_CHECK(builder.supportsAtomicOperation(exchangeOperation));
    SlangNVVMAtomicOperationDesc compareExchangeOperation = atomicOperation;
    compareExchangeOperation.operation = SLANG_NVVM_ATOMIC_OP_COMPARE_EXCHANGE;
    SLANG_CHECK(builder.supportsAtomicOperation(compareExchangeOperation));
    unsupportedAtomicOperation = compareExchangeOperation;
    unsupportedAtomicOperation.failureMemoryOrder = SLANG_NVVM_MEMORY_ORDER_ACQUIRE;
    SLANG_CHECK(!builder.supportsAtomicOperation(unsupportedAtomicOperation));
    SlangNVVMAtomicOperationDesc floatingExchangeOperation = exchangeOperation;
    floatingExchangeOperation.valueType = NVVMSemantics::kFloat32;
    floatingExchangeOperation.addressSpace = SLANG_NVVM_ADDRESS_SPACE_SHARED;
    SLANG_CHECK(builder.supportsAtomicOperation(floatingExchangeOperation));

    auto expectRejected = [&](SlangNVVMModuleHandle targetModule,
                              SlangNVVMValueHandle pointer,
                              SlangNVVMValueHandle addend)
    {
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        const SlangNVVMValueHandle operands[] = {pointer, addend};
        SLANG_CHECK(
            builder.emitAtomicOperation(
                targetModule,
                atomicOperation,
                operands,
                SLANG_COUNT_OF(operands),
                rejected) == SLANG_E_INVALID_ARG);
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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _emitNVVMTestIntegerSignedLessThan(builder, module.module, value, value, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle nonDominatingValue = nullptr;
    SlangNVVMValueHandle nonDominatingPointer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMTestIntegerBinary(
        builder,
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
    const SlangNVVMValueHandle directOperands[] = {destination, value};
    SLANG_CHECK(
        api->emitOperation(
            module.module,
            &atomicOperation,
            directOperands,
            SLANG_COUNT_OF(directOperands),
            nullptr) == SLANG_E_INVALID_ARG);
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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        atomicOperation,
        directOperands,
        SLANG_COUNT_OF(directOperands),
        oldValue)));
    SlangNVVMValueHandle sharedOldValue = nullptr;
    const SlangNVVMValueHandle sharedOperands[] = {sharedDestination, value};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        sharedAtomicOperation,
        sharedOperands,
        SLANG_COUNT_OF(sharedOperands),
        sharedOldValue)));
    SlangNVVMValueHandle wideOldValue = nullptr;
    const SlangNVVMValueHandle wideOperands[] = {wideDestination, wideValue};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        unsignedWideMaxOperation,
        wideOperands,
        SLANG_COUNT_OF(wideOperands),
        wideOldValue)));
    const SlangNVVMValueHandle loadOperands[] = {destination};
    SlangNVVMValueHandle loadedValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        loadOperation,
        loadOperands,
        SLANG_COUNT_OF(loadOperands),
        loadedValue)));
    SlangNVVMValueHandle exchangedValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        exchangeOperation,
        directOperands,
        SLANG_COUNT_OF(directOperands),
        exchangedValue)));
    const SlangNVVMValueHandle compareExchangeOperands[] = {destination, value, value};
    SlangNVVMValueHandle comparedValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        compareExchangeOperation,
        compareExchangeOperands,
        SLANG_COUNT_OF(compareExchangeOperands),
        comparedValue)));
    SlangNVVMValueHandle storeResult = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        storeOperation,
        directOperands,
        SLANG_COUNT_OF(directOperands),
        storeResult)));
    SLANG_CHECK(storeResult == nullptr);
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
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("monotonic")) >= 7);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("align 4")) >= 6);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("align 8")) == 1);
    SLANG_CHECK(assembly.indexOf("monotonic, align 4") >= 0);
    SLANG_CHECK(assembly.indexOf("syncscope(") < 0);
    SLANG_CHECK(assembly.indexOf("acquire") < 0);
    SLANG_CHECK(assembly.indexOf("release") < 0);
    SLANG_CHECK(assembly.indexOf("seq_cst") < 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("store i32")) == 1);
    SLANG_CHECK(assembly.indexOf("load atomic ") < 0);
    SLANG_CHECK(assembly.indexOf("store atomic ") < 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw xchg i32 addrspace(1)*")) == 2);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("cmpxchg i32 addrspace(1)*")) == 2);
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
    const UnownedStringSlice compatibleAssemblySlice = compatibleAssembly.getUnownedSlice();
    SLANG_CHECK(
        _countOccurrences(compatibleAssemblySlice, toSlice("cmpxchg i32 addrspace(1)*")) == 2);
    SLANG_CHECK(
        _countOccurrences(compatibleAssemblySlice, toSlice("atomicrmw xchg i32 addrspace(1)*")) ==
        2);
    SLANG_CHECK(compatibleAssembly.indexOf("monotonic, align") < 0);
    SLANG_CHECK(compatibleAssembly.indexOf("atomicrmw add i32 addrspace(1)*") >= 0);
    SLANG_CHECK(compatibleAssembly.indexOf("atomicrmw umax i64 addrspace(1)*") >= 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderEmitsGlobalHalf2AtomicAdd)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("global-half2-atomic-add"), module.module)));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle halfType = nullptr;
    SlangNVVMTypeHandle half2Type = nullptr;
    SlangNVVMTypeHandle pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, 16, halfType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getVectorType(module.module, halfType, 2, half2Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        half2Type,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));

    const SlangNVVMTypeHandle parameterTypes[] = {pointerType, half2Type};
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
        toSlice("globalHalf2AtomicAdd"),
        function)));
    SlangNVVMValueHandle pointer = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, pointer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, value)));
    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(module.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));

    const SlangNVVMAtomicOperationDesc operation = {
        SLANG_NVVM_ATOMIC_OP_ADD,
        {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 16, 2},
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        SLANG_NVVM_MEMORY_ORDER_RELAXED,
    };
    const SlangNVVMValueHandle operands[] = {pointer, value};
    SlangNVVMValueHandle originalValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitAtomicOperation(
        module.module,
        operation,
        operands,
        SLANG_COUNT_OF(operands),
        originalValue)));
    SLANG_CHECK_ABORT(originalValue != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("atom.global.add.noftz.f16x2 $0, [$1], $2;") >= 0);
    SLANG_CHECK(assembly.indexOf("=r,l,r") >= 0);
}


struct ScopedNVVMTestDeviceLibrary
{
    const NVVMIRBuilder* builder;
    SlangNVVMDeviceLibraryHandle library = nullptr;
    ~ScopedNVVMTestDeviceLibrary() { builder->destroyDeviceLibrary(library); }
};

// Build independent selected-library definitions with the ordinary construction API. In particular,
// double(double) under __nv_roundf proves the reader uses the selected signature, not the name.
static ComPtr<ISlangBlob> _makeNVVMDeviceLibraryFixture(
    const NVVMIRBuilder& builder,
    uint32_t width,
    bool definition = true,
    const char* name = "__nv_roundf",
    uint32_t operandCount = 1,
    const SlangNVVMValueTypeDesc* outputPointee = nullptr,
    bool outputByValue = false)
{
    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("device-library-fixture"), module.module)));
    SlangNVVMTypeHandle type = nullptr, functionType = nullptr;
    SlangNVVMValueHandle function = nullptr, value = nullptr;
    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, width, type)));
    SlangNVVMTypeHandle parameterTypes[] = {type, type, type};
    SlangNVVMTypeHandle pointee = type;
    if (outputPointee)
    {
        SLANG_CHECK_ABORT(operandCount == 2);
        if (outputPointee->kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER)
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, pointee)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            module.module,
            pointee,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            parameterTypes[1])));
    }
    SLANG_CHECK_ABORT(operandCount >= 1 && operandCount <= SLANG_COUNT_OF(parameterTypes));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(module.module, type, parameterTypes, operandCount, functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        UnownedStringSlice(name),
        function)));
    if (outputByValue)
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setFunctionParameterAttributes(
            module.module,
            function,
            1,
            SLANG_NVVM_PARAMETER_FLAG_BY_VALUE,
            pointee,
            outputPointee->kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ? 4 : width / 8)));
    if (definition)
    {
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, value)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(module.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));
        if (outputPointee)
        {
            SlangNVVMValueHandle output = nullptr, storedValue = value;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, output)));
            if (outputPointee->kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER)
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getIntegerConstant(module.module, pointee, 7, storedValue)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(
                module.module,
                storedValue,
                output,
                outputPointee->kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ? 4 : width / 8)));
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(module.module, value)));
    }
    ComPtr<ISlangBlob> result;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(module.module, SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE, result)));
    return result;
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryPointerOutputs)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (uint32_t width : {32u, 64u})
        for (bool frexp : {true, false})
        {
            const char* name = frexp ? (width == 32 ? "__nv_frexpf" : "__nv_frexp")
                                     : (width == 32 ? "__nv_modff" : "__nv_modf");
            const auto valueType = width == 32 ? NVVMSemantics::kFloat32 : NVVMSemantics::kFloat64;
            const auto outputType = frexp ? NVVMSemantics::kSignedI32 : valueType;
            const SlangNVVMNamedIntrinsicOperandDesc operands[] = {
                {valueType, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE},
                {outputType, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_OUT_POINTER}};
            const SlangNVVMNamedIntrinsicDesc desc = {name, strlen(name), valueType, operands, 2};
            auto bytes = _makeNVVMDeviceLibraryFixture(builder, width, true, name, 2, &outputType);
            ScopedNVVMTestDeviceLibrary library{&builder};
            String diagnostics;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.loadDeviceLibrary(bytes, library.library, diagnostics)));
            SLANG_CHECK(builder.supportsDeviceLibraryFunction(library.library, desc));
            SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
            auto attributedBytes =
                _makeNVVMDeviceLibraryFixture(builder, width, true, name, 2, &outputType, true);
            ScopedNVVMTestDeviceLibrary attributed{&builder};
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.loadDeviceLibrary(attributedBytes, attributed.library, diagnostics)));
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(attributed.library, desc));

            String control;
            for (bool injectFailures : {false, true})
            {
                ScopedNVVMBuilderModule scope;
                scope.builder = &builder;
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(builder.createModule(toSlice("pointer-math"), scope.module)));
                SlangNVVMTypeHandle scalar = nullptr, pointee = nullptr, globalPointer = nullptr;
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, width, scalar)));
                if (frexp)
                {
                    SLANG_CHECK_ABORT(
                        SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, pointee)));
                }
                else
                    pointee = scalar;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
                    scope.module,
                    pointee,
                    SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
                    globalPointer)));
                const SlangNVVMTypeHandle parameters[] = {scalar, globalPointer};
                SlangNVVMTypeHandle functionType = nullptr;
                SlangNVVMValueHandle function = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionType(scope.module, scalar, parameters, 2, functionType)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                    scope.module,
                    functionType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_FUNCTION_FLAG_NONE,
                    toSlice("caller"),
                    function)));
                SlangNVVMBlockHandle block = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.createBlock(scope.module, function, toSlice("entry"), block)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
                SlangNVVMValueHandle input = nullptr, global = nullptr, output = nullptr,
                                     result = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(scope.module, function, 0, input)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(scope.module, function, 1, global)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitLocalStorage(
                    scope.module,
                    pointee,
                    frexp ? 4 : width / 8,
                    toSlice("output"),
                    output)));
                const SlangNVVMValueHandle arguments[] = {input, output};
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitDeviceLibraryFunction(
                    library.library,
                    scope.module,
                    desc,
                    arguments,
                    2,
                    result)));
                if (injectFailures)
                {
                    SlangNVVMValueHandle rejected = nullptr;
                    const SlangNVVMValueHandle wrongAddressSpace[] = {input, global};
                    SLANG_CHECK(SLANG_FAILED(builder.emitDeviceLibraryFunction(
                        library.library,
                        scope.module,
                        desc,
                        wrongAddressSpace,
                        2,
                        rejected)));
                    SLANG_CHECK(rejected == nullptr);
                    SLANG_CHECK(SLANG_FAILED(builder.emitDeviceLibraryFunction(
                        attributed.library,
                        scope.module,
                        desc,
                        arguments,
                        2,
                        rejected)));
                    SLANG_CHECK(rejected == nullptr);
                    for (bool wrongRole : {false, true})
                    {
                        SlangNVVMNamedIntrinsicOperandDesc wrongOperands[] = {
                            operands[0],
                            operands[1]};
                        if (wrongRole)
                            wrongOperands[1].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE;
                        else
                            wrongOperands[1].type = frexp ? valueType : NVVMSemantics::kSignedI32;
                        auto wrong = desc;
                        wrong.operands = wrongOperands;
                        SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, wrong));
                        SLANG_CHECK(SLANG_FAILED(builder.emitDeviceLibraryFunction(
                            library.library,
                            scope.module,
                            wrong,
                            arguments,
                            2,
                            rejected)));
                        SLANG_CHECK(rejected == nullptr);
                    }
                }
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, result)));
                for (auto format :
                     {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                      SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
                {
                    ComPtr<ISlangBlob> assembly;
                    SLANG_CHECK_ABORT(
                        SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
                    const String text = _getBlobText(assembly);
                    if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
                    {
                        if (injectFailures)
                        {
                            SLANG_CHECK(text == control);
                        }
                        else
                            control = text;
                    }
                    StringBuilder expected;
                    expected << "declare " << (width == 32 ? "float" : "double") << " @" << name
                             << "(" << (width == 32 ? "float" : "double") << ", "
                             << (frexp         ? "i32"
                                 : width == 32 ? "float"
                                               : "double")
                             << "*)";
                    SLANG_CHECK(text.contains(expected.getUnownedSlice()));
                    SLANG_CHECK(
                        _countOccurrences(text.getUnownedSlice(), UnownedStringSlice(name)) == 2);
                }
            }
        }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarMathOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    const SlangNVVMValueTypeDesc halfOperands[] = {NVVMSemantics::kFloat16};
    const SlangNVVMValueTypeDesc float64Operands[] = {
        NVVMSemantics::kFloat64,
        NVVMSemantics::kFloat64,
    };
    const SlangNVVMValueOperation binaryMathOperations[] = {
        SLANG_NVVM_VALUE_OP_FMOD,
    };

    SlangNVVMValueOperationDesc operation = {};
    for (auto mathOperation : binaryMathOperations)
    {
        operation = {mathOperation, NVVMSemantics::kFloat64, float64Operands, 2};
        SLANG_CHECK(builder.supportsValueOperation(operation));
    }
    SlangNVVMValueTypeDesc vectorFloat64 = NVVMSemantics::kFloat64;
    vectorFloat64.laneCount = 2;
    const SlangNVVMValueTypeDesc vectorFloat64Operands[] = {vectorFloat64};
    operation = {
        SlangNVVMValueOperation(66),
        vectorFloat64,
        vectorFloat64Operands,
        1,
    };
    SLANG_CHECK(!builder.supportsValueOperation(operation));
    operation = {
        SlangNVVMValueOperation(66),
        NVVMSemantics::kFloat16,
        halfOperands,
        1,
    };
    SLANG_CHECK(!builder.supportsValueOperation(operation));
    operation = {
        SlangNVVMValueOperation(49),
        NVVMSemantics::kUnsignedI32,
        &NVVMSemantics::kUnsignedI32,
        1,
    };
    SLANG_CHECK(!builder.supportsValueOperation(operation));

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(toSlice("scalar-math"), module.module)));
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle int32Type = nullptr;
    SlangNVVMTypeHandle halfType = nullptr;
    SlangNVVMTypeHandle float32Type = nullptr;
    SlangNVVMTypeHandle float64Type = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, int32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, 16, halfType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, 32, float32Type)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, 64, float64Type)));
    const SlangNVVMTypeHandle parameterTypes[] = {
        int32Type,
        halfType,
        float32Type,
        float64Type,
    };
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
        toSlice("useScalarMath"),
        function)));
    SlangNVVMValueHandle values[SLANG_COUNT_OF(parameterTypes)] = {};
    for (Index i = 0; i < SLANG_COUNT_OF(values); ++i)
    {
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, i, values[i])));
    }
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));

    SlangNVVMValueHandle result = nullptr;
    for (const char* name : {"__nv_floor", "__nv_rsqrt"})
    {
        auto mathBytes = _makeNVVMDeviceLibraryFixture(builder, 64, true, name);
        ScopedNVVMTestDeviceLibrary mathLibrary{&builder};
        String mathDiagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.loadDeviceLibrary(mathBytes, mathLibrary.library, mathDiagnostics)));
        const SlangNVVMNamedIntrinsicOperandDesc mathOperand = {
            NVVMSemantics::kFloat64,
            SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
        const SlangNVVMNamedIntrinsicDesc mathOperation =
            {name, strlen(name), NVVMSemantics::kFloat64, &mathOperand, 1};
        SLANG_CHECK_ABORT(
            builder.supportsDeviceLibraryFunction(mathLibrary.library, mathOperation));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitDeviceLibraryFunction(
            mathLibrary.library,
            module.module,
            mathOperation,
            &values[3],
            1,
            result)));
    }
    const SlangNVVMNamedIntrinsicOperandDesc sqrtOperand = {
        NVVMSemantics::kFloat64,
        SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
    const SlangNVVMNamedIntrinsicDesc sqrtOperation =
        {"llvm.sqrt", sizeof("llvm.sqrt") - 1, NVVMSemantics::kFloat64, &sqrtOperand, 1};
    SLANG_CHECK_ABORT(builder.supportsNamedIntrinsic(sqrtOperation));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitNamedIntrinsic(module.module, sqrtOperation, &values[3], 1, result)));
    const SlangNVVMValueHandle binaryOperands[] = {values[3], values[3]};
    for (auto mathOperation : binaryMathOperations)
    {
        operation = {mathOperation, NVVMSemantics::kFloat64, float64Operands, 2};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitValueOperation(module.module, operation, binaryOperands, 2, result)));
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    const SlangNVVMSerializationFormat formats[] = {
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
    };
    for (auto format : formats)
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(text.indexOf("declare double @llvm.sqrt.f64(double)") >= 0);
        SLANG_CHECK(text.indexOf("call double @__nv_floor(double") >= 0);
        SLANG_CHECK(text.indexOf("call double @__nv_rsqrt(double") >= 0);
        SLANG_CHECK(text.indexOf("declare double @__nv_rsqrt(double)") >= 0);
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
    RefPtr<DownstreamCompilerSet> llvmCompilers;
    const SlangResult llvmResult = _queryLLVM21(unitTestContext, llvmCompilers);
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

// Exercise the platform loader with real provider bytes: fake loaders do not decorate paths and
// therefore cannot catch a second .so suffix or fallback away from an explicitly selected file.
SLANG_UNIT_TEST(nvvmIRBuilderLoadsExactProviderFile)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    const String providerPath = SharedLibraryUtils::getSharedLibraryFileName(
        reinterpret_cast<void*>(preflightBuilder.getAPI().queryInterface));
    SLANG_CHECK_ABORT(File::exists(providerPath));
    NVVMIRBuilder exactBuilder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(
        providerPath,
        DefaultSharedLibraryLoader::getSingleton(),
        exactBuilder)));
    SLANG_CHECK(exactBuilder.isInitialized());

    TempDirectory temporary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(temporary)));
    const String explicitFile = Path::combine(temporary.path, "nvvm-provider-shadow.invalid");
    const String decoratedFile = SharedLibrary::calcPlatformPath(explicitFile.getUnownedSlice());
    List<unsigned char> providerBytes;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllBytes(providerPath, providerBytes)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::writeAllBytes(decoratedFile, providerBytes.getBuffer(), providerBytes.getCount())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(explicitFile, "invalid provider")));

    // A valid decorated sibling proves that loading the undecorated explicit file would succeed
    // incorrectly if the loader ignored the existing file and normalized its name instead.
    ComPtr<ISlangSharedLibrary> sibling;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(DefaultSharedLibraryLoader::getSingleton()->loadPlatformSharedLibrary(
            decoratedFile.getBuffer(),
            sibling.writeRef())));
    ComPtr<ISlangSharedLibrary> invalidLibrary;
    SLANG_CHECK(SLANG_FAILED(DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(
        explicitFile.getBuffer(),
        invalidLibrary.writeRef())));
    SLANG_CHECK(invalidLibrary == nullptr);
}

// FP8 descriptors preserve format; widening does not admit reverse or generic numeric casts.
SLANG_UNIT_TEST(nvvmIRBuilderFloat8TransportContract)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (auto fp8 : {NVVMSemantics::kFloatE4M3, NVVMSemantics::kFloatE5M2})
    {
        for (auto bits :
             {NVVMSemantics::kSignedI8,
              NVVMSemantics::kUnsignedI8,
              NVVMSemantics::kFloatE4M3,
              NVVMSemantics::kFloatE5M2})
        {
            if (NVVMSemantics::areSameType(bits, fp8))
                continue;
            SlangNVVMValueOperationDesc decode =
                {SLANG_NVVM_VALUE_OP_BIT_REINTERPRET, fp8, &bits, 1};
            SlangNVVMValueOperationDesc encode =
                {SLANG_NVVM_VALUE_OP_BIT_REINTERPRET, bits, &fp8, 1};
            SLANG_CHECK(builder.supportsValueOperation(decode));
            SLANG_CHECK(builder.supportsValueOperation(encode));
        }
        SlangNVVMValueTypeDesc operands[] = {NVVMSemantics::kBool, fp8, fp8};
        SlangNVVMValueOperationDesc select = {SLANG_NVVM_VALUE_OP_SELECT, fp8, operands, 3};
        SLANG_CHECK(builder.supportsValueOperation(select));
        operands[2] = NVVMSemantics::areSameType(fp8, NVVMSemantics::kFloatE4M3)
                          ? NVVMSemantics::kFloatE5M2
                          : NVVMSemantics::kFloatE4M3;
        SLANG_CHECK(!builder.supportsValueOperation(select));
        operands[2] = fp8;
        operands[0] = NVVMSemantics::kUnsignedI8;
        SLANG_CHECK(!builder.supportsValueOperation(select));
        for (auto numeric :
             {NVVMSemantics::kFloat16,
              NVVMSemantics::kFloat32,
              NVVMSemantics::kFloat64,
              NVVMSemantics::kBFloat16,
              NVVMSemantics::kUnsignedI8,
              NVVMSemantics::kSignedI32})
        {
            for (auto operation :
                 {SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
                  SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT,
                  SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER})
            {
                SlangNVVMValueOperationDesc narrow = {operation, fp8, &numeric, 1};
                SlangNVVMValueOperationDesc widen = {operation, numeric, &fp8, 1};
                SLANG_CHECK(!builder.supportsValueOperation(narrow));
                const bool isQualifiedWiden =
                    operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT &&
                    NVVMSemantics::areSameType(numeric, NVVMSemantics::kFloat32);
                SLANG_CHECK(builder.supportsValueOperation(widen) == isQualifiedWiden);
            }
        }
        SlangNVVMValueTypeDesc binary[] = {fp8, fp8};
        for (auto operation :
             {SLANG_NVVM_VALUE_OP_ADD,
              SLANG_NVVM_VALUE_OP_SUBTRACT,
              SLANG_NVVM_VALUE_OP_MULTIPLY,
              SLANG_NVVM_VALUE_OP_DIVIDE})
        {
            SlangNVVMValueOperationDesc arithmetic = {operation, fp8, binary, 2};
            SLANG_CHECK(!builder.supportsValueOperation(arithmetic));
        }
        auto malformed = fp8;
        malformed.laneCount = 2;
        SlangNVVMValueOperationDesc vectorBits =
            {SLANG_NVVM_VALUE_OP_BIT_REINTERPRET, malformed, &NVVMSemantics::kUnsignedI16, 1};
        SLANG_CHECK(!builder.supportsValueOperation(vectorBits));
        malformed.laneCount = 1;
        malformed.bitWidth = 16;
        vectorBits.resultType = malformed;
        SLANG_CHECK(!builder.supportsValueOperation(vectorBits));
        SlangNVVMValueOperationDesc wrongWidth =
            {SLANG_NVVM_VALUE_OP_BIT_REINTERPRET, fp8, &NVVMSemantics::kUnsignedI16, 1};
        SLANG_CHECK(!builder.supportsValueOperation(wrongWidth));
    }
}

// The real provider accepts scalar FP8 widening and rejects adjacent unqualified descriptors.
SLANG_UNIT_TEST(nvvmIRBuilderFloat8WideningContract)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (auto fp8 : {NVVMSemantics::kFloatE4M3, NVVMSemantics::kFloatE5M2})
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fp8-widen"), scope.module)));
        SlangNVVMTypeHandle byteType = nullptr;
        SlangNVVMTypeHandle floatType = nullptr;
        SlangNVVMTypeHandle functionType = nullptr;
        SlangNVVMValueHandle function = nullptr;
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 8, byteType)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 32, floatType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, floatType, &byteType, 1, functionType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("expand"),
            function)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        SlangNVVMValueHandle value = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, value)));
        SlangNVVMValueOperationDesc widen =
            {SLANG_NVVM_VALUE_OP_FLOAT_CONVERT, NVVMSemantics::kFloat32, &fp8, 1};
        SLANG_CHECK_ABORT(builder.supportsValueOperation(widen));
        SlangNVVMValueHandle widened = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitValueOperation(scope.module, widen, &value, 1, widened)));

        // Rejected descriptors must not produce values or invalidate the eventual function.
        const SlangNVVMValueTypeDesc wrongOperands[] = {
            {fp8.kind, 16, 1},
            {fp8.kind, 8, 0},
            {fp8.kind, 8, 2},
            {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 8, 1},
            NVVMSemantics::kUnsignedI8,
        };
        for (const auto& operand : wrongOperands)
        {
            auto rejected = widen;
            rejected.operandTypes = &operand;
            SLANG_CHECK(!builder.supportsValueOperation(rejected));
            SlangNVVMValueHandle invalid = nullptr;
            SLANG_CHECK(SLANG_FAILED(
                builder.emitValueOperation(scope.module, rejected, &value, 1, invalid)));
            SLANG_CHECK(invalid == nullptr);
        }
        const SlangNVVMValueTypeDesc wrongResults[] = {
            NVVMSemantics::kFloat16,
            NVVMSemantics::kFloat64,
            NVVMSemantics::kBFloat16,
            {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, 0},
            {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, 2},
        };
        for (const auto& result : wrongResults)
        {
            auto rejected = widen;
            rejected.resultType = result;
            SLANG_CHECK(!builder.supportsValueOperation(rejected));
            SlangNVVMValueHandle invalid = nullptr;
            SLANG_CHECK(SLANG_FAILED(
                builder.emitValueOperation(scope.module, rejected, &value, 1, invalid)));
            SLANG_CHECK(invalid == nullptr);
        }
        auto wrongArity = widen;
        wrongArity.operandCount = 0;
        SLANG_CHECK(!builder.supportsValueOperation(wrongArity));
        SlangNVVMValueHandle invalidArity = nullptr;
        SLANG_CHECK(SLANG_FAILED(
            builder.emitValueOperation(scope.module, wrongArity, nullptr, 0, invalidArity)));
        SLANG_CHECK(invalidArity == nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, widened)));
        for (auto format :
             {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
              SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
        {
            ComPtr<ISlangBlob> assembly;
            String diagnostics;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.serializeModule(scope.module, format, assembly, diagnostics)));
            SLANG_CHECK(diagnostics.getLength() == 0);
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderNamedIntrinsicSignaturesArePure)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(toSlice("named-query"), scope.module)));
    ComPtr<ISlangBlob> before;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, before)));
    struct Case
    {
        const char* name;
        SlangNVVMValueTypeDesc result;
        size_t operandCount;
        bool supported;
    };
    const Case cases[] = {
        {"llvm.nvvm.read.ptx.sreg.tid.x", NVVMSemantics::kUnsignedI32, 0, true},
        {"llvm.nvvm.read.ptx.sreg.tid.x", NVVMSemantics::kSignedI32, 0, true},
        {"llvm.nvvm.read.ptx.sreg.nctaid.z", NVVMSemantics::kUnsignedI32, 0, true},
        {"llvm.nvvm.read.ptx.sreg.tid.x.extra", NVVMSemantics::kUnsignedI32, 0, false},
        {"llvm.nvvm.read.ptx.sreg.missing.x", NVVMSemantics::kUnsignedI32, 0, false},
        {"llvm.nvvm.read.ptx.sreg.tid.x()", NVVMSemantics::kUnsignedI32, 0, false},
        {"llvm.ctpop.i32", NVVMSemantics::kUnsignedI32, 0, false},
        {"llvm.nvvm.read.ptx.sreg.tid.x", NVVMSemantics::kFloat32, 0, false},
        {"llvm.nvvm.read.ptx.sreg.tid.x", NVVMSemantics::kUnsignedI32x3, 0, false},
        {"llvm.nvvm.read.ptx.sreg.tid.x",
         {SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER, 64, 1},
         0,
         false},
        {"llvm.nvvm.read.ptx.sreg.tid.x", NVVMSemantics::kUnsignedI32, 1, false},
        {"llvm.nvvm.barrier0", NVVMSemantics::kVoid, 0, true},
        {"llvm.nvvm.membar.gl", NVVMSemantics::kVoid, 0, true},
        {"llvm.nvvm.membar.cta", NVVMSemantics::kVoid, 0, true},
        {"llvm.nvvm.barrier0", NVVMSemantics::kUnsignedI32, 0, false},
        {"llvm.nvvm.membar.gl", NVVMSemantics::kFloat32, 0, false},
        {"llvm.nvvm.membar.cta", NVVMSemantics::kVoid, 1, false},
        {"llvm.nvvm.barrier0", {SLANG_NVVM_VALUE_TYPE_VOID, 32, 0}, 0, false},
        {"llvm.nvvm.membar.gl", {SLANG_NVVM_VALUE_TYPE_VOID, 0, 1}, 0, false},
        {"llvm.nvvm.read.ptx.sreg.tid.x", NVVMSemantics::kVoid, 0, false},
        {"llvm.nvvm.barrier0()", NVVMSemantics::kVoid, 0, false},
        {"llvm.nvvm.membar.missing", NVVMSemantics::kVoid, 0, false},
        {"llvm.nvvm.membar.sys", NVVMSemantics::kVoid, 0, false},
    };
    const SlangNVVMNamedIntrinsicOperandDesc unusedOperand = {
        NVVMSemantics::kUnsignedI32,
        SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
    for (const auto& testCase : cases)
    {
        SlangNVVMNamedIntrinsicDesc intrinsic = {
            testCase.name,
            strlen(testCase.name),
            testCase.result,
            &unusedOperand,
            testCase.operandCount};
        SLANG_CHECK(builder.supportsNamedIntrinsic(intrinsic) == testCase.supported);
        if (!testCase.supported)
        {
            SlangNVVMValueHandle value = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
            SLANG_CHECK(
                builder.emitNamedIntrinsic(scope.module, intrinsic, nullptr, 0, value) ==
                SLANG_E_NOT_AVAILABLE);
            SLANG_CHECK(value == nullptr);
        }
    }
    const char embeddedNull[] = "llvm.nvvm.read.ptx.sreg.tid.x\0suffix";
    SlangNVVMNamedIntrinsicDesc malformed =
        {embeddedNull, sizeof(embeddedNull) - 1, NVVMSemantics::kUnsignedI32, nullptr, 0};
    SLANG_CHECK(!builder.supportsNamedIntrinsic(malformed));
    ComPtr<ISlangBlob> after;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, after)));
    SLANG_CHECK(_getBlobText(before) == _getBlobText(after));

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionType(scope.module, voidType, nullptr, 0, functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("queryHost"),
        function)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

    for (const auto& testCase : cases)
    {
        if (testCase.supported)
            continue;
        SlangNVVMNamedIntrinsicDesc intrinsic = {
            testCase.name,
            strlen(testCase.name),
            testCase.result,
            &unusedOperand,
            testCase.operandCount};
        SlangNVVMValueHandle value = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        // Bypass wrapper preflight at a valid insertion point to test provider rejection itself.
        const SlangNVVMValueHandle unusedValue = nullptr;
        SLANG_CHECK(
            builder.getValueOperationsAPI()->emitNamedIntrinsic(
                scope.module,
                &intrinsic,
                &unusedValue,
                intrinsic.operandCount,
                &value) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(value == nullptr);
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    ComPtr<ISlangBlob> completed;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        completed)));
    const String text = _getBlobText(completed);
    SLANG_CHECK(!text.contains("@llvm."));
    SLANG_CHECK(!text.contains("call "));
    SLANG_CHECK(text.contains("ret void"));
}

SLANG_UNIT_TEST(nvvmIRBuilderNamedIntegerIntrinsicSignaturesArePure)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("integer-query"), scope.module)));
    ComPtr<ISlangBlob> before;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, before)));
    const auto api = builder.getValueOperationsAPI();
    for (uint32_t width : {8u, 16u, 32u, 64u})
    {
        for (const char* name : {"llvm.ctpop", "llvm.bitreverse", "llvm.ctlz", "llvm.cttz"})
        {
            const bool isScan = String(name) == "llvm.ctlz" || String(name) == "llvm.cttz";
            for (bool isSigned : {false, true})
            {
                SlangNVVMValueTypeDesc type = {
                    isSigned ? SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                             : SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
                    width,
                    1};
                SlangNVVMNamedIntrinsicOperandDesc operands[] = {
                    {type, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE},
                    {NVVMSemantics::kBool, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT},
                    {type, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE},
                };
                SlangNVVMNamedIntrinsicDesc desc =
                    {name, strlen(name), type, operands, isScan ? 2u : 1u};
                SLANG_CHECK(builder.supportsNamedIntrinsic(desc));
                // LLVM integers are signless: Slang owns the signed high-index formula.
                operands[0].type.kind = isSigned ? SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER
                                                 : SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER;
                SLANG_CHECK(builder.supportsNamedIntrinsic(desc));
                operands[0].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT;
                SLANG_CHECK(builder.supportsNamedIntrinsic(desc));
                operands[0].kind = 99;
                SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                operands[0].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE;
                operands[0].type.bitWidth = width == 64 ? 32 : 64;
                SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                operands[0].type = type;
                --desc.operandCount;
                SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                desc.operandCount += 2;
                SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                --desc.operandCount;
                if (isScan)
                {
                    operands[1].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE;
                    SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                    operands[1].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT;
                }
                const SlangNVVMValueTypeDesc invalidTypes[] = {
                    NVVMSemantics::kBool,
                    NVVMSemantics::kFloat16,
                    NVVMSemantics::kFloat32,
                    {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 1},
                    {SLANG_NVVM_VALUE_TYPE_FLOAT_E4M3, 8, 1},
                    {SLANG_NVVM_VALUE_TYPE_FLOAT_E5M2, 8, 1},
                    {SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER, 24, 1},
                    {SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER, width, 2},
                    NVVMSemantics::kVoid,
                    {SlangNVVMValueTypeKind(99), width, 1}};
                for (const auto& invalidType : invalidTypes)
                {
                    desc.resultType = invalidType;
                    SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                    operands[0].type = invalidType;
                    desc.resultType = invalidType;
                    SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                    desc.resultType = type;
                    SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                    operands[0].type = type;
                }
                for (const char* invalidName :
                     {"llvm.ctpop.i32",
                      "llvm.ctpop.",
                      "llvm.ctpop()",
                      " llvm.ctpop",
                      "llvm.missing"})
                {
                    desc.name = invalidName;
                    desc.nameSize = strlen(invalidName);
                    SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                }
                const char embeddedNull[] = "llvm.ctpop\0suffix";
                desc.name = embeddedNull;
                desc.nameSize = sizeof(embeddedNull) - 1;
                SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
                desc.name = name;
                desc.nameSize = strlen(name);
                desc.operands = nullptr;
                uint32_t supported = 77;
                SLANG_CHECK(
                    api->isNamedIntrinsicSupported(&desc, &supported) == SLANG_E_INVALID_ARG);
                SLANG_CHECK(supported == 0);
            }
        }
    }
    uint32_t supported = 77;
    SLANG_CHECK(api->isNamedIntrinsicSupported(nullptr, &supported) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(supported == 0);
    ComPtr<ISlangBlob> after;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, after)));
    SLANG_CHECK(_getBlobText(before) == _getBlobText(after));
}

SLANG_UNIT_TEST(nvvmIRBuilderNamedIntegerIntrinsicsPreserveOperandContracts)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("integer-operands"), scope.module)));
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle types[5] = {};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    const uint32_t widths[] = {8, 16, 32, 64, 1};
    for (Index i = 0; i < SLANG_COUNT_OF(widths); ++i)
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getIntegerType(scope.module, widths[i], types[i])));
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .getFunctionType(scope.module, voidType, types, SLANG_COUNT_OF(types), functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        scope.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("useBits"),
        function)));
    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
    SlangNVVMValueHandle parameters[5] = {};
    for (Index i = 0; i < SLANG_COUNT_OF(parameters); ++i)
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(scope.module, function, i, parameters[i])));
    SlangNVVMValueHandle flags[2] = {};
    for (uint64_t i = 0; i < 2; ++i)
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getIntegerConstant(scope.module, types[4], i, flags[i])));
    const auto api = builder.getValueOperationsAPI();
    for (Index i = 0; i < 4; ++i)
    {
        const SlangNVVMValueTypeDesc type = {SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER, widths[i], 1};
        for (const char* name : {"llvm.ctpop", "llvm.bitreverse", "llvm.ctlz", "llvm.cttz"})
        {
            const bool isScan = String(name) == "llvm.ctlz" || String(name) == "llvm.cttz";
            SlangNVVMNamedIntrinsicOperandDesc metadata[] = {
                {type, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE},
                {NVVMSemantics::kBool, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT}};
            SlangNVVMNamedIntrinsicDesc desc =
                {name, strlen(name), type, metadata, isScan ? 2u : 1u};
            SlangNVVMValueHandle operands[] = {parameters[i], flags[0]};
            SlangNVVMValueHandle value = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
            SLANG_CHECK(
                api->emitNamedIntrinsic(scope.module, &desc, nullptr, desc.operandCount, &value) ==
                SLANG_E_INVALID_ARG);
            SLANG_CHECK(value == nullptr);
            SLANG_CHECK(
                api->emitNamedIntrinsic(scope.module, &desc, operands, 0, &value) ==
                SLANG_E_INVALID_ARG);
            SLANG_CHECK(value == nullptr);
            metadata[0].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT;
            SLANG_CHECK(
                api->emitNamedIntrinsic(scope.module, &desc, operands, desc.operandCount, &value) ==
                SLANG_E_INVALID_ARG);
            SLANG_CHECK(value == nullptr);
            metadata[0].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE;
            operands[0] = parameters[(i + 1) % 4];
            SLANG_CHECK(
                api->emitNamedIntrinsic(scope.module, &desc, operands, desc.operandCount, &value) ==
                SLANG_E_INVALID_ARG);
            SLANG_CHECK(value == nullptr);
            operands[0] = parameters[i];
            if (isScan)
            {
                operands[1] = parameters[4];
                SLANG_CHECK(
                    api->emitNamedIntrinsic(
                        scope.module,
                        &desc,
                        operands,
                        desc.operandCount,
                        &value) == SLANG_E_INVALID_ARG);
                SLANG_CHECK(value == nullptr);
                operands[1] = flags[0];
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder
                    .emitNamedIntrinsic(scope.module, desc, operands, desc.operandCount, value)));
            SLANG_CHECK(value != nullptr);
            if (isScan)
            {
                // A true flag is valid with a nonzero input; this unit only serializes the call.
                operands[1] = flags[1];
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitNamedIntrinsic(
                    scope.module,
                    desc,
                    operands,
                    desc.operandCount,
                    value)));
            }
        }
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    for (auto format :
         {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
          SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("call i")) == 24);
        for (uint32_t width : {8u, 16u, 32u, 64u})
        {
            for (const char* name : {"llvm.ctpop", "llvm.bitreverse", "llvm.ctlz", "llvm.cttz"})
            {
                StringBuilder declaration;
                declaration << "@" << name << ".i" << width;
                SLANG_CHECK(text.contains(declaration.getBuffer()));
            }
        }
        SLANG_CHECK(text.contains("i1 false"));
        SLANG_CHECK(text.contains("i1 true"));
        SLANG_CHECK(
            text.contains("immarg") == (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY));
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderNamedIntegerIntrinsicsRejectForeignValuesWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    String control;
    for (bool injectFailures : {false, true})
    {
        ScopedNVVMBuilderModule scope, foreign;
        scope.builder = foreign.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("operand-ownership"), scope.module)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("foreign"), foreign.module)));
        SlangNVVMTypeHandle voidType = nullptr, i32 = nullptr, i1 = nullptr, foreignI32 = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, i32)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 1, i1)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(foreign.module, 32, foreignI32)));
        SlangNVVMValueHandle foreignConstant = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getIntegerConstant(foreign.module, foreignI32, 1, foreignConstant)));
        const SlangNVVMTypeHandle parameterTypes[] = {i32, i1};
        SlangNVVMTypeHandle functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, voidType, parameterTypes, 2, functionType)));
        SlangNVVMValueHandle function = nullptr, otherFunction = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("host"),
            function)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("other"),
            otherFunction)));
        SlangNVVMValueHandle value = nullptr, condition = nullptr, otherParameter = nullptr,
                             one = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, value)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, condition)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(scope.module, otherFunction, 0, otherParameter)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerConstant(scope.module, i32, 1, one)));
        SlangNVVMBlockHandle entry = nullptr, left = nullptr, right = nullptr, merge = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), entry)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("left"), left)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("right"), right)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("merge"), merge)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entry)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitConditionalBranch(scope.module, condition, left, right)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, left)));
        const SlangNVVMValueTypeDesc types[] = {
            NVVMSemantics::kUnsignedI32,
            NVVMSemantics::kUnsignedI32};
        const SlangNVVMValueOperationDesc add =
            {SLANG_NVVM_VALUE_OP_ADD, NVVMSemantics::kUnsignedI32, types, 2};
        const SlangNVVMValueHandle addOperands[] = {value, one};
        SlangNVVMValueHandle leftValue = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitValueOperation(scope.module, add, addOperands, 2, leftValue)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(scope.module, merge)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, right)));
        if (injectFailures)
        {
            const SlangNVVMNamedIntrinsicOperandDesc operand = {
                NVVMSemantics::kUnsignedI32,
                SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
            const char* name = "llvm.bitreverse";
            const SlangNVVMNamedIntrinsicDesc intrinsic =
                {name, strlen(name), NVVMSemantics::kUnsignedI32, &operand, 1};
            for (SlangNVVMValueHandle invalid :
                 {foreignConstant, otherParameter, leftValue, SlangNVVMValueHandle(nullptr)})
            {
                SlangNVVMValueHandle result = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                SLANG_CHECK(
                    builder.getValueOperationsAPI()
                        ->emitNamedIntrinsic(scope.module, &intrinsic, &invalid, 1, &result) ==
                    SLANG_E_INVALID_ARG);
                SLANG_CHECK(result == nullptr);
            }
            SLANG_CHECK(
                builder.getValueOperationsAPI()
                    ->emitNamedIntrinsic(scope.module, &intrinsic, &value, 1, nullptr) ==
                SLANG_E_INVALID_ARG);
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(scope.module, merge)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, merge)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
            assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(!text.contains("@llvm.bitreverse"));
        if (injectFailures)
        {
            SLANG_CHECK(text == control);
        }
        else
        {
            control = text;
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderNamedIntegerIntrinsicsRejectConflictingSymbols)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (Index conflictKind = 0; conflictKind < 3; ++conflictKind)
    {
        String controlAssembly, controlDiagnostics;
        for (bool injectFailure : {false, true})
        {
            ScopedNVVMBuilderModule scope;
            scope.builder = &builder;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createModule(toSlice("symbol-conflict"), scope.module)));
            SlangNVVMTypeHandle voidType = nullptr, i32 = nullptr, functionType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, i32)));
            const char* symbol = "llvm.ctpop.i32";
            SlangNVVMValueHandle conflicting = nullptr;
            if (conflictKind == 0)
            {
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionType(scope.module, voidType, nullptr, 0, functionType)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                    scope.module,
                    functionType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_FUNCTION_FLAG_NONE,
                    UnownedStringSlice(symbol),
                    conflicting)));
            }
            else if (conflictKind == 1)
            {
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareGlobalStorage(
                    scope.module,
                    i32,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
                    4,
                    UnownedStringSlice(symbol),
                    conflicting)));
            }
            else
            {
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionType(scope.module, i32, &i32, 1, functionType)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                    scope.module,
                    functionType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_FUNCTION_FLAG_NONE,
                    UnownedStringSlice(symbol),
                    conflicting)));
                SlangNVVMBlockHandle body = nullptr;
                SlangNVVMValueHandle parameter = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.createBlock(scope.module, conflicting, toSlice("body"), body)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, body)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(scope.module, conflicting, 0, parameter)));
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, parameter)));
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionType(scope.module, voidType, &i32, 1, functionType)));
            SlangNVVMValueHandle host = nullptr, parameter = nullptr;
            SlangNVVMBlockHandle entry = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                scope.module,
                functionType,
                SLANG_NVVM_LINKAGE_EXTERNAL,
                SLANG_NVVM_FUNCTION_FLAG_NONE,
                toSlice("host"),
                host)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, host, 0, parameter)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createBlock(scope.module, host, toSlice("entry"), entry)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entry)));
            if (injectFailure)
            {
                const SlangNVVMNamedIntrinsicOperandDesc operand = {
                    NVVMSemantics::kUnsignedI32,
                    SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
                const char* name = "llvm.ctpop";
                const SlangNVVMNamedIntrinsicDesc intrinsic =
                    {name, strlen(name), NVVMSemantics::kUnsignedI32, &operand, 1};
                SlangNVVMValueHandle result = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                SLANG_CHECK(
                    builder.getValueOperationsAPI()
                        ->emitNamedIntrinsic(scope.module, &intrinsic, &parameter, 1, &result) ==
                    SLANG_E_INVALID_ARG);
                SLANG_CHECK(result == nullptr);
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
            ComPtr<ISlangBlob> assembly;
            String diagnostics;
            const SlangResult serialized = builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                assembly,
                diagnostics);
            if (conflictKind != 2)
            {
                // LLVM verifies an unused intrinsic declaration without checking its signature,
                // and a same-named global is also valid. Compare complete module bytes to prove
                // rejecting the call did not introduce a declaration or instruction.
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(serialized));
                SLANG_CHECK_ABORT(assembly != nullptr);
                const String text = _getBlobText(assembly);
                if (injectFailure)
                {
                    SLANG_CHECK(text == controlAssembly);
                }
                else
                {
                    controlAssembly = text;
                }
            }
            else
            {
                // LLVM rejects an intrinsic definition even without our call. Compare the
                // completed-module verifier diagnostics instead.
                SLANG_CHECK(serialized == SLANG_FAIL);
                SLANG_CHECK(assembly == nullptr);
                SLANG_CHECK(diagnostics.getLength() != 0);
                if (injectFailure)
                {
                    SLANG_CHECK(diagnostics == controlDiagnostics);
                }
                else
                {
                    controlDiagnostics = diagnostics;
                }
            }
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderRequiresNamedIntrinsicInterface)
{
    for (bool query : {false, true})
    {
        _resetDirectNVVMFakes();
        if (query)
            gFakeNVVMBuilder.valueOperations.isNamedIntrinsicSupported = nullptr;
        else
            gFakeNVVMBuilder.valueOperations.emitNamedIntrinsic = nullptr;
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }
}


SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryUsesSelectedDefinitions)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const char* name :
         {"__nv_roundf",
          "__nv_rsqrtf",
          "__nv_rsqrt",
          "__nv_expf",
          "__nv_exp",
          "__nv_exp2f",
          "__nv_exp2",
          "__nv_logf",
          "__nv_log",
          "__nv_log2f",
          "__nv_log2",
          "__nv_log10f",
          "__nv_log10"})
        for (uint32_t width : {32u, 64u})
        {
            auto bytes = _makeNVVMDeviceLibraryFixture(builder, width, true, name);
            ScopedNVVMTestDeviceLibrary library{&builder};
            String diagnostics;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.loadDeviceLibrary(bytes, library.library, diagnostics)));
            SLANG_CHECK(diagnostics.getLength() == 0);
            bytes.setNull(); // Loading owns input storage and cannot borrow the caller's blob.
            SlangNVVMNamedIntrinsicOperandDesc operand = {
                {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, width, 1},
                SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
            SlangNVVMNamedIntrinsicDesc desc = {name, strlen(name), operand.type, &operand, 1};
            SLANG_CHECK(builder.supportsDeviceLibraryFunction(library.library, desc));
            for (uint32_t wrongWidth : {16u, width == 32 ? 64u : 32u})
            {
                desc.resultType.bitWidth = wrongWidth;
                SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            }
            desc.resultType = operand.type;
            if (width == 64)
            {
                desc.resultType = operand.type = NVVMSemantics::kFloat32;
                SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
                desc.resultType = operand.type = NVVMSemantics::kFloat64;
            }
            operand.type.bitWidth = width == 32 ? 64u : 32u;
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            operand.type.bitWidth = width;
            operand.type.laneCount = 2;
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            operand.type.laneCount = 1;
            operand.kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT;
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            operand.kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE;
            desc.operandCount = 0;
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            desc.operandCount = 1;
            SlangNVVMNamedIntrinsicOperandDesc extraOperands[] = {operand, operand};
            desc.operands = extraOperands;
            desc.operandCount = 2;
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            desc.operands = &operand;
            desc.operandCount = 1;
            operand.type = NVVMSemantics::kUnsignedI32;
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            operand.type = desc.resultType;
            desc.resultType = NVVMSemantics::kUnsignedI32;
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            desc.resultType = operand.type;
            desc.resultType.laneCount = 2;
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            desc.resultType.laneCount = 1;
            desc.name = "__nv_unadmitted";
            desc.nameSize = strlen(desc.name);
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            desc.name = String(name) == "__nv_roundf" ? "__nv_round" : "__nv_roundf";
            desc.nameSize = strlen(desc.name);
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
            desc.name = name;
            desc.nameSize = strlen(desc.name);

            ScopedNVVMBuilderModule output;
            output.builder = &builder;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createModule(toSlice("query-purity"), output.module)));
            ComPtr<ISlangBlob> before, after;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                output.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                before)));
            for (int i = 0; i < 3; ++i)
                SLANG_CHECK(builder.supportsDeviceLibraryFunction(library.library, desc));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                output.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                after)));
            SLANG_CHECK(_getBlobText(before) == _getBlobText(after));
        }
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryRejectsUnsupportedABI)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    struct Case
    {
        const uint8_t* bytes;
        size_t size;
    };
    const Case cases[] = {
        {kDeviceLibraryAvailableExternallyBitcode,
         sizeof(kDeviceLibraryAvailableExternallyBitcode)},
        {kDeviceLibraryFastccBitcode, sizeof(kDeviceLibraryFastccBitcode)},
        {kDeviceLibraryHiddenBitcode, sizeof(kDeviceLibraryHiddenBitcode)},
        {kDeviceLibraryInternalBitcode, sizeof(kDeviceLibraryInternalBitcode)},
        {kDeviceLibraryParameterInregBitcode, sizeof(kDeviceLibraryParameterInregBitcode)},
        {kDeviceLibraryReturnInregBitcode, sizeof(kDeviceLibraryReturnInregBitcode)},
        {kDeviceLibraryVariadicBitcode, sizeof(kDeviceLibraryVariadicBitcode)},
    };
    SlangNVVMNamedIntrinsicOperandDesc operand = {
        NVVMSemantics::kFloat32,
        SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
    SlangNVVMNamedIntrinsicDesc desc =
        {"__nv_roundf", strlen("__nv_roundf"), operand.type, &operand, 1};
    for (const char* name :
         {"__nv_roundf",
          "__nv_rsqrtf",
          "__nv_rsqrt",
          "__nv_expf",
          "__nv_exp",
          "__nv_exp2f",
          "__nv_exp2",
          "__nv_logf",
          "__nv_log",
          "__nv_log2f",
          "__nv_log2",
          "__nv_log10f",
          "__nv_log10"})
        for (uint32_t width : {32u, 64u})
        {
            auto declaration = _makeNVVMDeviceLibraryFixture(builder, width, false, name);
            ScopedNVVMTestDeviceLibrary library{&builder};
            String diagnostics;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.loadDeviceLibrary(declaration, library.library, diagnostics)));
            SlangNVVMNamedIntrinsicOperandDesc declarationOperand = {
                {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, width, 1},
                SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
            const SlangNVVMNamedIntrinsicDesc declarationDesc =
                {name, strlen(name), declarationOperand.type, &declarationOperand, 1};
            SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, declarationDesc));
        }
    for (auto testCase : cases)
    {
        ScopedNVVMTestDeviceLibrary library{&builder};
        String diagnostics;
        auto bytes = RawBlob::create(testCase.bytes, testCase.size);
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.loadDeviceLibrary(bytes, library.library, diagnostics)));
        SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryPreservesParseDiagnostics)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    const uint8_t corrupt[] = {0x42, 0x43, 0xc0, 0xde, 0xff};
    for (bool invalidIR : {false, true})
    {
        const void* data = invalidIR ? kDeviceLibraryInvalidFastccVariadicBitcode : corrupt;
        size_t size =
            invalidIR ? sizeof(kDeviceLibraryInvalidFastccVariadicBitcode) : sizeof(corrupt);
        auto bytes = RawBlob::create(data, size);
        ScopedNVVMTestDeviceLibrary library{&builder};
        String diagnostics;
        SLANG_CHECK(SLANG_FAILED(builder.loadDeviceLibrary(bytes, library.library, diagnostics)));
        SLANG_CHECK(library.library == nullptr);
        SLANG_CHECK(diagnostics.getLength() != 0);
        bytes.setNull();
        if (invalidIR)
            SLANG_CHECK(diagnostics.contains("Calling convention does not support"));
        SLANG_CHECK(
            SLANG_FAILED(builder.getValueOperationsAPI()
                             ->loadDeviceLibrary(data, size, &library.library, nullptr, nullptr)));
        SLANG_CHECK(library.library == nullptr);
    }
    SlangNVVMDeviceLibraryHandle library = nullptr;
    SLANG_CHECK(
        builder.getValueOperationsAPI()
            ->loadDeviceLibrary(nullptr, 1, &library, nullptr, nullptr) == SLANG_E_INVALID_ARG);
    SlangNVVMNamedIntrinsicDesc intrinsic = {
        "llvm.nvvm.read.ptx.sreg.tid.x",
        strlen("llvm.nvvm.read.ptx.sreg.tid.x"),
        NVVMSemantics::kUnsignedI32,
        nullptr,
        0};
    SLANG_CHECK(builder.supportsNamedIntrinsic(intrinsic));
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryEmitsAndRejectsWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const char* name :
         {"__nv_roundf",
          "__nv_rsqrtf",
          "__nv_rsqrt",
          "__nv_expf",
          "__nv_exp",
          "__nv_exp2f",
          "__nv_exp2",
          "__nv_logf",
          "__nv_log",
          "__nv_log2f",
          "__nv_log2",
          "__nv_log10f",
          "__nv_log10"})
        for (uint32_t width : {32u, 64u})
        {
            const uint32_t wrongWidth = width == 32 ? 64u : 32u;
            auto bytes = _makeNVVMDeviceLibraryFixture(builder, width, true, name);
            ScopedNVVMTestDeviceLibrary library{&builder};
            String diagnostics;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.loadDeviceLibrary(bytes, library.library, diagnostics)));
            SlangNVVMNamedIntrinsicOperandDesc operand = {
                {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, width, 1},
                SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
            SlangNVVMNamedIntrinsicDesc desc = {name, strlen(name), operand.type, &operand, 1};
            for (int rejection = 0; rejection < 8; ++rejection)
            {
                String baseline;
                for (bool inject : {false, true})
                {
                    ScopedNVVMBuilderModule output, foreign;
                    output.builder = foreign.builder = &builder;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        builder.createModule(toSlice("library-emission"), output.module)));
                    SLANG_CHECK_ABORT(
                        SLANG_SUCCEEDED(builder.createModule(toSlice("foreign"), foreign.module)));
                    SlangNVVMTypeHandle type = nullptr, functionType = nullptr,
                                        foreignType = nullptr;
                    SlangNVVMValueHandle host = nullptr, value = nullptr, foreignValue = nullptr;
                    SlangNVVMBlockHandle block = nullptr;
                    SLANG_CHECK_ABORT(
                        SLANG_SUCCEEDED(builder.getFloatingPointType(output.module, width, type)));
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        builder.getFunctionType(output.module, type, &type, 1, functionType)));
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                        output.module,
                        functionType,
                        SLANG_NVVM_LINKAGE_EXTERNAL,
                        SLANG_NVVM_FUNCTION_FLAG_NONE,
                        toSlice("host"),
                        host)));
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        builder.getFunctionParameter(output.module, host, 0, value)));
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        builder.createBlock(output.module, host, toSlice("entry"), block)));
                    SLANG_CHECK_ABORT(
                        SLANG_SUCCEEDED(builder.setInsertBlock(output.module, block)));
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        builder.getFloatingPointType(foreign.module, width, foreignType)));
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointConstant(
                        foreign.module,
                        foreignType,
                        width,
                        0,
                        foreignValue)));
                    if (rejection == 3)
                    {
                        SlangNVVMTypeHandle wrongType = nullptr, wrongFunction = nullptr;
                        SlangNVVMValueHandle collision = nullptr;
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                            builder.getFloatingPointType(output.module, wrongWidth, wrongType)));
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
                            output.module,
                            wrongType,
                            &wrongType,
                            1,
                            wrongFunction)));
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                            output.module,
                            wrongFunction,
                            SLANG_NVVM_LINKAGE_EXTERNAL,
                            SLANG_NVVM_FUNCTION_FLAG_NONE,
                            UnownedStringSlice(name),
                            collision)));
                    }
                    SlangNVVMValueHandle rejectedArgument = value;
                    if (rejection == 2)
                        rejectedArgument = foreignValue;
                    if (rejection == 4)
                    {
                        SlangNVVMTypeHandle wrongType = nullptr;
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                            builder.getFloatingPointType(output.module, wrongWidth, wrongType)));
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointConstant(
                            output.module,
                            wrongType,
                            wrongWidth,
                            0,
                            rejectedArgument)));
                    }
                    if (rejection == 5)
                    {
                        SlangNVVMValueHandle collision = nullptr;
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareGlobalStorage(
                            output.module,
                            type,
                            SLANG_NVVM_LINKAGE_INTERNAL,
                            SLANG_NVVM_ADDRESS_SPACE_SHARED,
                            4,
                            UnownedStringSlice(name),
                            collision)));
                    }
                    if (rejection == 6)
                    {
                        SlangNVVMValueHandle collision = nullptr, parameter = nullptr;
                        SlangNVVMBlockHandle body = nullptr;
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                            output.module,
                            functionType,
                            SLANG_NVVM_LINKAGE_EXTERNAL,
                            SLANG_NVVM_FUNCTION_FLAG_NONE,
                            UnownedStringSlice(name),
                            collision)));
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                            builder.getFunctionParameter(output.module, collision, 0, parameter)));
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                            builder.createBlock(output.module, collision, toSlice("body"), body)));
                        SLANG_CHECK_ABORT(
                            SLANG_SUCCEEDED(builder.setInsertBlock(output.module, body)));
                        SLANG_CHECK_ABORT(
                            SLANG_SUCCEEDED(builder.emitValueReturn(output.module, parameter)));
                        SLANG_CHECK_ABORT(
                            SLANG_SUCCEEDED(builder.setInsertBlock(output.module, block)));
                    }
                    if (rejection == 7)
                    {
                        SlangNVVMBlockHandle detached = nullptr;
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createBlock(
                            output.module,
                            host,
                            toSlice("unreachable"),
                            detached)));
                        SLANG_CHECK_ABORT(
                            SLANG_SUCCEEDED(builder.setInsertBlock(output.module, detached)));
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitDeviceLibraryFunction(
                            library.library,
                            output.module,
                            desc,
                            &value,
                            1,
                            rejectedArgument)));
                        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                            builder.emitValueReturn(output.module, rejectedArgument)));
                        SLANG_CHECK_ABORT(
                            SLANG_SUCCEEDED(builder.setInsertBlock(output.module, block)));
                    }
                    SlangNVVMValueHandle result = nullptr;
                    if (inject)
                    {
                        if (rejection == 0)
                        {
                            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitDeviceLibraryFunction(
                                library.library,
                                output.module,
                                desc,
                                &value,
                                1,
                                result)));
                            value = result;
                            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitDeviceLibraryFunction(
                                library.library,
                                output.module,
                                desc,
                                &value,
                                1,
                                result)));
                            value = result;
                        }
                        else
                        {
                            const auto argument = rejectedArgument;
                            result = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                            SLANG_CHECK(SLANG_FAILED(
                                builder.getValueOperationsAPI()->emitDeviceLibraryFunction(
                                    library.library,
                                    output.module,
                                    &desc,
                                    &argument,
                                    rejection == 1 ? 0 : 1,
                                    &result)));
                            SLANG_CHECK(result == nullptr);
                        }
                    }
                    SLANG_CHECK_ABORT(
                        SLANG_SUCCEEDED(builder.emitValueReturn(output.module, value)));
                    ComPtr<ISlangBlob> assembly;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                        output.module,
                        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                        assembly)));
                    if (!inject)
                    {
                        baseline = _getBlobText(assembly);
                    }
                    else if (rejection == 0)
                    {
                        StringBuilder expected;
                        expected << "call " << (width == 32 ? "float" : "double") << " @" << name
                                 << "(";
                        SLANG_CHECK(
                            _countOccurrences(
                                _getBlobText(assembly).getUnownedSlice(),
                                expected.getUnownedSlice()) == 2);
                    }
                    else
                    {
                        SLANG_CHECK(_getBlobText(assembly) == baseline);
                    }
                }
            }
        }
}

SLANG_UNIT_TEST(nvvmIRBuilderRequiresDeviceLibraryInterface)
{
    for (int missing = 0; missing < 4; ++missing)
    {
        _resetDirectNVVMFakes();
        switch (missing)
        {
        case 0:
            gFakeNVVMBuilder.valueOperations.loadDeviceLibrary = nullptr;
            break;
        case 1:
            gFakeNVVMBuilder.valueOperations.destroyDeviceLibrary = nullptr;
            break;
        case 2:
            gFakeNVVMBuilder.valueOperations.isDeviceLibraryFunctionSupported = nullptr;
            break;
        case 3:
            gFakeNVVMBuilder.valueOperations.emitDeviceLibraryFunction = nullptr;
            break;
        }
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsRetiredRoundWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto type :
         {NVVMSemantics::kFloat16, NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
    {
        const SlangNVVMValueOperationDesc operation = {SlangNVVMValueOperation(64), type, &type, 1};
        SLANG_CHECK(!builder.supportsValueOperation(operation));
        String control;
        for (bool attempt : {false, true})
        {
            ScopedNVVMBuilderModule module;
            module.builder = &builder;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createModule(toSlice("retired-round"), module.module)));
            SlangNVVMTypeHandle valueType = nullptr, functionType = nullptr;
            SlangNVVMValueHandle function = nullptr, parameter = nullptr;
            SlangNVVMBlockHandle block = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFloatingPointType(module.module, type.bitWidth, valueType)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionType(module.module, valueType, &valueType, 1, functionType)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                module.module,
                functionType,
                SLANG_NVVM_LINKAGE_EXTERNAL,
                SLANG_NVVM_FUNCTION_FLAG_NONE,
                toSlice("identity"),
                function)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionParameter(module.module, function, 0, parameter)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.createBlock(module.module, function, toSlice("entry"), block)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));
            if (attempt)
            {
                SlangNVVMValueHandle rejected = parameter;
                SLANG_CHECK(SLANG_FAILED(
                    builder.emitValueOperation(module.module, operation, &parameter, 1, rejected)));
                SLANG_CHECK(!rejected);
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(module.module, parameter)));
            ComPtr<ISlangBlob> assembly;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                module.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                assembly)));
            if (attempt)
            {
                SLANG_CHECK(_getBlobText(assembly) == control);
            }
            else
            {
                control = _getBlobText(assembly);
            }
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsRetiredDirectedRoundingWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const uint32_t identity : {42u, 54u, 57u})
        for (const auto type :
             {NVVMSemantics::kFloat16, NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
        {
            const SlangNVVMValueOperationDesc operation =
                {SlangNVVMValueOperation(identity), type, &type, 1};
            SLANG_CHECK(!builder.supportsValueOperation(operation));
            auto unsupported = operation;
            unsupported.operandCount = 0;
            SLANG_CHECK(!builder.supportsValueOperation(unsupported));
            unsupported = operation;
            unsupported.resultType.laneCount = 2;
            SLANG_CHECK(!builder.supportsValueOperation(unsupported));
            unsupported = operation;
            unsupported.resultType.kind = SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER;
            SLANG_CHECK(!builder.supportsValueOperation(unsupported));
            String control;
            for (bool attempt : {false, true})
            {
                ScopedNVVMBuilderModule module;
                module.builder = &builder;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.createModule(toSlice("retired-directed-rounding"), module.module)));
                SlangNVVMTypeHandle valueType = nullptr, functionType = nullptr;
                SlangNVVMValueHandle function = nullptr, parameter = nullptr;
                SlangNVVMBlockHandle block = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFloatingPointType(module.module, type.bitWidth, valueType)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder
                        .getFunctionType(module.module, valueType, &valueType, 1, functionType)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                    module.module,
                    functionType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_FUNCTION_FLAG_NONE,
                    toSlice("identity"),
                    function)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(module.module, function, 0, parameter)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.createBlock(module.module, function, toSlice("entry"), block)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));
                if (attempt)
                {
                    SlangNVVMValueHandle rejected = parameter;
                    SLANG_CHECK(SLANG_FAILED(builder.emitValueOperation(
                        module.module,
                        operation,
                        &parameter,
                        1,
                        rejected)));
                    SLANG_CHECK(!rejected);
                }
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(builder.emitValueReturn(module.module, parameter)));
                ComPtr<ISlangBlob> assembly;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                    module.module,
                    SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                    assembly)));
                if (attempt)
                {
                    SLANG_CHECK(_getBlobText(assembly) == control);
                }
                else
                {
                    control = _getBlobText(assembly);
                }
            }
        }
}

// Check a selected definition through one live call in both LLVM serializers.
// Consider __nv_rsqrt with a double(double) definition: the module must return the selected
// call's result and declare that same type. The selected fixture owns the signature; the two
// serializers must preserve it without folding the runtime argument or replacing the call.
static void _checkNVVMDeviceLibraryTypedCall(
    NVVMIRBuilder& builder,
    const char* name,
    uint32_t width,
    const char* moduleName,
    uint32_t operandCount = 1)
{
    auto bytes = _makeNVVMDeviceLibraryFixture(builder, width, true, name, operandCount);
    ScopedNVVMTestDeviceLibrary library{&builder};
    String diagnostics;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.loadDeviceLibrary(bytes, library.library, diagnostics)));
    SLANG_CHECK(diagnostics.getLength() == 0);
    SlangNVVMNamedIntrinsicOperandDesc operand = {
        {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, width, 1},
        SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
    SlangNVVMNamedIntrinsicOperandDesc operands[] = {operand, operand, operand};
    SlangNVVMNamedIntrinsicDesc desc = {name, strlen(name), operand.type, operands, operandCount};
    SLANG_CHECK(builder.supportsDeviceLibraryFunction(library.library, desc));
    for (const uint32_t wrongWidth : {16u, width == 32 ? 64u : 32u})
    {
        desc.resultType.bitWidth = operands[0].type.bitWidth = wrongWidth;
        SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
    }
    desc.resultType.bitWidth = operands[0].type.bitWidth = width;
    desc.operandCount = operandCount - 1;
    SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
    desc.operandCount = operandCount;
    desc.name = "__nv_unadmitted";
    desc.nameSize = strlen(desc.name);
    SLANG_CHECK(!builder.supportsDeviceLibraryFunction(library.library, desc));
    desc.name = name;
    desc.nameSize = strlen(desc.name);

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(UnownedStringSlice(moduleName), module.module)));
    SlangNVVMTypeHandle type = nullptr, functionType = nullptr;
    SlangNVVMValueHandle function = nullptr, parameter = nullptr;
    SlangNVVMBlockHandle block = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, width, type)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionType(module.module, type, &type, 1, functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("apply"),
        function)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, parameter)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(module.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));
    const SlangNVVMValueHandle arguments[] = {parameter, parameter, parameter};
    SlangNVVMValueHandle result = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitDeviceLibraryFunction(
        library.library,
        module.module,
        desc,
        arguments,
        operandCount,
        result)));
    SLANG_CHECK_ABORT(result != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(module.module, result)));
    const char* typeName = width == 32 ? "float" : "double";
    StringBuilder call, declaration;
    call << "call " << typeName << " @" << name << "(" << typeName;
    declaration << "declare " << typeName << " @" << name << "(";
    for (uint32_t i = 0; i < operandCount; ++i)
        declaration << (i ? ", " : "") << typeName;
    declaration << ")";
    for (const auto format :
         {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
          SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
    {
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assembly)));
        const String text = _getBlobText(assembly);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), call.getUnownedSlice()) == 1);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), declaration.getUnownedSlice()) == 1);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsCoreMathNamedCalls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto& testCase : kNVVMCoreMathTestCases)
        for (uint32_t width : {32u, 64u})
            _checkNVVMDeviceLibraryTypedCall(
                builder,
                width == 32 ? testCase.floatName : testCase.doubleName,
                width,
                testCase.name,
                testCase.operandCount);
}


SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryBuildsDirectedRoundingCalls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    struct Case
    {
        const char* name;
        uint32_t width;
    };
    const Case cases[] = {
        {"__nv_ceilf", 32},
        {"__nv_ceil", 64},
        {"__nv_floorf", 32},
        {"__nv_floor", 64},
        {"__nv_truncf", 32},
        {"__nv_trunc", 64},
    };
    for (const auto& testCase : cases)
        _checkNVVMDeviceLibraryTypedCall(
            builder,
            testCase.name,
            testCase.width,
            "named-directed-rounding");
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryBuildsRsqrtCalls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_rsqrtf", 32, "named-rsqrt-float");
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_rsqrt", 64, "named-rsqrt-double");
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryBuildsExpCalls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_expf", 32, "named-exp-float");
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_exp", 64, "named-exp-double");
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryBuildsExp2Calls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_exp2f", 32, "named-exp2-float");
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_exp2", 64, "named-exp2-double");
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryBuildsLogCalls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_logf", 32, "named-log-float");
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_log", 64, "named-log-double");
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryBuildsLog2Calls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_log2f", 32, "named-log2-float");
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_log2", 64, "named-log2-double");
}

SLANG_UNIT_TEST(nvvmIRBuilderDeviceLibraryBuildsLog10Calls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_log10f", 32, "named-log10-float");
    _checkNVVMDeviceLibraryTypedCall(builder, "__nv_log10", 64, "named-log10-double");
}

SLANG_UNIT_TEST(nvvmIRBuilderFracRetainsFloorAndSubtract)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto type : {NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
    {
        const char* name = type.bitWidth == 32 ? "__nv_floorf" : "__nv_floor";
        auto bytes = _makeNVVMDeviceLibraryFixture(builder, type.bitWidth, true, name);
        ScopedNVVMTestDeviceLibrary library{&builder};
        String diagnostics;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.loadDeviceLibrary(bytes, library.library, diagnostics)));
        const SlangNVVMNamedIntrinsicOperandDesc operand = {
            type,
            SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
        const SlangNVVMNamedIntrinsicDesc floorOperation = {name, strlen(name), type, &operand, 1};
        SLANG_CHECK_ABORT(builder.supportsDeviceLibraryFunction(library.library, floorOperation));
        const SlangNVVMValueTypeDesc operandTypes[] = {type, type};
        const SlangNVVMValueOperationDesc operation =
            {SLANG_NVVM_VALUE_OP_SUBTRACT, type, operandTypes, 2};
        SLANG_CHECK_ABORT(builder.supportsValueOperation(operation));
        ScopedNVVMBuilderModule module;
        module.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("retained-frac"), module.module)));
        SlangNVVMTypeHandle valueType = nullptr, functionType = nullptr;
        SlangNVVMValueHandle function = nullptr, parameter = nullptr;
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFloatingPointType(module.module, type.bitWidth, valueType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(module.module, valueType, &valueType, 1, functionType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            module.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("fraction"),
            function)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, parameter)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(module.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));
        // Keep a runtime parameter so LLVM cannot fold away the composition being checked.
        SlangNVVMValueHandle floorResult = nullptr, result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitDeviceLibraryFunction(
            library.library,
            module.module,
            floorOperation,
            &parameter,
            1,
            floorResult)));
        const SlangNVVMValueHandle operands[] = {parameter, floorResult};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitValueOperation(module.module, operation, operands, 2, result)));
        SLANG_CHECK_ABORT(result != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(module.module, result)));
        for (const auto format :
             {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
              SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
        {
            ComPtr<ISlangBlob> assembly;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.serializeModule(module.module, format, assembly)));
            const String text = _getBlobText(assembly);
            const char* call = type.bitWidth == 32 ? "call float @__nv_floorf(float"
                                                   : "call double @__nv_floor(double";
            const char* subtract = type.bitWidth == 32 ? "fsub float" : "fsub double";
            SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), UnownedStringSlice(call)) == 1);
            SLANG_CHECK(
                _countOccurrences(text.getUnownedSlice(), UnownedStringSlice(subtract)) == 1);
            // _declareFunction names the argument slangParameter0 for both LLVM dialects.
            // The first unnamed instruction is therefore %0, the floor result. Check its
            // input and the subtraction together to prove x - floor(x) in both serializers.
            SLANG_CHECK(text.contains(
                type.bitWidth == 32 ? "%0 = call float @__nv_floorf(float %slangParameter0)"
                                    : "%0 = call double @__nv_floor(double %slangParameter0)"));
            SLANG_CHECK(text.contains(
                type.bitWidth == 32 ? "fsub float %slangParameter0, %0"
                                    : "fsub double %slangParameter0, %0"));
            SLANG_CHECK(text.contains(
                type.bitWidth == 32 ? "declare float @__nv_floorf(float)"
                                    : "declare double @__nv_floor(double)"));
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderNamedHalfMathPreservesSignaturesAndCalls)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    const auto api = builder.getValueOperationsAPI();
    for (const char* name : {"llvm.ceil", "llvm.floor", "llvm.trunc", "llvm.fma"})
    {
        const size_t count = String(name) == "llvm.fma" ? 3 : 1;
        String control[2];
        for (bool injectFailures : {false, true})
        {
            ScopedNVVMBuilderModule scope;
            scope.builder = &builder;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createModule(toSlice("half-math"), scope.module)));
            SlangNVVMTypeHandle voidType = nullptr, halfType = nullptr, floatType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 16, halfType)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 32, floatType)));
            SlangNVVMTypeHandle parameterTypes[] = {halfType, halfType, halfType, floatType};
            SlangNVVMTypeHandle functionType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionType(scope.module, voidType, parameterTypes, 4, functionType)));
            SlangNVVMValueHandle function = nullptr, parameters[4] = {};
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                scope.module,
                functionType,
                SLANG_NVVM_LINKAGE_EXTERNAL,
                SLANG_NVVM_FUNCTION_FLAG_NONE,
                toSlice("host"),
                function)));
            for (Index i = 0; i < 4; ++i)
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(scope.module, function, i, parameters[i])));
            SlangNVVMBlockHandle entry = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.createBlock(scope.module, function, toSlice("entry"), entry)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entry)));
            SlangNVVMNamedIntrinsicOperandDesc operands[4];
            for (auto& operand : operands)
                operand = {NVVMSemantics::kFloat16, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
            SlangNVVMNamedIntrinsicDesc desc =
                {name, strlen(name), NVVMSemantics::kFloat16, operands, count};
            SLANG_CHECK(builder.supportsNamedIntrinsic(desc));
            if (injectFailures)
            {
                // Descriptor rejection and live-block physical rejection must not create a
                // declaration or call. The final serialization is compared with the control.
                auto reject = [&](const SlangNVVMNamedIntrinsicDesc& invalid)
                {
                    SLANG_CHECK(!builder.supportsNamedIntrinsic(invalid));
                    SlangNVVMValueHandle result =
                        reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                    SLANG_CHECK(
                        api->emitNamedIntrinsic(
                            scope.module,
                            &invalid,
                            parameters,
                            invalid.operandCount,
                            &result) == SLANG_E_NOT_AVAILABLE);
                    SLANG_CHECK(result == nullptr);
                };
                auto invalid = desc;
                invalid.operandCount = 0;
                reject(invalid);
                invalid.operandCount = count + 1;
                reject(invalid);
                const SlangNVVMValueTypeDesc invalidTypes[] = {
                    NVVMSemantics::kFloat32,
                    NVVMSemantics::kFloat64,
                    {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 16, 2},
                    {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 1},
                    NVVMSemantics::kUnsignedI32};
                for (auto type : invalidTypes)
                {
                    invalid = desc;
                    invalid.resultType = type;
                    reject(invalid);
                    for (size_t i = 0; i < count; ++i)
                        operands[i].type = type;
                    reject(invalid);
                    invalid.resultType = NVVMSemantics::kFloat16;
                    reject(invalid);
                    for (auto& operand : operands)
                        operand.type = NVVMSemantics::kFloat16;
                }
                for (size_t i = 0; i < count; ++i)
                {
                    operands[i].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT;
                    reject(desc);
                    operands[i].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE;
                    SlangNVVMValueHandle values[] = {parameters[0], parameters[1], parameters[2]};
                    values[i] = parameters[3];
                    SlangNVVMValueHandle result =
                        reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                    SLANG_CHECK(
                        api->emitNamedIntrinsic(scope.module, &desc, values, count, &result) ==
                        SLANG_E_INVALID_ARG);
                    SLANG_CHECK(result == nullptr);
                }
                String suffixed = String(name) + ".f16";
                invalid = desc;
                invalid.name = suffixed.getBuffer();
                invalid.nameSize = suffixed.getLength();
                reject(invalid);
            }
            SlangNVVMValueHandle result = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.emitNamedIntrinsic(scope.module, desc, parameters, count, result)));
            SLANG_CHECK(result != nullptr);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
            Index formatIndex = 0;
            for (auto format :
                 {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                  SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
            {
                ComPtr<ISlangBlob> assembly;
                String diagnostics;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.serializeModule(scope.module, format, assembly, diagnostics)));
                SLANG_CHECK(diagnostics.getLength() == 0);
                const String text = _getBlobText(assembly);
                StringBuilder call, declaration;
                call << "call half @" << name << ".f16(half %slangParameter0";
                if (count == 3)
                    call << ", half %slangParameter1, half %slangParameter2";
                call << ")";
                declaration << "declare half @" << name << ".f16(half";
                if (count == 3)
                    declaration << ", half, half";
                declaration << ")";
                if (String(name) == "llvm.trunc")
                {
                    SLANG_CHECK(text.contains("bitcast half %slangParameter0 to i16"));
                    SLANG_CHECK(text.contains("call i16 asm \"cvt.rzi.f16.f16 $0, $1;\", "
                                              "\"=h,h\"(i16 %"));
                    SLANG_CHECK(text.contains("bitcast i16") && text.contains("to half"));
                    SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("bitcast")) == 2);
                    SLANG_CHECK(!text.contains("llvm.trunc.f16"));
                    SLANG_CHECK(!text.contains("asm sideeffect"));
                }
                else
                {
                    SLANG_CHECK(text.contains(call.getBuffer()));
                    SLANG_CHECK(text.contains(declaration.getBuffer()));
                }
                SLANG_CHECK(!text.contains("fpext") && !text.contains("fptrunc"));
                if (injectFailures)
                {
                    SLANG_CHECK(text == control[formatIndex]);
                }
                else
                {
                    control[formatIndex] = text;
                }
                ++formatIndex;
            }
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderNamedSqrtSignaturesArePure)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(toSlice("sqrt-query"), scope.module)));
    ComPtr<ISlangBlob> before;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, before)));
    const auto api = builder.getValueOperationsAPI();
    for (const auto type : {NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
    {
        SlangNVVMNamedIntrinsicOperandDesc operands[] = {
            {type, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE},
            {type, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE}};
        SlangNVVMNamedIntrinsicDesc desc =
            {"llvm.sqrt", sizeof("llvm.sqrt") - 1, type, operands, 1};
        SLANG_CHECK(builder.supportsNamedIntrinsic(desc));
        for (size_t count : {size_t(0), size_t(2)})
        {
            desc.operandCount = count;
            SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
        }
        desc.operandCount = 1;
        operands[0].type.bitWidth = type.bitWidth == 32 ? 64 : 32;
        SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
        operands[0].type = type;
        for (auto kind :
             {SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT,
              SlangNVVMNamedIntrinsicOperandKind(99)})
        {
            operands[0].kind = kind;
            SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
        }
        operands[0].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE;
        const SlangNVVMValueTypeDesc invalidTypes[] = {
            NVVMSemantics::kFloat16,
            NVVMSemantics::kSignedI32,
            NVVMSemantics::kUnsignedI32,
            NVVMSemantics::kBool,
            NVVMSemantics::kVoid,
            {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 24, 1},
            {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, type.bitWidth, 0},
            {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, type.bitWidth, 2},
            {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 1},
            {SlangNVVMValueTypeKind(99), type.bitWidth, 1}};
        for (const auto invalid : invalidTypes)
        {
            desc.resultType = invalid;
            SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
            operands[0].type = invalid;
            SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
            desc.resultType = type;
            SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
            operands[0].type = type;
        }
        for (const char* name :
             {"llvm.sqrt.f32", "llvm.sqrt.f64", "llvm.sqrt()", "llvm.sin", "llvm.missing"})
        {
            desc.name = name;
            desc.nameSize = strlen(name);
            SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
        }
        const char embeddedNull[] = "llvm.sqrt\0suffix";
        desc.name = embeddedNull;
        desc.nameSize = sizeof(embeddedNull) - 1;
        SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
        desc.name = "llvm.sqrt";
        desc.nameSize = 0;
        SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
        desc.nameSize = sizeof("llvm.sqrt") - 1;
        desc.name = nullptr;
        SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
        desc.name = "llvm.sqrt";
        desc.operands = nullptr;
        uint32_t supported = 77;
        SLANG_CHECK(api->isNamedIntrinsicSupported(&desc, &supported) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(supported == 0);
    }
    uint32_t supported = 77;
    SLANG_CHECK(api->isNamedIntrinsicSupported(nullptr, &supported) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(supported == 0);
    ComPtr<ISlangBlob> after;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, after)));
    SLANG_CHECK(_getBlobText(before) == _getBlobText(after));
}

SLANG_UNIT_TEST(nvvmIRBuilderNamedSqrtPreservesTypedCallsAndOwnership)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    const auto api = builder.getValueOperationsAPI();
    for (const auto type : {NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
    {
        String control[2];
        for (bool injectFailures : {false, true})
        {
            ScopedNVVMBuilderModule scope, foreign;
            scope.builder = foreign.builder = &builder;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createModule(toSlice("sqrt-ownership"), scope.module)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createModule(toSlice("foreign"), foreign.module)));
            SlangNVVMTypeHandle voidType = nullptr, types[3] = {}, foreignType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFloatingPointType(scope.module, type.bitWidth, types[0])));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(
                scope.module,
                type.bitWidth == 32 ? 64 : 32,
                types[1])));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 1, types[2])));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFloatingPointType(foreign.module, type.bitWidth, foreignType)));
            SlangNVVMValueHandle foreignValue = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointConstant(
                foreign.module,
                foreignType,
                type.bitWidth,
                0,
                foreignValue)));
            SlangNVVMTypeHandle functionType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionType(scope.module, voidType, types, 3, functionType)));
            SlangNVVMValueHandle function = nullptr, otherFunction = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                scope.module,
                functionType,
                SLANG_NVVM_LINKAGE_EXTERNAL,
                SLANG_NVVM_FUNCTION_FLAG_NONE,
                toSlice("host"),
                function)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                scope.module,
                functionType,
                SLANG_NVVM_LINKAGE_EXTERNAL,
                SLANG_NVVM_FUNCTION_FLAG_NONE,
                toSlice("other"),
                otherFunction)));
            SlangNVVMValueHandle parameters[3] = {}, otherParameter = nullptr;
            for (Index i = 0; i < 3; ++i)
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(scope.module, function, i, parameters[i])));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionParameter(scope.module, otherFunction, 0, otherParameter)));
            const SlangNVVMNamedIntrinsicOperandDesc operands[] = {
                {type, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE},
                {type, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE}};
            SlangNVVMNamedIntrinsicDesc desc =
                {"llvm.sqrt", sizeof("llvm.sqrt") - 1, type, operands, 1};
            if (injectFailures)
            {
                SlangNVVMValueHandle rejected =
                    reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                SLANG_CHECK(
                    api->emitNamedIntrinsic(scope.module, &desc, parameters, 1, &rejected) ==
                    SLANG_E_INVALID_ARG);
                SLANG_CHECK(rejected == nullptr);
            }
            SlangNVVMBlockHandle entry = nullptr, left = nullptr, right = nullptr, merge = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.createBlock(scope.module, function, toSlice("entry"), entry)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.createBlock(scope.module, function, toSlice("left"), left)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.createBlock(scope.module, function, toSlice("right"), right)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.createBlock(scope.module, function, toSlice("merge"), merge)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entry)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.emitConditionalBranch(scope.module, parameters[2], left, right)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, left)));
            SlangNVVMValueHandle leftValue = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.emitNamedIntrinsic(scope.module, desc, parameters, 1, leftValue)));
            SLANG_CHECK_ABORT(leftValue != nullptr);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(scope.module, merge)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, right)));
            if (injectFailures)
            {
                for (auto invalid :
                     {foreignValue,
                      otherParameter,
                      leftValue,
                      parameters[1],
                      SlangNVVMValueHandle(nullptr)})
                {
                    SlangNVVMValueHandle rejected =
                        reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                    SLANG_CHECK(
                        api->emitNamedIntrinsic(scope.module, &desc, &invalid, 1, &rejected) ==
                        SLANG_E_INVALID_ARG);
                    SLANG_CHECK(rejected == nullptr);
                }
                SlangNVVMValueHandle rejected =
                    reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                SLANG_CHECK(
                    api->emitNamedIntrinsic(scope.module, &desc, nullptr, 1, &rejected) ==
                    SLANG_E_INVALID_ARG);
                SLANG_CHECK(rejected == nullptr);
                SLANG_CHECK(
                    api->emitNamedIntrinsic(scope.module, &desc, parameters, 0, &rejected) ==
                    SLANG_E_INVALID_ARG);
                SLANG_CHECK(rejected == nullptr);
                SLANG_CHECK(
                    api->emitNamedIntrinsic(scope.module, &desc, parameters, 1, nullptr) ==
                    SLANG_E_INVALID_ARG);
                for (size_t count : {size_t(0), size_t(2)})
                {
                    desc.operandCount = count;
                    SLANG_CHECK(
                        api->emitNamedIntrinsic(
                            scope.module,
                            &desc,
                            parameters,
                            count,
                            &rejected) == SLANG_E_NOT_AVAILABLE);
                    SLANG_CHECK(rejected == nullptr);
                }
                desc.operandCount = 1;
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(scope.module, merge)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, merge)));
            SlangNVVMValueHandle result = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.emitNamedIntrinsic(scope.module, desc, parameters, 1, result)));
            SLANG_CHECK_ABORT(result != nullptr);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
            if (injectFailures)
            {
                SlangNVVMValueHandle rejected =
                    reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                SLANG_CHECK(
                    api->emitNamedIntrinsic(scope.module, &desc, parameters, 1, &rejected) ==
                    SLANG_E_INVALID_ARG);
                SLANG_CHECK(rejected == nullptr);
            }
            Index formatIndex = 0;
            for (auto format :
                 {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                  SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
            {
                ComPtr<ISlangBlob> assembly;
                String diagnostics;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.serializeModule(scope.module, format, assembly, diagnostics)));
                SLANG_CHECK(diagnostics.getLength() == 0);
                const String text = _getBlobText(assembly);
                const char* call = type.bitWidth == 32 ? "call float @llvm.sqrt.f32(float"
                                                       : "call double @llvm.sqrt.f64(double";
                const char* declaration = type.bitWidth == 32
                                              ? "declare float @llvm.sqrt.f32(float)"
                                              : "declare double @llvm.sqrt.f64(double)";
                SLANG_CHECK(
                    _countOccurrences(text.getUnownedSlice(), UnownedStringSlice(call)) == 2);
                SLANG_CHECK(
                    _countOccurrences(text.getUnownedSlice(), UnownedStringSlice(declaration)) ==
                    1);
                SLANG_CHECK(!text.contains("__nv_"));
                if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
                    for (const char* attribute :
                         {"nofree", "nosync", "nounwind", "readnone", "speculatable", "willreturn"})
                        SLANG_CHECK(text.contains(attribute));
                if (injectFailures)
                {
                    SLANG_CHECK(text == control[formatIndex]);
                }
                else
                {
                    control[formatIndex] = text;
                }
                ++formatIndex;
            }
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderReservedSqrtOperationRejectsWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto type :
         {NVVMSemantics::kFloat16, NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
    {
        String control;
        for (bool injectFailures : {false, true})
        {
            ScopedNVVMBuilderModule scope;
            scope.builder = &builder;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createModule(toSlice("retired-sqrt"), scope.module)));
            SlangNVVMTypeHandle valueType = nullptr, functionType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFloatingPointType(scope.module, type.bitWidth, valueType)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionType(scope.module, valueType, &valueType, 1, functionType)));
            SlangNVVMValueHandle function = nullptr, value = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                scope.module,
                functionType,
                SLANG_NVVM_LINKAGE_EXTERNAL,
                SLANG_NVVM_FUNCTION_FLAG_NONE,
                toSlice("identity"),
                function)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, value)));
            SlangNVVMBlockHandle block = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.createBlock(scope.module, function, toSlice("entry"), block)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
            if (injectFailures)
            {
                // Keep the raw historical identity here: no public operation name remains.
                const SlangNVVMValueOperationDesc desc =
                    {SlangNVVMValueOperation(36), type, &type, 1};
                SLANG_CHECK(!builder.supportsValueOperation(desc));
                SlangNVVMValueHandle rejected =
                    reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                SLANG_CHECK(
                    builder.getValueOperationsAPI()
                        ->emitOperation(scope.module, &desc, &value, 1, &rejected) ==
                    SLANG_E_INVALID_ARG);
                SLANG_CHECK(rejected == nullptr);
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, value)));
            ComPtr<ISlangBlob> assembly;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                assembly)));
            const String text = _getBlobText(assembly);
            SLANG_CHECK(!text.contains("@llvm.sqrt"));
            if (injectFailures)
            {
                SLANG_CHECK(text == control);
            }
            else
            {
                control = text;
            }
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderReservedFracOperationRejectsWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto type :
         {NVVMSemantics::kFloat16, NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
    {
        String control;
        for (bool injectFailures : {false, true})
        {
            ScopedNVVMBuilderModule scope;
            scope.builder = &builder;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.createModule(toSlice("retired-frac"), scope.module)));
            SlangNVVMTypeHandle valueType = nullptr, functionType = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFloatingPointType(scope.module, type.bitWidth, valueType)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionType(scope.module, valueType, &valueType, 1, functionType)));
            SlangNVVMValueHandle function = nullptr, value = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                scope.module,
                functionType,
                SLANG_NVVM_LINKAGE_EXTERNAL,
                SLANG_NVVM_FUNCTION_FLAG_NONE,
                toSlice("identity"),
                function)));
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, value)));
            SlangNVVMBlockHandle block = nullptr;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.createBlock(scope.module, function, toSlice("entry"), block)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
            if (injectFailures)
            {
                // Keep the raw historical identity here: no public operation name remains.
                const SlangNVVMValueOperationDesc desc =
                    {SlangNVVMValueOperation(59), type, &type, 1};
                SLANG_CHECK(!builder.supportsValueOperation(desc));
                SlangNVVMValueHandle rejected =
                    reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                SLANG_CHECK(
                    builder.getValueOperationsAPI()
                        ->emitOperation(scope.module, &desc, &value, 1, &rejected) ==
                    SLANG_E_INVALID_ARG);
                SLANG_CHECK(rejected == nullptr);
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, value)));
            ComPtr<ISlangBlob> assembly;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                assembly)));
            const String text = _getBlobText(assembly);
            SLANG_CHECK(!text.contains("@__nv_floor"));
            if (injectFailures)
            {
                SLANG_CHECK(text == control);
            }
            else
            {
                control = text;
            }
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderReservedRsqrtOperationRejectsWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto type :
         {NVVMSemantics::kFloat16, NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
        for (uint32_t lanes : {1u, 2u})
        {
            auto testedType = type;
            testedType.laneCount = lanes;
            String control;
            for (bool injectFailures : {false, true})
            {
                ScopedNVVMBuilderModule scope;
                scope.builder = &builder;
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(builder.createModule(toSlice("retired-rsqrt"), scope.module)));
                SlangNVVMTypeHandle valueType = nullptr, functionType = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFloatingPointType(scope.module, type.bitWidth, valueType)));
                if (lanes == 2)
                {
                    SlangNVVMTypeHandle vectorType = nullptr;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        builder.getVectorType(scope.module, valueType, lanes, vectorType)));
                    valueType = vectorType;
                }
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionType(scope.module, valueType, &valueType, 1, functionType)));
                SlangNVVMValueHandle function = nullptr, value = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                    scope.module,
                    functionType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_FUNCTION_FLAG_NONE,
                    toSlice("identity"),
                    function)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(scope.module, function, 0, value)));
                SlangNVVMBlockHandle block = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.createBlock(scope.module, function, toSlice("entry"), block)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
                if (injectFailures)
                {
                    // Keep the raw historical identity here: no public operation name remains.
                    const SlangNVVMValueOperationDesc desc =
                        {SlangNVVMValueOperation(65), testedType, &testedType, 1};
                    SLANG_CHECK(!builder.supportsValueOperation(desc));
                    SlangNVVMValueHandle rejected =
                        reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                    SLANG_CHECK(
                        builder.getValueOperationsAPI()
                            ->emitOperation(scope.module, &desc, &value, 1, &rejected) ==
                        SLANG_E_INVALID_ARG);
                    SLANG_CHECK(rejected == nullptr);
                }
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, value)));
                ComPtr<ISlangBlob> assembly;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                    scope.module,
                    SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                    assembly)));
                const String text = _getBlobText(assembly);
                SLANG_CHECK(!text.contains("@__nv_rsqrt"));
                if (injectFailures)
                {
                    SLANG_CHECK(text == control);
                }
                else
                {
                    control = text;
                }
            }
        }
}

SLANG_UNIT_TEST(nvvmIRBuilderReservedExpOperationRejectsWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto type :
         {NVVMSemantics::kFloat16, NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
        for (uint32_t lanes : {1u, 2u})
        {
            auto testedType = type;
            testedType.laneCount = lanes;
            String control;
            for (bool injectFailures : {false, true})
            {
                ScopedNVVMBuilderModule scope;
                scope.builder = &builder;
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(builder.createModule(toSlice("retired-exp"), scope.module)));
                SlangNVVMTypeHandle valueType = nullptr, functionType = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFloatingPointType(scope.module, type.bitWidth, valueType)));
                if (lanes == 2)
                {
                    SlangNVVMTypeHandle vectorType = nullptr;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        builder.getVectorType(scope.module, valueType, lanes, vectorType)));
                    valueType = vectorType;
                }
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionType(scope.module, valueType, &valueType, 1, functionType)));
                SlangNVVMValueHandle function = nullptr, value = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                    scope.module,
                    functionType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_FUNCTION_FLAG_NONE,
                    toSlice("identity"),
                    function)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(scope.module, function, 0, value)));
                SlangNVVMBlockHandle block = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.createBlock(scope.module, function, toSlice("entry"), block)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
                if (injectFailures)
                {
                    // Keep the raw historical identity here: no public operation name remains.
                    const SlangNVVMValueOperationDesc desc =
                        {SlangNVVMValueOperation(55), testedType, &testedType, 1};
                    SLANG_CHECK(!builder.supportsValueOperation(desc));
                    SlangNVVMValueHandle rejected =
                        reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                    SLANG_CHECK(
                        builder.getValueOperationsAPI()
                            ->emitOperation(scope.module, &desc, &value, 1, &rejected) ==
                        SLANG_E_INVALID_ARG);
                    SLANG_CHECK(rejected == nullptr);
                }
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, value)));
                ComPtr<ISlangBlob> assembly;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                    scope.module,
                    SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                    assembly)));
                const String text = _getBlobText(assembly);
                SLANG_CHECK(!text.contains("@__nv_exp"));
                if (injectFailures)
                {
                    SLANG_CHECK(text == control);
                }
                else
                {
                    control = text;
                }
            }
        }
}

SLANG_UNIT_TEST(nvvmIRBuilderReservedExp2OperationRejectsWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto type :
         {NVVMSemantics::kFloat16, NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
        for (uint32_t lanes : {1u, 2u})
        {
            auto testedType = type;
            testedType.laneCount = lanes;
            String control;
            for (bool injectFailures : {false, true})
            {
                ScopedNVVMBuilderModule scope;
                scope.builder = &builder;
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(builder.createModule(toSlice("retired-exp2"), scope.module)));
                SlangNVVMTypeHandle valueType = nullptr, functionType = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFloatingPointType(scope.module, type.bitWidth, valueType)));
                if (lanes == 2)
                {
                    SlangNVVMTypeHandle vectorType = nullptr;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        builder.getVectorType(scope.module, valueType, lanes, vectorType)));
                    valueType = vectorType;
                }
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionType(scope.module, valueType, &valueType, 1, functionType)));
                SlangNVVMValueHandle function = nullptr, value = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                    scope.module,
                    functionType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_FUNCTION_FLAG_NONE,
                    toSlice("identity"),
                    function)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(scope.module, function, 0, value)));
                SlangNVVMBlockHandle block = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.createBlock(scope.module, function, toSlice("entry"), block)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
                if (injectFailures)
                {
                    // Keep the raw historical identity here: no public operation name remains.
                    const SlangNVVMValueOperationDesc desc =
                        {SlangNVVMValueOperation(56), testedType, &testedType, 1};
                    SLANG_CHECK(!builder.supportsValueOperation(desc));
                    SlangNVVMValueHandle rejected =
                        reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                    SLANG_CHECK(
                        builder.getValueOperationsAPI()
                            ->emitOperation(scope.module, &desc, &value, 1, &rejected) ==
                        SLANG_E_INVALID_ARG);
                    SLANG_CHECK(rejected == nullptr);
                }
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, value)));
                ComPtr<ISlangBlob> assembly;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                    scope.module,
                    SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                    assembly)));
                const String text = _getBlobText(assembly);
                SLANG_CHECK(!text.contains("@__nv_exp2"));
                if (injectFailures)
                {
                    SLANG_CHECK(text == control);
                }
                else
                {
                    control = text;
                }
            }
        }
}

// Reject a retired math ID without changing the module or returning an output value.
static void _checkNVVMReservedMathOperation(
    UnitTestContext* unitTestContext,
    SlangNVVMValueOperation operation,
    uint32_t operandCount = 1,
    bool booleanResult = false,
    bool signedResult = false)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto type :
         {NVVMSemantics::kFloat16, NVVMSemantics::kFloat32, NVVMSemantics::kFloat64})
        for (uint32_t lanes : {1u, 2u})
        {
            auto testedType = type;
            testedType.laneCount = lanes;
            String control;
            for (bool injectFailures : {false, true})
            {
                ScopedNVVMBuilderModule scope;
                scope.builder = &builder;
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(builder.createModule(toSlice("retired-math"), scope.module)));
                SlangNVVMTypeHandle valueType = nullptr, functionType = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFloatingPointType(scope.module, type.bitWidth, valueType)));
                if (lanes == 2)
                {
                    SlangNVVMTypeHandle vectorType = nullptr;
                    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                        builder.getVectorType(scope.module, valueType, lanes, vectorType)));
                    valueType = vectorType;
                }
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionType(scope.module, valueType, &valueType, 1, functionType)));
                SlangNVVMValueHandle function = nullptr, value = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
                    scope.module,
                    functionType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_FUNCTION_FLAG_NONE,
                    toSlice("identity"),
                    function)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.getFunctionParameter(scope.module, function, 0, value)));
                SlangNVVMBlockHandle block = nullptr;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                    builder.createBlock(scope.module, function, toSlice("entry"), block)));
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
                if (injectFailures)
                {
                    // Keep the raw historical identity here: no public operation name remains.
                    const SlangNVVMValueTypeDesc operandTypes[] = {
                        testedType,
                        testedType,
                        testedType};
                    const SlangNVVMValueHandle operands[] = {value, value, value};
                    auto resultType = signedResult    ? NVVMSemantics::kSignedI32
                                      : booleanResult ? NVVMSemantics::kBool
                                                      : testedType;
                    resultType.laneCount = lanes;
                    const SlangNVVMValueOperationDesc desc =
                        {operation, resultType, operandTypes, operandCount};
                    SLANG_CHECK(!builder.supportsValueOperation(desc));
                    SlangNVVMValueHandle rejected =
                        reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                    SLANG_CHECK(
                        builder.getValueOperationsAPI()->emitOperation(
                            scope.module,
                            &desc,
                            operands,
                            operandCount,
                            &rejected) == SLANG_E_INVALID_ARG);
                    SLANG_CHECK(rejected == nullptr);
                }
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, value)));
                ComPtr<ISlangBlob> assembly;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
                    scope.module,
                    SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
                    assembly)));
                const String text = _getBlobText(assembly);
                SLANG_CHECK(!text.contains("@__nv_log"));
                if (injectFailures)
                {
                    SLANG_CHECK(text == control);
                }
                else
                {
                    control = text;
                }
            }
        }
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsRetiredPointerMathOperations)
{
    for (uint32_t operation : {69u, 70u, 77u, 78u})
        _checkNVVMReservedMathOperation(unitTestContext, operation, 1, false, operation == 70);
}

SLANG_UNIT_TEST(nvvmIRBuilderReservedLogOperationRejectsWithoutMutation)
{
    _checkNVVMReservedMathOperation(unitTestContext, SlangNVVMValueOperation(60));
}

SLANG_UNIT_TEST(nvvmIRBuilderReservedLog2OperationRejectsWithoutMutation)
{
    _checkNVVMReservedMathOperation(unitTestContext, SlangNVVMValueOperation(61));
}

SLANG_UNIT_TEST(nvvmIRBuilderReservedLog10OperationRejectsWithoutMutation)
{
    _checkNVVMReservedMathOperation(unitTestContext, SlangNVVMValueOperation(62));
}

SLANG_UNIT_TEST(nvvmIRBuilderRetiredCoreMathRejectsWithoutMutation)
{
    for (const auto& testCase : kNVVMCoreMathTestCases)
        if (testCase.operation != SLANG_NVVM_VALUE_OP_FMOD)
            _checkNVVMReservedMathOperation(
                unitTestContext,
                testCase.operation,
                testCase.operandCount);
}

SLANG_UNIT_TEST(nvvmIRBuilderRetiredNaNRejectsWithoutMutation)
{
    _checkNVVMReservedMathOperation(unitTestContext, SlangNVVMValueOperation(67), 1, true);
}

SLANG_UNIT_TEST(nvvmIRBuilderCoreValuesUseNamedFunctions)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto& testCase : kNVVMCoreValueTestCases)
    {
        _checkNVVMDeviceLibraryTypedCall(
            builder,
            testCase.floatName,
            32,
            "core-value-float",
            testCase.operandCount);
        _checkNVVMDeviceLibraryTypedCall(
            builder,
            testCase.doubleName,
            64,
            "core-value-double",
            testCase.operandCount);
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderCoreValuesRejectRetiredOperations)
{
    _checkNVVMReservedMathOperation(unitTestContext, 49);
    _checkNVVMReservedMathOperation(unitTestContext, 68, 1, false, true);
}

SLANG_UNIT_TEST(nvvmIRBuilderCoreWavesUseNamedSignatures)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    for (const auto& testCase : kNVVMWaveNamedTestCases)
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("named-wave"), scope.module)));
        SlangNVVMTypeHandle integerType = nullptr, boolType = nullptr, floatType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 1, boolType)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 32, floatType)));
        SlangNVVMNamedIntrinsicOperandDesc operands[4] = {};
        SlangNVVMTypeHandle types[4] = {};
        for (uint32_t i = 0; i < testCase.operandCount; ++i)
        {
            operands[i] = {testCase.operands[i], SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE};
            types[i] = testCase.operands[i].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ? floatType
                       : testCase.operands[i].kind == SLANG_NVVM_VALUE_TYPE_BOOL         ? boolType
                                                                                 : integerType;
        }
        const SlangNVVMTypeHandle resultType =
            testCase.result.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ? floatType
            : testCase.result.kind == SLANG_NVVM_VALUE_TYPE_BOOL         ? boolType
                                                                         : integerType;
        SlangNVVMNamedIntrinsicDesc desc = {
            testCase.name,
            strlen(testCase.name),
            testCase.result,
            operands,
            testCase.operandCount};
        SLANG_CHECK_ABORT(builder.supportsNamedIntrinsic(desc));
        SlangNVVMTypeHandle functionType = nullptr;
        SlangNVVMValueHandle function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            resultType,
            types,
            testCase.operandCount,
            functionType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("apply"),
            function)));
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        SlangNVVMValueHandle values[4] = {};
        for (uint32_t i = 0; i < testCase.operandCount; ++i)
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionParameter(scope.module, function, i, values[i])));
        auto invalid = desc;
        invalid.resultType = NVVMSemantics::kFloat64;
        SLANG_CHECK(!builder.supportsNamedIntrinsic(invalid));
        SlangNVVMValueHandle rejected = reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
        SLANG_CHECK(SLANG_FAILED(builder.emitNamedIntrinsic(
            scope.module,
            invalid,
            values,
            testCase.operandCount,
            rejected)));
        SLANG_CHECK(rejected == nullptr);
        invalid = desc;
        invalid.operandCount = testCase.operandCount ? testCase.operandCount - 1 : 1;
        SLANG_CHECK(!builder.supportsNamedIntrinsic(invalid));
        if (testCase.operandCount)
        {
            auto saved = operands[0];
            operands[0].type = NVVMSemantics::kFloat64;
            SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
            operands[0] = saved;
        }
        if (testCase.result.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT)
        {
            operands[1].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT;
            SLANG_CHECK(!builder.supportsNamedIntrinsic(desc));
            operands[1].kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE;
        }
        SlangNVVMValueHandle result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitNamedIntrinsic(scope.module, desc, values, testCase.operandCount, result)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, result)));
        for (const auto format :
             {SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
              SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY})
        {
            ComPtr<ISlangBlob> assembly;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(builder.serializeModule(scope.module, format, assembly)));
            StringBuilder symbol;
            symbol << "@" << testCase.name << "(";
            const String text = _getBlobText(assembly);
            SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), symbol.getUnownedSlice()) == 2);
            if (testCase.operandCount)
                SLANG_CHECK(text.contains("convergent inaccessiblememonly nounwind"));
            SLANG_CHECK(!text.contains("double"));
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderCoreWavesRejectRetiredOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    String control;
    for (bool injectFailures : {false, true})
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("retired-wave"), scope.module)));
        SlangNVVMTypeHandle integerType = nullptr, boolType = nullptr, functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 1, boolType)));
        const SlangNVVMTypeHandle types[] = {integerType, integerType, boolType};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, integerType, types, 3, functionType)));
        SlangNVVMValueHandle function = nullptr, values[3] = {};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("identity"),
            function)));
        for (uint32_t i = 0; i < 3; ++i)
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionParameter(scope.module, function, i, values[i])));
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        if (injectFailures)
            for (uint32_t operation : {16u, 19u, 20u, 21u, 22u, 23u})
            {
                const bool vote = operation == 21 || operation == 22;
                const SlangNVVMValueTypeDesc operands[] = {
                    NVVMSemantics::kUnsignedI32,
                    vote ? NVVMSemantics::kBool : NVVMSemantics::kUnsignedI32};
                const SlangNVVMValueHandle arguments[] = {values[0], values[vote ? 2 : 1]};
                const SlangNVVMValueOperationDesc desc = {
                    operation,
                    operation == 16 || operation == 19 ? NVVMSemantics::kUnsignedI32
                                                       : NVVMSemantics::kBool,
                    operands,
                    operation == 16   ? 0u
                    : operation == 20 ? 1u
                                      : 2u};
                SLANG_CHECK(!builder.supportsValueOperation(desc));
                SlangNVVMValueHandle rejected =
                    reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                SLANG_CHECK(
                    builder.getValueOperationsAPI()->emitOperation(
                        scope.module,
                        &desc,
                        arguments,
                        desc.operandCount,
                        &rejected) == SLANG_E_INVALID_ARG);
                SLANG_CHECK(rejected == nullptr);
            }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, values[0])));
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
            assembly)));
        if (injectFailures)
        {
            SLANG_CHECK(_getBlobText(assembly) == control);
        }
        else
        {
            control = _getBlobText(assembly);
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderMaskedWavesRejectRetiredOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    String control;
    for (bool injectFailures : {false, true})
    {
        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("retired-wave"), scope.module)));
        SlangNVVMTypeHandle integerType = nullptr, boolType = nullptr, functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 1, boolType)));
        const SlangNVVMTypeHandle types[] = {integerType, integerType, boolType};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, integerType, types, 3, functionType)));
        SlangNVVMValueHandle function = nullptr, values[3] = {};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice("identity"),
            function)));
        for (uint32_t i = 0; i < 3; ++i)
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionParameter(scope.module, function, i, values[i])));
        SlangNVVMBlockHandle block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        if (injectFailures)
            for (uint32_t operation : {15u, 17u, 43u, 44u, 45u, 46u, 47u, 48u})
            {
                SlangNVVMValueTypeDesc types[] = {
                    NVVMSemantics::kUnsignedI32,
                    NVVMSemantics::kUnsignedI32,
                    NVVMSemantics::kSignedI32};
                const uint32_t count = operation == 15                        ? 0
                                       : operation == 17                      ? 3
                                       : (operation == 43 || operation == 44) ? 2
                                                                              : 1;
                const SlangNVVMValueOperationDesc desc =
                    {operation, NVVMSemantics::kUnsignedI32, types, count};
                SLANG_CHECK(!builder.supportsValueOperation(desc));
                SlangNVVMValueHandle arguments[] = {values[0], values[0], values[0]};
                SlangNVVMValueHandle rejected =
                    reinterpret_cast<SlangNVVMValueHandle>(uintptr_t(1));
                SLANG_CHECK(
                    builder.getValueOperationsAPI()
                        ->emitOperation(scope.module, &desc, arguments, count, &rejected) ==
                    SLANG_E_INVALID_ARG);
                SLANG_CHECK(rejected == nullptr);
            }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitValueReturn(scope.module, values[0])));
        ComPtr<ISlangBlob> assembly;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
            assembly)));
        if (injectFailures)
        {
            SLANG_CHECK(_getBlobText(assembly) == control);
        }
        else
        {
            control = _getBlobText(assembly);
        }
    }
}
