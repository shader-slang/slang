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

static bool _hasRequiredFoundation(const SlangNVVMBuilderFoundationAPI& api)
{
    return api.createModule && api.destroyModule && api.serializeModuleWithDiagnostics &&
           api.serializeNVVMIR20AssemblyWithDiagnostics;
}

static bool _hasRequiredConstruction(const SlangNVVMBuilderConstructionAPI& api)
{
    return api.getVoidType && api.getIntegerType && api.getFloatingPointType &&
           api.getPointerType && api.getFunctionType && api.getArrayType && api.getVectorType &&
           api.getStructType && api.getRawRWStructuredBufferI32Type && api.declareFunction &&
           api.getFunctionParameter && api.createBlock && api.setInsertBlock && api.emitLoad &&
           api.emitStore && api.emitBranch && api.emitConditionalBranch && api.getIntegerConstant &&
           api.getFloatingPointConstant && api.emitPhi && api.addPhiIncoming && api.emitCall &&
           api.emitValueReturn && api.emitReturnVoid && api.emitPointerOffset &&
           api.emitArrayElementPointer && api.emitStructFieldPointer &&
           api.emitVectorElementExtract && api.emitRawRWStructuredBufferI32ElementPointer &&
           api.emitRelaxedGlobalI32AtomicAdd && api.declareGlobalStorage &&
           api.markFunctionAsKernel;
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
    return SLANG_OK;
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
            << ";timestamp="
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
    return m_construction.emitStore(module, value, pointer, alignment);
}

