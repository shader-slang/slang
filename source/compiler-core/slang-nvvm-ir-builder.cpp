#include "slang-nvvm-ir-builder.h"

#include "core/slang-blob.h"
#include "slang-downstream-compiler-util.h"
#include "slang-nvvm-semantic-catalog.h"

namespace Slang
{

static bool _isVerificationStatus(SlangNVVMVerificationStatus status)
{
    return status == SLANG_NVVM_VERIFICATION_VALID || status == SLANG_NVVM_VERIFICATION_INVALID;
}

template<typename T>
static SlangResult _validateHandleResult(SlangNVVMResult result, T& handle)
{
    if (result < 0)
    {
        handle = nullptr;
        return result;
    }
    return handle ? result : SLANG_FAIL;
}

static void _addFeature(SlangNVVMBuilderFeatureSet& features, SlangNVVMBuilderFeature feature)
{
    features.words[feature / 64u] |= uint64_t(1) << (feature % 64u);
}

static bool _hasFeature(const SlangNVVMBuilderFeatureSet& features, SlangNVVMBuilderFeature feature)
{
    if (feature >= SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT * 64u)
        return false;
    return (features.words[feature / 64u] & (uint64_t(1) << (feature % 64u))) != 0;
}

static bool _hasRequiredFoundation(const SlangNVVMBuilderFoundationAPI& api)
{
    return api.createModule && api.destroyModule && api.serializeModuleWithDiagnostics &&
           api.serializeNVVMIR20AssemblyWithDiagnostics;
}

static bool _hasRequiredConstruction(const SlangNVVMBuilderConstructionAPI& api)
{
    return api.getVoidType && api.getIntegerType && api.getFloatingPointType &&
           api.getPointerType && api.getFunctionType && api.getArrayType && api.getVectorType &&
           api.getRawRWStructuredBufferI32Type && api.declareFunction && api.getFunctionParameter &&
           api.createBlock && api.setInsertBlock && api.emitLoad && api.emitStore &&
           api.emitBranch && api.emitConditionalBranch && api.getIntegerConstant &&
           api.getFloatingPointConstant && api.emitPhi && api.addPhiIncoming && api.emitCall &&
           api.emitValueReturn && api.emitReturnVoid && api.emitPointerOffset &&
           api.emitArrayElementPointer && api.emitVectorElementExtract &&
           api.emitRawRWStructuredBufferI32ElementPointer && api.emitRelaxedGlobalI32AtomicAdd &&
           api.declareGlobalStorage && api.markFunctionAsKernel;
}

static bool _hasRequiredValueOperations(const SlangNVVMBuilderValueOperationsAPI& api)
{
    return api.isOperationSupported && api.emitOperation;
}

/* static */ SlangResult NVVMIRBuilder::load(
    const String& path,
    ISlangSharedLibraryLoader* loader,
    NVVMIRBuilder& outBuilder)
{
    outBuilder = NVVMIRBuilder();
    if (!loader)
        loader = DefaultSharedLibraryLoader::getSingleton();

    ComPtr<ISlangSharedLibrary> library;
    SLANG_RETURN_ON_FAIL(DownstreamCompilerUtil::loadSharedLibrary(
        path,
        loader,
        nullptr,
        "slang-llvm-nvvm",
        library));
    if (!library)
        return SLANG_FAIL;

    SlangGetNVVMBuilderAPI getAPI = reinterpret_cast<SlangGetNVVMBuilderAPI>(
        library->findFuncByName(SLANG_NVVM_BUILDER_GET_API_NAME));
    if (!getAPI)
        return SLANG_E_NO_INTERFACE;

    SlangNVVMBuilderAPI api = {};
    SLANG_RETURN_ON_FAIL(getAPI(SLANG_NVVM_BUILDER_ABI_REVISION, &api));
    return initialize(api, library, outBuilder);
}

/* static */ SlangResult NVVMIRBuilder::initialize(
    const SlangNVVMBuilderAPI& api,
    ISlangSharedLibrary* library,
    NVVMIRBuilder& outBuilder)
{
    outBuilder = NVVMIRBuilder();
    if (!library)
        return SLANG_E_INVALID_ARG;
    if (api.llvmVersionMajor != 14 || api.llvmVersionMinor != 0 || api.llvmVersionPatch != 6 ||
        api.nvvmIRVersionMajor != 2 || api.nvvmIRVersionMinor != 0 ||
        api.pointerModel != SLANG_NVVM_POINTER_MODEL_TYPED || !api.queryInterface)
    {
        return SLANG_E_NO_INTERFACE;
    }

    const void* foundationRaw = nullptr;
    const void* constructionRaw = nullptr;
    const void* valueOperationsRaw = nullptr;
    SLANG_RETURN_ON_FAIL(
        api.queryInterface(SLANG_NVVM_BUILDER_INTERFACE_FOUNDATION, &foundationRaw));
    SLANG_RETURN_ON_FAIL(
        api.queryInterface(SLANG_NVVM_BUILDER_INTERFACE_CONSTRUCTION, &constructionRaw));
    SLANG_RETURN_ON_FAIL(
        api.queryInterface(SLANG_NVVM_BUILDER_INTERFACE_VALUE_OPERATIONS, &valueOperationsRaw));
    if (!foundationRaw || !constructionRaw || !valueOperationsRaw)
        return SLANG_E_NO_INTERFACE;

    const auto& foundation = *static_cast<const SlangNVVMBuilderFoundationAPI*>(foundationRaw);
    const auto& construction =
        *static_cast<const SlangNVVMBuilderConstructionAPI*>(constructionRaw);
    const auto& valueOperations =
        *static_cast<const SlangNVVMBuilderValueOperationsAPI*>(valueOperationsRaw);
    if (!_hasRequiredFoundation(foundation) || !_hasRequiredConstruction(construction) ||
        !_hasRequiredValueOperations(valueOperations))
    {
        return SLANG_E_NO_INTERFACE;
    }

    outBuilder.m_api = api;
    outBuilder.m_foundation = foundation;
    outBuilder.m_construction = construction;
    outBuilder.m_valueOperations = valueOperations;
    outBuilder.m_library = library;

    SlangNVVMBuilderFeatureSet& features = outBuilder.m_features;
    const SlangNVVMBuilderFeature structuralFeatures[] = {
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_SSA,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FUNCTIONS,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_POINTER_ARITHMETIC,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_ARRAY_ADDRESSING,
        SLANG_NVVM_BUILDER_FEATURE_RELAXED_GLOBAL_I32_ATOMIC_ADD,
        SLANG_NVVM_BUILDER_FEATURE_NVVM_IR_2_0_ASSEMBLY,
        SLANG_NVVM_BUILDER_FEATURE_RAW_RW_STRUCTURED_BUFFER_I32,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI,
        SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS,
    };
    for (SlangNVVMBuilderFeature feature : structuralFeatures)
        _addFeature(features, feature);

    for (SlangNVVMBuilderFeature feature = 0; feature < SLANG_NVVM_BUILDER_FEATURE_COUNT; ++feature)
    {
        bool hasCatalogEntries = false;
        bool supportsEveryEntry = true;
        for (const NVVMSemantics::CatalogEntry& entry : NVVMSemantics::kCatalog)
        {
            if (entry.legacyFeature != feature)
                continue;

            hasCatalogEntries = true;
            const SlangNVVMValueOperationDesc desc = NVVMSemantics::getOperationDesc(entry);
            uint32_t supported = 0;
            SLANG_RETURN_ON_FAIL(valueOperations.isOperationSupported(&desc, &supported));
            supportsEveryEntry = supportsEveryEntry && supported != 0;
        }
        if (hasCatalogEntries && supportsEveryEntry)
            _addFeature(features, feature);
    }
    return SLANG_OK;
}

bool NVVMIRBuilder::supportsScalarOperations() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY);
}

