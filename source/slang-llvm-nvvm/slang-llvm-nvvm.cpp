#include "compiler-core/slang-nvvm-ir-builder-api.h"
#include "slang.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/raw_ostream.h"

#include <cstring>
#include <memory>
#include <new>
#include <string>

#if LLVM_VERSION_MAJOR != 14 || LLVM_VERSION_MINOR != 0 || LLVM_VERSION_PATCH != 6
#error slang-llvm-nvvm requires LLVM 14.0.6
#endif

namespace
{

static const char kNVPTX64DataLayout[] =
    "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-i128:128:128-"
    "f32:32:32-f64:64:64-v16:16:16-v32:32:32-v64:64:64-v128:128:128-n16:32:64";

struct ModuleState
{
    ModuleState(llvm::StringRef moduleName)
        : module(new llvm::Module(moduleName, context)), builder(context)
    {
        module->setTargetTriple("nvptx64-nvidia-cuda");
        module->setDataLayout(kNVPTX64DataLayout);

        llvm::Type* int32Type = llvm::Type::getInt32Ty(context);
        llvm::Metadata* versionOperands[] = {
            llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(int32Type, 2)),
            llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(int32Type, 0)),
        };
        module->getOrInsertNamedMetadata("nvvmir.version")
            ->addOperand(llvm::MDNode::get(context, versionOperands));
    }

    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;
};

static ModuleState* _getModule(SlangNVVMModuleHandle_1 module)
{
    return reinterpret_cast<ModuleState*>(module);
}

static llvm::Type* _getType(SlangNVVMTypeHandle_1 type)
{
    return reinterpret_cast<llvm::Type*>(type);
}

static llvm::Value* _getValue(SlangNVVMValueHandle_1 value)
{
    return reinterpret_cast<llvm::Value*>(value);
}

static llvm::BasicBlock* _getBlock(SlangNVVMBlockHandle_1 block)
{
    return reinterpret_cast<llvm::BasicBlock*>(block);
}

static llvm::StringRef _getStringRef(const char* data, size_t size)
{
    return size ? llvm::StringRef(data, size) : llvm::StringRef();
}

static bool _isNVVMAddressSpace(SlangNVVMAddressSpace_2 addressSpace)
{
    return addressSpace == SLANG_NVVM_ADDRESS_SPACE_GENERIC ||
           addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL ||
           addressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED ||
           addressSpace == SLANG_NVVM_ADDRESS_SPACE_CONSTANT ||
           addressSpace == SLANG_NVVM_ADDRESS_SPACE_LOCAL;
}

static llvm::PointerType* _getLoadablePointerType(ModuleState* state, llvm::Value* pointer)
{
    if (!state || !pointer || &pointer->getContext() != &state->context)
        return nullptr;

    llvm::PointerType* pointerType = llvm::dyn_cast<llvm::PointerType>(pointer->getType());
    if (!pointerType || pointerType->isOpaque() ||
        !_isNVVMAddressSpace(pointerType->getAddressSpace()))
    {
        return nullptr;
    }

    llvm::Type* pointeeType = pointerType->getNonOpaquePointerElementType();
    return llvm::PointerType::isLoadableOrStorableType(pointeeType) ? pointerType : nullptr;
}

static bool _hasValidInsertionBlock(ModuleState* state)
{
    if (!state)
        return false;

    llvm::BasicBlock* block = state->builder.GetInsertBlock();
    return block && !block->getTerminator() && block->getParent() &&
           block->getParent()->getParent() == state->module.get();
}

static bool _isValidAlignment(uint32_t alignment)
{
    return llvm::isPowerOf2_32(alignment);
}

static SlangResult SLANG_NVVM_CALL
_createModule(const char* moduleName, size_t moduleNameSize, SlangNVVMModuleHandle_1* outModule)
{
    if (!outModule || (!moduleName && moduleNameSize))
        return SLANG_E_INVALID_ARG;

    *outModule = nullptr;
    ModuleState* state = new (std::nothrow) ModuleState(_getStringRef(moduleName, moduleNameSize));
    if (!state)
        return SLANG_E_OUT_OF_MEMORY;

    *outModule = reinterpret_cast<SlangNVVMModuleHandle_1>(state);
    return SLANG_OK;
}