SlangResult NVVMIRBuilder::emitIntegerBinary(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (operation != SLANG_NVVM_VALUE_OP_ADD && operation != SLANG_NVVM_VALUE_OP_SUBTRACT)
    {
        return SLANG_E_INVALID_ARG;
    }
    return emitIntegerBinaryOperation(module, operation, left, right, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerUnary(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    if (operation != SLANG_NVVM_VALUE_OP_BIT_NOT && operation != SLANG_NVVM_VALUE_OP_NEGATE)
        return SLANG_E_INVALID_ARG;
    const SlangNVVMValueTypeDesc operandTypes[] = {NVVMSemantics::kSignedI32};
    const SlangNVVMValueOperationDesc desc = {
        operation,
        NVVMSemantics::kSignedI32,
        operandTypes,
        SLANG_COUNT_OF(operandTypes),
    };
    const SlangNVVMValueHandle operands[] = {value};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBinaryOperation(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    if (operation < SLANG_NVVM_VALUE_OP_ADD || operation > SLANG_NVVM_VALUE_OP_BIT_XOR)
        return SLANG_E_INVALID_ARG;
    const SlangNVVMValueTypeDesc operandTypes[] = {
        NVVMSemantics::kSignedI32,
        NVVMSemantics::kSignedI32,
    };
    const SlangNVVMValueOperationDesc desc = {
        operation,
        NVVMSemantics::kSignedI32,
        operandTypes,
        SLANG_COUNT_OF(operandTypes),
    };
    const SlangNVVMValueHandle operands[] = {left, right};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitIntegerCompare(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    if (operation < SLANG_NVVM_VALUE_OP_EQUAL || operation > SLANG_NVVM_VALUE_OP_GREATER_EQUAL)
        return SLANG_E_INVALID_ARG;
    const SlangNVVMValueTypeDesc operandTypes[] = {
        NVVMSemantics::kSignedI32,
        NVVMSemantics::kSignedI32,
    };
    const SlangNVVMValueOperationDesc desc = {
        operation,
        NVVMSemantics::kBool,
        operandTypes,
        SLANG_COUNT_OF(operandTypes),
    };
    const SlangNVVMValueHandle operands[] = {left, right};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitFloatingBinary(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    if (operation < SLANG_NVVM_VALUE_OP_ADD || operation > SLANG_NVVM_VALUE_OP_DIVIDE)
        return SLANG_E_INVALID_ARG;
    const SlangNVVMValueTypeDesc operandTypes[] = {
        NVVMSemantics::kFloat32,
        NVVMSemantics::kFloat32,
    };
    const SlangNVVMValueOperationDesc desc = {
        operation,
        NVVMSemantics::kFloat32,
        operandTypes,
        SLANG_COUNT_OF(operandTypes),
    };
    const SlangNVVMValueHandle operands[] = {left, right};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitFloatingUnary(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    if (operation != SLANG_NVVM_VALUE_OP_NEGATE)
        return SLANG_E_INVALID_ARG;
    const SlangNVVMValueTypeDesc operandTypes[] = {NVVMSemantics::kFloat32};
    const SlangNVVMValueOperationDesc desc = {
        operation,
        NVVMSemantics::kFloat32,
        operandTypes,
        SLANG_COUNT_OF(operandTypes),
    };
    const SlangNVVMValueHandle operands[] = {value};
    return emitValueOperation(module, desc, operands, SLANG_COUNT_OF(operands), outValue);
}

SlangResult NVVMIRBuilder::emitFloatingCompare(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    if (operation < SLANG_NVVM_VALUE_OP_EQUAL || operation > SLANG_NVVM_VALUE_OP_GREATER_EQUAL)
        return SLANG_E_INVALID_ARG;
    const SlangNVVMValueTypeDesc operandTypes[] = {
        NVVMSemantics::kFloat32,
        NVVMSemantics::kFloat32,
    };
    const SlangNVVMValueOperationDesc desc = {
        operation,
        NVVMSemantics::kBool,
        operandTypes,
        SLANG_COUNT_OF(operandTypes),
    };
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
    const SlangNVVMResult result =
        m_construction.emitCall(module, callee, arguments, argumentCount, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitValueReturn(SlangNVVMModuleHandle module, SlangNVVMValueHandle value)
    const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    return m_construction.emitValueReturn(module, value);
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
    return emitIntegerCompare(module, SLANG_NVVM_VALUE_OP_LESS_THAN, left, right, outValue);
}

SlangResult NVVMIRBuilder::emitBranch(
    SlangNVVMModuleHandle module,
    SlangNVVMBlockHandle targetBlock) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
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
    const SlangNVVMResult result =
        m_construction.getVectorType(module, elementType, elementCount, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getStructType(
    SlangNVVMModuleHandle module,
    const SlangNVVMTypeHandle* fieldTypes,
    size_t fieldCount,
    SlangNVVMTypeHandle& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult result =
        m_construction.getStructType(module, fieldTypes, fieldCount, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::declareGlobalStorage(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle valueType,
    SlangNVVMGlobalLinkage linkage,
    SlangNVVMAddressSpace addressSpace,
    uint32_t alignment,
    const UnownedStringSlice& name,
    SlangNVVMValueHandle& outStorage) const
{
    outStorage = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult result = m_construction.declareGlobalStorage(
        module,
        valueType,
        linkage,
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
    const SlangNVVMResult result =
        m_construction.emitArrayElementPointer(module, baseArrayPointer, elementIndex, &outPointer);
    return _validateHandleResult(result, outPointer);
}

SlangResult NVVMIRBuilder::emitStructFieldPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle baseStructPointer,
    uint32_t fieldIndex,
    SlangNVVMValueHandle& outPointer) const
{
    outPointer = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult result =
        m_construction.emitStructFieldPointer(module, baseStructPointer, fieldIndex, &outPointer);
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
    return emitIntegerBinaryOperation(module, SLANG_NVVM_VALUE_OP_MULTIPLY, left, right, outValue);
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
    return emitIntegerBinaryOperation(module, SLANG_NVVM_VALUE_OP_BIT_AND, left, right, outValue);
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
    return emitIntegerBinaryOperation(module, SLANG_NVVM_VALUE_OP_BIT_OR, left, right, outValue);
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
    return emitIntegerBinaryOperation(module, SLANG_NVVM_VALUE_OP_BIT_XOR, left, right, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBitNot(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    return emitIntegerUnary(module, SLANG_NVVM_VALUE_OP_BIT_NOT, value, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerNegate(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    return emitIntegerUnary(module, SLANG_NVVM_VALUE_OP_NEGATE, value, outValue);
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
    return emitIntegerCompare(module, SLANG_NVVM_VALUE_OP_EQUAL, left, right, outValue);
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
    return emitIntegerCompare(module, SLANG_NVVM_VALUE_OP_NOT_EQUAL, left, right, outValue);
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
    return emitIntegerCompare(module, SLANG_NVVM_VALUE_OP_GREATER_THAN, left, right, outValue);
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
    return emitIntegerCompare(module, SLANG_NVVM_VALUE_OP_LESS_EQUAL, left, right, outValue);
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
    return emitIntegerCompare(module, SLANG_NVVM_VALUE_OP_GREATER_EQUAL, left, right, outValue);
}

SlangResult NVVMIRBuilder::getRawRWStructuredBufferI32Type(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
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

    auto serializeWithDiagnostics = m_foundation.serializeModuleWithDiagnostics;
    if (format == SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY)
    {
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