bool NVVMIRBuilder::supportsScalarControlFlow() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW);
}

bool NVVMIRBuilder::supportsScalarSSA() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_SSA);
}

bool NVVMIRBuilder::supportsScalarFunctions() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FUNCTIONS);
}

bool NVVMIRBuilder::supportsScalarPointerArithmetic() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_POINTER_ARITHMETIC);
}

bool NVVMIRBuilder::supportsScalarArrayAddressing() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_ARRAY_ADDRESSING);
}

bool NVVMIRBuilder::supportsScalarIntegerMultiply() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY);
}

bool NVVMIRBuilder::supportsScalarIntegerBitAnd() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_AND);
}

bool NVVMIRBuilder::supportsScalarIntegerBitOr() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_OR);
}

bool NVVMIRBuilder::supportsScalarIntegerBitXor() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_XOR);
}

bool NVVMIRBuilder::supportsScalarIntegerBitNot() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_NOT);
}

bool NVVMIRBuilder::supportsScalarIntegerNegate() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NEGATE);
}

bool NVVMIRBuilder::supportsNVVMIR20Assembly() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_NVVM_IR_2_0_ASSEMBLY);
}

bool NVVMIRBuilder::supportsRelaxedGlobalI32AtomicAdd() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_RELAXED_GLOBAL_I32_ATOMIC_ADD);
}