static void SLANG_NVVM_CALL _destroyModule(SlangNVVMModuleHandle_1 module)
{
    delete _getModule(module);
}

static SlangResult SLANG_NVVM_CALL
_getVoidType(SlangNVVMModuleHandle_1 module, SlangNVVMTypeHandle_1* outType)
{
    ModuleState* state = _getModule(module);
    if (!state || !outType)
        return SLANG_E_INVALID_ARG;

    *outType = reinterpret_cast<SlangNVVMTypeHandle_1>(llvm::Type::getVoidTy(state->context));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_getIntegerType(SlangNVVMModuleHandle_1 module, uint32_t bitWidth, SlangNVVMTypeHandle_1* outType)
{
    if (outType)
        *outType = nullptr;

    ModuleState* state = _getModule(module);
    if (!state || !outType || bitWidth == 0 || bitWidth > uint32_t(llvm::IntegerType::MAX_INT_BITS))
    {
        return SLANG_E_INVALID_ARG;
    }

    *outType =
        reinterpret_cast<SlangNVVMTypeHandle_1>(llvm::IntegerType::get(state->context, bitWidth));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getPointerType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 pointeeType,
    SlangNVVMAddressSpace_2 addressSpace,
    SlangNVVMTypeHandle_1* outType)
{
    if (outType)
        *outType = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Type* llvmPointeeType = _getType(pointeeType);
    if (!state || !llvmPointeeType || &llvmPointeeType->getContext() != &state->context ||
        !llvm::PointerType::isLoadableOrStorableType(llvmPointeeType) ||
        !_isNVVMAddressSpace(addressSpace) || !outType)
    {
        return SLANG_E_INVALID_ARG;
    }

    *outType = reinterpret_cast<SlangNVVMTypeHandle_1>(
        llvm::PointerType::get(llvmPointeeType, addressSpace));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getFunctionType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 resultType,
    const SlangNVVMTypeHandle_1* parameterTypes,
    size_t parameterCount,
    SlangNVVMTypeHandle_1* outType)
{
    ModuleState* state = _getModule(module);
    llvm::Type* llvmResultType = _getType(resultType);
    if (!state || !llvmResultType || &llvmResultType->getContext() != &state->context ||
        !llvm::FunctionType::isValidReturnType(llvmResultType) ||
        (!parameterTypes && parameterCount) || !outType)
        return SLANG_E_INVALID_ARG;

    llvm::SmallVector<llvm::Type*, 8> llvmParameterTypes;
    llvmParameterTypes.reserve(parameterCount);
    for (size_t i = 0; i < parameterCount; ++i)
    {
        llvm::Type* parameterType = _getType(parameterTypes[i]);
        if (!parameterType || &parameterType->getContext() != &state->context ||
            !llvm::FunctionType::isValidArgumentType(parameterType))
        {
            return SLANG_E_INVALID_ARG;
        }
        llvmParameterTypes.push_back(parameterType);
    }

    *outType = reinterpret_cast<SlangNVVMTypeHandle_1>(
        llvm::FunctionType::get(llvmResultType, llvmParameterTypes, false));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _declareFunction(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 functionType,
    const char* name,
    size_t nameSize,
    SlangNVVMValueHandle_1* outFunction)
{
    ModuleState* state = _getModule(module);
    llvm::FunctionType* llvmFunctionType =
        llvm::dyn_cast_or_null<llvm::FunctionType>(_getType(functionType));
    if (!state || !llvmFunctionType || &llvmFunctionType->getContext() != &state->context ||
        !name || !nameSize || !outFunction)
        return SLANG_E_INVALID_ARG;

    const llvm::StringRef llvmName = _getStringRef(name, nameSize);
    if (state->module->getNamedValue(llvmName))
        return SLANG_E_INVALID_ARG;

    llvm::Function* function = llvm::Function::Create(
        llvmFunctionType,
        llvm::GlobalValue::ExternalLinkage,
        llvmName,
        *state->module);
    *outFunction = reinterpret_cast<SlangNVVMValueHandle_1>(function);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getFunctionParameter(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function,
    size_t parameterIndex,
    SlangNVVMValueHandle_1* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Function* llvmFunction = llvm::dyn_cast_or_null<llvm::Function>(_getValue(function));
    if (!state || !llvmFunction || llvmFunction->getParent() != state->module.get() || !outValue ||
        parameterIndex >= llvmFunction->arg_size())
    {
        return SLANG_E_INVALID_ARG;
    }

    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
        llvmFunction->getArg(static_cast<unsigned>(parameterIndex)));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _createBlock(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function,
    const char* name,
    size_t nameSize,
    SlangNVVMBlockHandle_1* outBlock)
{
    ModuleState* state = _getModule(module);
    llvm::Function* llvmFunction = llvm::dyn_cast_or_null<llvm::Function>(_getValue(function));
    if (!state || !llvmFunction || llvmFunction->getParent() != state->module.get() ||
        (!name && nameSize) || !outBlock)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::BasicBlock* block =
        llvm::BasicBlock::Create(state->context, _getStringRef(name, nameSize), llvmFunction);
    *outBlock = reinterpret_cast<SlangNVVMBlockHandle_1>(block);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_setInsertBlock(SlangNVVMModuleHandle_1 module, SlangNVVMBlockHandle_1 block)
{
    ModuleState* state = _getModule(module);
    llvm::BasicBlock* llvmBlock = _getBlock(block);
    if (!state || !llvmBlock || !llvmBlock->getParent() ||
        llvmBlock->getParent()->getParent() != state->module.get())
    {
        return SLANG_E_INVALID_ARG;
    }

    state->builder.SetInsertPoint(llvmBlock);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitLoad(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 pointer,
    uint32_t alignment,
    SlangNVVMValueHandle_1* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmPointer = _getValue(pointer);
    llvm::PointerType* pointerType = _getLoadablePointerType(state, llvmPointer);
    if (!pointerType || !_hasValidInsertionBlock(state) || !_isValidAlignment(alignment) ||
        !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Type* pointeeType = pointerType->getNonOpaquePointerElementType();
    llvm::Value* value =
        state->builder.CreateAlignedLoad(pointeeType, llvmPointer, llvm::Align(alignment));
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(value);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitStore(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1 pointer,
    uint32_t alignment)
{
    ModuleState* state = _getModule(module);
    llvm::Value* llvmValue = _getValue(value);
    llvm::Value* llvmPointer = _getValue(pointer);
    llvm::PointerType* pointerType = _getLoadablePointerType(state, llvmPointer);
    if (!pointerType || !llvmValue || &llvmValue->getContext() != &state->context ||
        llvmValue->getType() != pointerType->getNonOpaquePointerElementType() ||
        pointerType->getAddressSpace() == SLANG_NVVM_ADDRESS_SPACE_CONSTANT ||
        !_hasValidInsertionBlock(state) || !_isValidAlignment(alignment))
    {
        return SLANG_E_INVALID_ARG;
    }

    state->builder.CreateAlignedStore(llvmValue, llvmPointer, llvm::Align(alignment));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitReturnVoid(SlangNVVMModuleHandle_1 module)
{
    ModuleState* state = _getModule(module);
    if (!state)
        return SLANG_E_INVALID_ARG;

    llvm::BasicBlock* block = state->builder.GetInsertBlock();
    if (!block || block->getTerminator() || !block->getParent() ||
        block->getParent()->getParent() != state->module.get() ||
        !block->getParent()->getReturnType()->isVoidTy())
    {
        return SLANG_E_INVALID_ARG;
    }

    state->builder.CreateRetVoid();
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_markFunctionAsKernel(SlangNVVMModuleHandle_1 module, SlangNVVMValueHandle_1 function)
{
    ModuleState* state = _getModule(module);
    llvm::Function* llvmFunction = llvm::dyn_cast_or_null<llvm::Function>(_getValue(function));
    if (!state || !llvmFunction || llvmFunction->getParent() != state->module.get())
        return SLANG_E_INVALID_ARG;

    llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
    llvm::Metadata* annotationOperands[] = {
        llvm::ValueAsMetadata::get(llvmFunction),
        llvm::MDString::get(state->context, "kernel"),
        llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(int32Type, 1)),
    };
    state->module->getOrInsertNamedMetadata("nvvm.annotations")
        ->addOperand(llvm::MDNode::get(state->context, annotationOperands));
    return SLANG_OK;
}

static SlangResult _copySerializedData(
    llvm::StringRef data,
    void* destination,
    size_t destinationSize,
    size_t* outSerializedSize)
{
    if (!outSerializedSize || (!destination && destinationSize))
        return SLANG_E_INVALID_ARG;

    *outSerializedSize = data.size();
    if (!destination)
        return SLANG_OK;
    if (destinationSize < data.size())
        return SLANG_E_BUFFER_TOO_SMALL;

    if (!data.empty())
        std::memcpy(destination, data.data(), data.size());
    return SLANG_OK;
}

// Checks whether the provider supports the requested wire encoding.
static bool _isSerializationFormat(SlangNVVMSerializationFormat_1 format)
{
    return format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY ||
           format == SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE;
}

// Verifies once and materializes the one canonical byte result shared by the V1 and V2 getters.
static SlangResult _materializeModule(
    ModuleState* state,
    SlangNVVMSerializationFormat_1 format,
    llvm::SmallVectorImpl<char>& outSerializedData,
    llvm::SmallVectorImpl<char>& outDiagnosticData,
    SlangNVVMVerificationStatus_2& outVerificationStatus)
{
    outSerializedData.clear();
    outDiagnosticData.clear();
    outVerificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    if (!state)
        return SLANG_E_INVALID_ARG;

    std::string verifierStorage;
    llvm::raw_string_ostream verifierOutput(verifierStorage);
    const bool isInvalid = llvm::verifyModule(*state->module, &verifierOutput);
    verifierOutput.flush();
    if (isInvalid)
    {
        if (verifierStorage.empty())
            verifierStorage = "LLVM rejected the module without a verifier diagnostic.";
        outDiagnosticData.append(verifierStorage.begin(), verifierStorage.end());
        outVerificationStatus = SLANG_NVVM_VERIFICATION_INVALID;
        return SLANG_OK;
    }

    // V1 historically verifies before rejecting an unknown serialization format.
    if (!_isSerializationFormat(format))
        return SLANG_E_INVALID_ARG;

    outDiagnosticData.clear();
    llvm::raw_svector_ostream serializedOutput(outSerializedData);
    if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
        state->module->print(serializedOutput, nullptr);
    else
        llvm::WriteBitcodeToFile(*state->module, serializedOutput);
    outVerificationStatus = SLANG_NVVM_VERIFICATION_VALID;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _serializeModule(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMSerializationFormat_1 format,
    void* destination,
    size_t destinationSize,
    size_t* outSerializedSize)
{
    ModuleState* state = _getModule(module);
    llvm::SmallVector<char, 0> serializedData;
    llvm::SmallVector<char, 0> diagnosticData;
    SlangNVVMVerificationStatus_2 verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    const SlangResult materializeResult =
        _materializeModule(state, format, serializedData, diagnosticData, verificationStatus);
    if (SLANG_FAILED(materializeResult))
        return materializeResult;
    if (verificationStatus == SLANG_NVVM_VERIFICATION_INVALID)
        return SLANG_FAIL;

    return _copySerializedData(
        llvm::StringRef(serializedData.data(), serializedData.size()),
        destination,
        destinationSize,
        outSerializedSize);
}

static SlangResult SLANG_NVVM_CALL _serializeModuleWithDiagnostics(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMSerializationFormat_1 format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus_2* outVerificationStatus)
{
    if (outSerializedSize)
        *outSerializedSize = 0;
    if (outDiagnosticSize)
        *outDiagnosticSize = 0;
    if (outVerificationStatus)
        *outVerificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;

    ModuleState* state = _getModule(module);
    if (!state || !_isSerializationFormat(format) || !outSerializedSize || !outDiagnosticSize ||
        !outVerificationStatus || (!serializedDestination && serializedDestinationSize) ||
        (!diagnosticDestination && diagnosticDestinationSize))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::SmallVector<char, 0> serializedData;
    llvm::SmallVector<char, 0> diagnosticData;
    SlangNVVMVerificationStatus_2 verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    const SlangResult materializeResult =
        _materializeModule(state, format, serializedData, diagnosticData, verificationStatus);
    if (SLANG_FAILED(materializeResult))
        return materializeResult;

    *outSerializedSize = serializedData.size();
    *outDiagnosticSize = diagnosticData.size();
    *outVerificationStatus = verificationStatus;

    const bool isQuery = !serializedDestination && !diagnosticDestination;
    if (!isQuery && ((!serializedDestination && !serializedData.empty()) ||
                     (serializedDestination && serializedDestinationSize < serializedData.size()) ||
                     (!diagnosticDestination && !diagnosticData.empty()) ||
                     (diagnosticDestination && diagnosticDestinationSize < diagnosticData.size())))
    {
        return SLANG_E_BUFFER_TOO_SMALL;
    }

    if (serializedDestination && !serializedData.empty())
        std::memcpy(serializedDestination, serializedData.data(), serializedData.size());
    if (diagnosticDestination && !diagnosticData.empty())
        std::memcpy(diagnosticDestination, diagnosticData.data(), diagnosticData.size());
    return SLANG_OK;
}

// Fills the canonical V1 table so the standalone and nested exports cannot diverge.
static void _fillBuilderAPIV1(SlangNVVMBuilderAPI_V1& api)
{
    api = {};
    api.structureSize = uint32_t(sizeof(api));
    api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_1;
    api.llvmVersionMajor = LLVM_VERSION_MAJOR;
    api.llvmVersionMinor = LLVM_VERSION_MINOR;
    api.llvmVersionPatch = LLVM_VERSION_PATCH;
    api.nvvmIRVersionMajor = 2;
    api.nvvmIRVersionMinor = 0;
    api.pointerModel = SLANG_NVVM_POINTER_MODEL_TYPED;
    api.createModule = _createModule;
    api.destroyModule = _destroyModule;
    api.getVoidType = _getVoidType;
    api.getFunctionType = _getFunctionType;
    api.declareFunction = _declareFunction;
    api.createBlock = _createBlock;
    api.setInsertBlock = _setInsertBlock;
    api.emitReturnVoid = _emitReturnVoid;
    api.markFunctionAsKernel = _markFunctionAsKernel;
    api.serializeModule = _serializeModule;
}

} // namespace

extern "C" SLANG_NVVM_BUILDER_API SlangResult SLANG_NVVM_CALL
slang_getNVVMBuilderAPI_V1(SlangNVVMBuilderAPI_V1* outAPI)
{
    if (!outAPI || outAPI->structureSize != sizeof(SlangNVVMBuilderAPI_V1) ||
        outAPI->abiVersion != SLANG_NVVM_BUILDER_ABI_VERSION_1)
    {
        return SLANG_E_NO_INTERFACE;
    }

    SlangNVVMBuilderAPI_V1 api;
    _fillBuilderAPIV1(api);
    *outAPI = api;
    return SLANG_OK;
}

extern "C" SLANG_NVVM_BUILDER_API SlangResult SLANG_NVVM_CALL
slang_getNVVMBuilderAPI_V2(SlangNVVMBuilderAPI_V2* outAPI)
{
    if (!outAPI || outAPI->structureSize < SLANG_NVVM_BUILDER_API_V2_MIN_SIZE ||
        outAPI->abiVersion != SLANG_NVVM_BUILDER_ABI_VERSION_2)
    {
        return SLANG_E_NO_INTERFACE;
    }

    const size_t callerCapacity = outAPI->structureSize;
    SlangNVVMBuilderAPI_V2 api = {};
    api.structureSize = uint32_t(sizeof(api));
    api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_2;
    _fillBuilderAPIV1(api.baseAPI);
    api.serializeModuleWithDiagnostics = _serializeModuleWithDiagnostics;
    api.getIntegerType = _getIntegerType;
    api.getPointerType = _getPointerType;
    api.getFunctionParameter = _getFunctionParameter;
    api.emitLoad = _emitLoad;
    api.emitStore = _emitStore;

    const size_t copySize = callerCapacity < sizeof(api) ? callerCapacity : sizeof(api);
    std::memcpy(outAPI, &api, copySize);
    return SLANG_OK;
}