bool NVVMIRBuilder::supportsScalarIntegerEqual() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_EQUAL);
}

bool NVVMIRBuilder::supportsScalarIntegerNotEqual() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NOT_EQUAL);
}

bool NVVMIRBuilder::supportsScalarIntegerSignedGreaterThan() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_THAN);
}

bool NVVMIRBuilder::supportsScalarIntegerSignedLessEqual() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_LESS_EQUAL);
}

bool NVVMIRBuilder::supportsScalarIntegerSignedGreaterEqual() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_EQUAL);
}

bool NVVMIRBuilder::supportsRawRWStructuredBufferI32() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_RAW_RW_STRUCTURED_BUFFER_I32);
}

bool NVVMIRBuilder::supportsFeature(SlangNVVMBuilderFeature feature) const
{
    return feature < SLANG_NVVM_BUILDER_FEATURE_COUNT && _hasFeature(m_features, feature);
}

bool NVVMIRBuilder::supportsFeatures(const SlangNVVMBuilderFeatureSet& requiredFeatures) const
{
    for (uint32_t i = 0; i < SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT; ++i)
    {
        const uint32_t firstFeature = i * 64u;
        const uint32_t remainingFeatures = SLANG_NVVM_BUILDER_FEATURE_COUNT > firstFeature
                                               ? SLANG_NVVM_BUILDER_FEATURE_COUNT - firstFeature
                                               : 0;
        const uint64_t knownFeatureMask = remainingFeatures >= 64u ? ~uint64_t(0)
                                          : remainingFeatures
                                              ? (uint64_t(1) << remainingFeatures) - 1u
                                              : 0;
        if ((requiredFeatures.words[i] & ~knownFeatureMask) != 0 ||
            (requiredFeatures.words[i] & ~m_features.words[i]) != 0)
        {
            return false;
        }
    }
    return true;
}

bool NVVMIRBuilder::supportsValueOperation(const SlangNVVMValueOperationDesc& operation) const
{
    if (!isInitialized())
        return false;
    uint32_t supported = 0;
    return SLANG_SUCCEEDED(m_valueOperations.isOperationSupported(&operation, &supported)) &&
           supported != 0;
}

SlangResult NVVMIRBuilder::emitValueOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMValueOperationDesc& operation,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (operation.operandCount != operandCount ||
        (!operation.operandTypes && operation.operandCount) || (!operands && operandCount))
    {
        return SLANG_E_INVALID_ARG;
    }
    if (!supportsValueOperation(operation))
        return SLANG_E_NOT_AVAILABLE;

    const SlangNVVMResult result =
        m_valueOperations.emitOperation(module, &operation, operands, operandCount, &outValue);
    if (operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_VOID)
    {
        if (SLANG_FAILED(result))
            return result;
        return !outValue ? SLANG_OK : SLANG_FAIL;
    }
    return _validateHandleResult(result, outValue);
}

String NVVMIRBuilder::getVersionString() const
{
    if (!isInitialized())
        return String();

    StringBuilder builder;
    builder << "slang-llvm-nvvm;builder-abi=" << SLANG_NVVM_BUILDER_ABI_REVISION
            << ";llvm=" << m_api.llvmVersionMajor << "." << m_api.llvmVersionMinor << "."
            << m_api.llvmVersionPatch << ";nvvm-ir=" << m_api.nvvmIRVersionMajor << "."
            << m_api.nvvmIRVersionMinor << ";pointer-model=" << uint32_t(m_api.pointerModel)
            << ";feature-words=";
    for (uint32_t i = 0; i < SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT; ++i)
    {
        if (i)
            builder << ",";
        builder << m_features.words[i];
    }
    builder << ";timestamp="
            << SharedLibraryUtils::getSharedLibraryTimestamp(
                   reinterpret_cast<void*>(m_foundation.createModule));
    return builder.produceString();
}

SlangResult NVVMIRBuilder::createModule(
    const UnownedStringSlice& moduleName,
    SlangNVVMModuleHandle& outModule) const
{
    outModule = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult result =
        m_foundation.createModule(moduleName.begin(), moduleName.getLength(), &outModule);
    return _validateHandleResult(result, outModule);
}

void NVVMIRBuilder::destroyModule(SlangNVVMModuleHandle module) const
{
    if (isInitialized())
        m_foundation.destroyModule(module);
}

SlangResult NVVMIRBuilder::getVoidType(SlangNVVMModuleHandle module, SlangNVVMTypeHandle& outType)
    const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult result = m_construction.getVoidType(module, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getIntegerType(
    SlangNVVMModuleHandle module,
    uint32_t bitWidth,
    SlangNVVMTypeHandle& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result = m_construction.getIntegerType(module, bitWidth, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getFloatingPointType(
    SlangNVVMModuleHandle module,
    uint32_t bitWidth,
    SlangNVVMTypeHandle& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_DIVIDE) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NEGATE) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_EQUAL) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NOT_EQUAL) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_THAN) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_EQUAL) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_EQUAL) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_THAN) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result = m_construction.getFloatingPointType(module, bitWidth, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getPointerType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle pointeeType,
    SlangNVVMAddressSpace addressSpace,
    SlangNVVMTypeHandle& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.getPointerType(module, pointeeType, addressSpace, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getFunctionType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle resultType,
    const SlangNVVMTypeHandle* parameterTypes,
    size_t parameterCount,
    SlangNVVMTypeHandle& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult result =
        m_construction
            .getFunctionType(module, resultType, parameterTypes, parameterCount, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::declareFunction(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle functionType,
    const UnownedStringSlice& name,
    SlangNVVMValueHandle& outFunction) const
{
    outFunction = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult result =
        m_construction
            .declareFunction(module, functionType, name.begin(), name.getLength(), &outFunction);
    return _validateHandleResult(result, outFunction);
}

SlangResult NVVMIRBuilder::getFunctionParameter(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function,
    size_t parameterIndex,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.getFunctionParameter(module, function, parameterIndex, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::createBlock(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function,
    const UnownedStringSlice& name,
    SlangNVVMBlockHandle& outBlock) const
{
    outBlock = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult result =
        m_construction.createBlock(module, function, name.begin(), name.getLength(), &outBlock);
    return _validateHandleResult(result, outBlock);
}

SlangResult NVVMIRBuilder::setInsertBlock(SlangNVVMModuleHandle module, SlangNVVMBlockHandle block)
    const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    return m_construction.setInsertBlock(module, block);
}

SlangResult NVVMIRBuilder::emitLoad(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle pointer,
    uint32_t alignment,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result = m_construction.emitLoad(module, pointer, alignment, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitStore(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle pointer,
    uint32_t alignment) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    return m_construction.emitStore(module, value, pointer, alignment);
}

SlangResult NVVMIRBuilder::emitIntegerBinary(
    SlangNVVMModuleHandle module,
    SlangNVVMIntegerBinaryOp operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (operation != SLANG_NVVM_INTEGER_BINARY_OP_ADD &&
        operation != SLANG_NVVM_INTEGER_BINARY_OP_SUBTRACT)
    {
        return SLANG_E_INVALID_ARG;
    }
    if (!supportsScalarControlFlow())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerBinaryOperation(module, operation, left, right, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerUnary(
    SlangNVVMModuleHandle module,
    SlangNVVMIntegerUnaryOp operation,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    const NVVMSemantics::CatalogEntry* entry = NVVMSemantics::findLegacyOperation(
        NVVMSemantics::LegacyFamily::IntegerUnary,
        uint32_t(operation));
    if (!entry)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(entry->legacyFeature))
        return SLANG_E_NOT_AVAILABLE;

    const SlangNVVMValueOperationDesc desc = NVVMSemantics::getOperationDesc(*entry);
    const SlangNVVMValueHandle operands[] = {value};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBinaryOperation(
    SlangNVVMModuleHandle module,
    SlangNVVMIntegerBinaryOp operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    const NVVMSemantics::CatalogEntry* entry = NVVMSemantics::findLegacyOperation(
        NVVMSemantics::LegacyFamily::IntegerBinary,
        uint32_t(operation));
    if (!entry)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(entry->legacyFeature))
        return SLANG_E_NOT_AVAILABLE;

    const SlangNVVMValueOperationDesc desc = NVVMSemantics::getOperationDesc(*entry);
    const SlangNVVMValueHandle operands[] = {left, right};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitIntegerCompare(
    SlangNVVMModuleHandle module,
    SlangNVVMIntegerCompareOp operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    const NVVMSemantics::CatalogEntry* entry = NVVMSemantics::findLegacyOperation(
        NVVMSemantics::LegacyFamily::IntegerCompare,
        uint32_t(operation));
    if (!entry)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(entry->legacyFeature))
        return SLANG_E_NOT_AVAILABLE;

    const SlangNVVMValueOperationDesc desc = NVVMSemantics::getOperationDesc(*entry);
    const SlangNVVMValueHandle operands[] = {left, right};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitFloatingBinary(
    SlangNVVMModuleHandle module,
    SlangNVVMFloatingBinaryOp operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    const NVVMSemantics::CatalogEntry* entry = NVVMSemantics::findLegacyOperation(
        NVVMSemantics::LegacyFamily::FloatingBinary,
        uint32_t(operation));
    if (!entry)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(entry->legacyFeature))
        return SLANG_E_NOT_AVAILABLE;

    const SlangNVVMValueOperationDesc desc = NVVMSemantics::getOperationDesc(*entry);
    const SlangNVVMValueHandle operands[] = {left, right};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitFloatingUnary(
    SlangNVVMModuleHandle module,
    SlangNVVMFloatingUnaryOp operation,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    const NVVMSemantics::CatalogEntry* entry = NVVMSemantics::findLegacyOperation(
        NVVMSemantics::LegacyFamily::FloatingUnary,
        uint32_t(operation));
    if (!entry)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(entry->legacyFeature))
        return SLANG_E_NOT_AVAILABLE;

    const SlangNVVMValueOperationDesc desc = NVVMSemantics::getOperationDesc(*entry);
    const SlangNVVMValueHandle operands[] = {value};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitFloatingCompare(
    SlangNVVMModuleHandle module,
    SlangNVVMFloatingCompareOp operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    const NVVMSemantics::CatalogEntry* entry = NVVMSemantics::findLegacyOperation(
        NVVMSemantics::LegacyFamily::FloatingCompare,
        uint32_t(operation));
    if (!entry)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(entry->legacyFeature))
        return SLANG_E_NOT_AVAILABLE;

    const SlangNVVMValueOperationDesc desc = NVVMSemantics::getOperationDesc(*entry);
    const SlangNVVMValueHandle operands[] = {left, right};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::getFloatingPointConstant(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle floatingPointType,
    uint32_t bitWidth,
    uint64_t bitPattern,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (bitWidth != 32 || (bitPattern >> 32) != 0)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction
            .getFloatingPointConstant(module, floatingPointType, bitWidth, bitPattern, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitPhi(
    SlangNVVMModuleHandle module,
    SlangNVVMBlockHandle targetBlock,
    SlangNVVMTypeHandle type,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result = m_construction.emitPhi(module, targetBlock, type, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::addPhiIncoming(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle phi,
    SlangNVVMValueHandle value,
    SlangNVVMBlockHandle predecessorBlock) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI))
        return SLANG_E_NOT_AVAILABLE;
    return m_construction.addPhiIncoming(module, phi, value, predecessorBlock);
}

SlangResult NVVMIRBuilder::emitCall(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle callee,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.emitCall(module, callee, arguments, argumentCount, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitValueReturn(SlangNVVMModuleHandle module, SlangNVVMValueHandle value)
    const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS))
        return SLANG_E_NOT_AVAILABLE;
    return m_construction.emitValueReturn(module, value);
}

SlangResult NVVMIRBuilder::emitIntrinsic(
    SlangNVVMModuleHandle module,
    SlangNVVMIntrinsicOp operation,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    const NVVMSemantics::CatalogEntry* entry = NVVMSemantics::findLegacyOperation(
        NVVMSemantics::LegacyFamily::Intrinsic,
        uint32_t(operation));
    if (!entry || argumentCount != entry->operandCount)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(entry->legacyFeature))
        return SLANG_E_NOT_AVAILABLE;

    const SlangNVVMValueOperationDesc desc = NVVMSemantics::getOperationDesc(*entry);
    return emitValueOperation(module, desc, arguments, argumentCount, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerSignedLessThan(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarControlFlow())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerCompare(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN,
        left,
        right,
        outValue);
}

SlangResult NVVMIRBuilder::emitBranch(
    SlangNVVMModuleHandle module,
    SlangNVVMBlockHandle targetBlock) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarControlFlow())
        return SLANG_E_NOT_AVAILABLE;
    return m_construction.emitBranch(module, targetBlock);
}

SlangResult NVVMIRBuilder::emitConditionalBranch(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle condition,
    SlangNVVMBlockHandle trueBlock,
    SlangNVVMBlockHandle falseBlock) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarControlFlow())
        return SLANG_E_NOT_AVAILABLE;
    return m_construction.emitConditionalBranch(module, condition, trueBlock, falseBlock);
}

SlangResult NVVMIRBuilder::getIntegerConstant(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle integerType,
    int64_t value,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarSSA())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.getIntegerConstant(module, integerType, value, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerPhi(
    SlangNVVMModuleHandle module,
    SlangNVVMBlockHandle targetBlock,
    SlangNVVMTypeHandle integerType,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarSSA())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.emitPhi(module, targetBlock, integerType, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::addIntegerPhiIncoming(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle phi,
    SlangNVVMValueHandle value,
    SlangNVVMBlockHandle predecessorBlock) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarSSA())
        return SLANG_E_NOT_AVAILABLE;
    return m_construction.addPhiIncoming(module, phi, value, predecessorBlock);
}

SlangResult NVVMIRBuilder::emitIntegerCall(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle callee,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarFunctions())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.emitCall(module, callee, arguments, argumentCount, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerReturn(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarFunctions())
        return SLANG_E_NOT_AVAILABLE;
    return m_construction.emitValueReturn(module, value);
}

SlangResult NVVMIRBuilder::emitPointerOffset(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle basePointer,
    SlangNVVMValueHandle elementOffset,
    SlangNVVMValueHandle& outPointer) const
{
    outPointer = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarPointerArithmetic())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.emitPointerOffset(module, basePointer, elementOffset, &outPointer);
    return _validateHandleResult(result, outPointer);
}

SlangResult NVVMIRBuilder::getArrayType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarArrayAddressing())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.getArrayType(module, elementType, elementCount, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getVectorType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsVectorConstruction())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.getVectorType(module, elementType, elementCount, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::declareGlobalStorage(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle valueType,
    SlangNVVMAddressSpace addressSpace,
    uint32_t alignment,
    const UnownedStringSlice& name,
    SlangNVVMValueHandle& outStorage) const
{
    outStorage = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsGlobalStorage())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result = m_construction.declareGlobalStorage(
        module,
        valueType,
        addressSpace,
        alignment,
        name.begin(),
        size_t(name.getLength()),
        &outStorage);
    return _validateHandleResult(result, outStorage);
}

SlangResult NVVMIRBuilder::emitVectorElementExtract(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle vector,
    uint32_t elementIndex,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsVectorConstruction())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.emitVectorElementExtract(module, vector, elementIndex, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitArrayElementPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle baseArrayPointer,
    SlangNVVMValueHandle elementIndex,
    SlangNVVMValueHandle& outPointer) const
{
    outPointer = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarArrayAddressing())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.emitArrayElementPointer(module, baseArrayPointer, elementIndex, &outPointer);
    return _validateHandleResult(result, outPointer);
}

SlangResult NVVMIRBuilder::emitIntegerMultiply(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerMultiply())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerBinaryOperation(
        module,
        SLANG_NVVM_INTEGER_BINARY_OP_MULTIPLY,
        left,
        right,
        outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBitAnd(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerBitAnd())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerBinaryOperation(
        module,
        SLANG_NVVM_INTEGER_BINARY_OP_BIT_AND,
        left,
        right,
        outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBitOr(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerBitOr())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerBinaryOperation(
        module,
        SLANG_NVVM_INTEGER_BINARY_OP_BIT_OR,
        left,
        right,
        outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBitXor(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerBitXor())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerBinaryOperation(
        module,
        SLANG_NVVM_INTEGER_BINARY_OP_BIT_XOR,
        left,
        right,
        outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBitNot(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerBitNot())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerUnary(module, SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT, value, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerNegate(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerNegate())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerUnary(module, SLANG_NVVM_INTEGER_UNARY_OP_NEGATE, value, outValue);
}

SlangResult NVVMIRBuilder::emitRelaxedGlobalI32AtomicAdd(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle pointer,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outOriginalValue) const
{
    outOriginalValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsRelaxedGlobalI32AtomicAdd())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result =
        m_construction.emitRelaxedGlobalI32AtomicAdd(module, pointer, value, &outOriginalValue);
    return _validateHandleResult(result, outOriginalValue);
}

SlangResult NVVMIRBuilder::emitIntegerEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerEqual())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerCompare(module, SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL, left, right, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerNotEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerNotEqual())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerCompare(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL,
        left,
        right,
        outValue);
}

SlangResult NVVMIRBuilder::emitIntegerSignedGreaterThan(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerSignedGreaterThan())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerCompare(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN,
        left,
        right,
        outValue);
}

SlangResult NVVMIRBuilder::emitIntegerSignedLessEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerSignedLessEqual())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerCompare(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL,
        left,
        right,
        outValue);
}

SlangResult NVVMIRBuilder::emitIntegerSignedGreaterEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerSignedGreaterEqual())
        return SLANG_E_NOT_AVAILABLE;
    return emitIntegerCompare(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL,
        left,
        right,
        outValue);
}

SlangResult NVVMIRBuilder::getRawRWStructuredBufferI32Type(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsRawRWStructuredBufferI32())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result = m_construction.getRawRWStructuredBufferI32Type(module, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::emitRawRWStructuredBufferI32ElementPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle buffer,
    SlangNVVMValueHandle elementIndex,
    SlangNVVMValueHandle& outPointer) const
{
    outPointer = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsRawRWStructuredBufferI32())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult result = m_construction.emitRawRWStructuredBufferI32ElementPointer(
        module,
        buffer,
        elementIndex,
        &outPointer);
    return _validateHandleResult(result, outPointer);
}

SlangResult NVVMIRBuilder::emitReturnVoid(SlangNVVMModuleHandle module) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    return m_construction.emitReturnVoid(module);
}

SlangResult NVVMIRBuilder::markFunctionAsKernel(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    return m_construction.markFunctionAsKernel(module, function);
}

SlangResult NVVMIRBuilder::serializeModule(
    SlangNVVMModuleHandle module,
    SlangNVVMSerializationFormat format,
    ComPtr<ISlangBlob>& outBlob) const
{
    String diagnostics;
    return serializeModule(module, format, outBlob, diagnostics);
}

SlangResult NVVMIRBuilder::serializeModule(
    SlangNVVMModuleHandle module,
    SlangNVVMSerializationFormat format,
    ComPtr<ISlangBlob>& outBlob,
    String& outDiagnostics) const
{
    outBlob.setNull();
    outDiagnostics = String();
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsSerializationDiagnostics())
        return SLANG_E_NOT_AVAILABLE;

    auto serializeWithDiagnostics = m_foundation.serializeModuleWithDiagnostics;
    if (format == SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY)
    {
        if (!supportsNVVMIR20Assembly())
            return SLANG_E_NOT_AVAILABLE;
        serializeWithDiagnostics = m_foundation.serializeNVVMIR20AssemblyWithDiagnostics;
    }

    size_t requiredSerializedSize = 0;
    size_t requiredDiagnosticSize = 0;
    SlangNVVMVerificationStatus queryStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_RETURN_ON_FAIL(serializeWithDiagnostics(
        module,
        format,
        nullptr,
        0,
        &requiredSerializedSize,
        nullptr,
        0,
        &requiredDiagnosticSize,
        &queryStatus));

    if (!_isVerificationStatus(queryStatus) || UInt64(requiredSerializedSize) > UInt64(kMaxIndex) ||
        UInt64(requiredDiagnosticSize) > UInt64(kMaxIndex))
    {
        return SLANG_FAIL;
    }
    if (queryStatus == SLANG_NVVM_VERIFICATION_VALID)
    {
        if (!requiredSerializedSize)
            return SLANG_FAIL;
    }
    else if (requiredSerializedSize || !requiredDiagnosticSize)
    {
        return SLANG_FAIL;
    }

    List<uint8_t> serializedStorage;
    serializedStorage.setCount(Index(requiredSerializedSize));
    List<char> diagnosticStorage;
    diagnosticStorage.setCount(Index(requiredDiagnosticSize));

    size_t actualSerializedSize = 0;
    size_t actualDiagnosticSize = 0;
    SlangNVVMVerificationStatus writeStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_RETURN_ON_FAIL(serializeWithDiagnostics(
        module,
        format,
        serializedStorage.getCount() ? serializedStorage.getBuffer() : nullptr,
        requiredSerializedSize,
        &actualSerializedSize,
        diagnosticStorage.getCount() ? diagnosticStorage.getBuffer() : nullptr,
        requiredDiagnosticSize,
        &actualDiagnosticSize,
        &writeStatus));

    if (actualSerializedSize != requiredSerializedSize ||
        actualDiagnosticSize != requiredDiagnosticSize || writeStatus != queryStatus)
    {
        return SLANG_FAIL;
    }

    if (requiredDiagnosticSize)
    {
        outDiagnostics = String(
            UnownedStringSlice(diagnosticStorage.getBuffer(), Index(requiredDiagnosticSize)));
    }
    if (queryStatus == SLANG_NVVM_VERIFICATION_INVALID)
        return SLANG_FAIL;

    outBlob = ListBlob::moveCreate(serializedStorage);
    return SLANG_OK;
}

} // namespace Slang
