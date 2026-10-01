#include "compiler-core/slang-nvvm-ir-builder-api.h"
#include "compiler-core/slang-nvvm-semantic-catalog.h"
#include "slang.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsNVPTX.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/MemoryBuffer.h"
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

// Input-library state is independent of every output ModuleState. Parse once; queries never
// insert declarations or IR into this module, and no vendor program is involved in its lifetime.
struct DeviceLibraryState
{
    llvm::LLVMContext context;
    std::unique_ptr<llvm::MemoryBuffer> bytes;
    std::unique_ptr<llvm::Module> module;
};

static SlangResult SLANG_NVVM_CALL _loadDeviceLibrary(
    const void* bytes,
    size_t byteCount,
    SlangNVVMDeviceLibraryHandle* outLibrary,
    SlangNVVMDiagnosticCallback diagnose,
    void* userData)
{
    if (outLibrary)
        *outLibrary = nullptr;
    if (!outLibrary || (!bytes && byteCount))
        return SLANG_E_INVALID_ARG;
    auto state = std::make_unique<DeviceLibraryState>();
    state->bytes = llvm::MemoryBuffer::getMemBufferCopy(
        llvm::StringRef(static_cast<const char*>(bytes), byteCount),
        "selected-libdevice");
    auto parsed = llvm::parseBitcodeFile(state->bytes->getMemBufferRef(), state->context);
    if (!parsed)
    {
        const auto message = llvm::toString(parsed.takeError());
        if (diagnose)
            diagnose(userData, message.data(), message.size());
        return SLANG_FAIL;
    }
    state->module = std::move(*parsed);
    std::string diagnostics;
    llvm::raw_string_ostream stream(diagnostics);
    if (llvm::verifyModule(*state->module, &stream))
    {
        stream.flush();
        if (diagnose)
            diagnose(userData, diagnostics.data(), diagnostics.size());
        return SLANG_FAIL;
    }
    *outLibrary = reinterpret_cast<SlangNVVMDeviceLibraryHandle>(state.release());
    return SLANG_OK;
}

static void SLANG_NVVM_CALL _destroyDeviceLibrary(SlangNVVMDeviceLibraryHandle library)
{
    delete reinterpret_cast<DeviceLibraryState*>(library);
}

static ModuleState* _getModule(SlangNVVMModuleHandle module)
{
    return reinterpret_cast<ModuleState*>(module);
}

static llvm::Type* _getType(SlangNVVMTypeHandle type)
{
    return reinterpret_cast<llvm::Type*>(type);
}

static llvm::Value* _getValue(SlangNVVMValueHandle value)
{
    return reinterpret_cast<llvm::Value*>(value);
}

static llvm::BasicBlock* _getBlock(SlangNVVMBlockHandle block)
{
    return reinterpret_cast<llvm::BasicBlock*>(block);
}

static llvm::StringRef _getStringRef(const char* data, size_t size)
{
    return size ? llvm::StringRef(data, size) : llvm::StringRef();
}

static bool _isNVVMAddressSpace(SlangNVVMAddressSpace addressSpace)
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

static llvm::BasicBlock* _getValidInsertionBlock(ModuleState* state)
{
    if (!state)
        return nullptr;

    llvm::BasicBlock* block = state->builder.GetInsertBlock();
    if (!block || block->getTerminator() || !block->getParent() ||
        block->getParent()->getParent() != state->module.get())
    {
        return nullptr;
    }
    return block;
}

// Checks module/context ownership shared by ordinary uses and phi incoming-edge uses.
static bool _isValueUsableInFunction(
    ModuleState* state,
    llvm::Function* function,
    llvm::Value* value)
{
    if (!state || !function || !value || function->getParent() != state->module.get() ||
        &value->getContext() != &state->context)
    {
        return false;
    }

    // Each ModuleState has its own LLVMContext. A context-owned constant is therefore usable by
    // this module, while a GlobalValue must additionally still be attached to this exact module.
    if (llvm::GlobalValue* globalValue = llvm::dyn_cast<llvm::GlobalValue>(value))
        return globalValue->getParent() == state->module.get();
    if (llvm::BlockAddress* blockAddress = llvm::dyn_cast<llvm::BlockAddress>(value))
        return blockAddress->getFunction()->getParent() == state->module.get();
    if (llvm::isa<llvm::Constant>(value))
        return true;

    if (llvm::Argument* argument = llvm::dyn_cast<llvm::Argument>(value))
        return argument->getParent() == function;
    if (llvm::Instruction* instruction = llvm::dyn_cast<llvm::Instruction>(value))
        return instruction->getFunction() == function;
    return false;
}

// Checks whether LLVM permits using a value at the builder's current insertion point.
static bool _isValueUsableAtInsertionPoint(
    ModuleState* state,
    llvm::BasicBlock* insertionBlock,
    llvm::Value* value)
{
    if (!state || !insertionBlock || !value)
        return false;

    llvm::Function* function = insertionBlock->getParent();
    if (!_isValueUsableInFunction(state, function, value))
        return false;

    llvm::Instruction* instruction = llvm::dyn_cast<llvm::Instruction>(value);
    if (!instruction)
        return true;

    if (instruction->getParent() == insertionBlock)
    {
        const llvm::BasicBlock::iterator insertionPoint = state->builder.GetInsertPoint();
        return insertionPoint == insertionBlock->end() ||
               instruction->comesBefore(&*insertionPoint);
    }

    llvm::DominatorTree dominatorTree(*function);
    return dominatorTree.dominates(instruction, insertionBlock);
}

// Checks whether a scalar integer value is usable at the current insertion point.
static bool _isIntegerValueUsableAtInsertionPoint(
    ModuleState* state,
    llvm::BasicBlock* insertionBlock,
    llvm::Value* value)
{
    return _isValueUsableAtInsertionPoint(state, insertionBlock, value) &&
           llvm::isa<llvm::IntegerType>(value->getType());
}

// Validates the common ownership and type contract before binary integer instruction creation.
static bool _areMatchingIntegerValues(
    ModuleState* state,
    llvm::BasicBlock* insertionBlock,
    llvm::Value* left,
    llvm::Value* right)
{
    return _isIntegerValueUsableAtInsertionPoint(state, insertionBlock, left) &&
           _isIntegerValueUsableAtInsertionPoint(state, insertionBlock, right) &&
           left->getType() == right->getType();
}

// Checks that every block has its final CFG successors before edge dominance is queried.
static bool _hasCompleteCFG(llvm::Function* function)
{
    if (!function)
        return false;

    for (llvm::BasicBlock& block : *function)
    {
        if (!block.getTerminator())
            return false;
    }
    return true;
}

// Checks whether a value is available on the outgoing edge of a terminated predecessor.
static bool _isValueUsableOnIncomingEdge(
    ModuleState* state,
    llvm::Function* function,
    llvm::BasicBlock* predecessor,
    llvm::Value* value)
{
    if (!predecessor || predecessor->getParent() != function ||
        !_isValueUsableInFunction(state, function, value))
    {
        return false;
    }

    llvm::Instruction* instruction = llvm::dyn_cast<llvm::Instruction>(value);
    if (!instruction)
        return true;

    llvm::Instruction* predecessorTerminator = predecessor->getTerminator();
    if (!predecessorTerminator)
        return false;

    llvm::DominatorTree dominatorTree(*function);
    return dominatorTree.dominates(instruction, predecessorTerminator);
}

static bool _isValidAlignment(uint32_t alignment)
{
    return llvm::isPowerOf2_32(alignment);
}

static SlangResult SLANG_NVVM_CALL
_createModule(const char* moduleName, size_t moduleNameSize, SlangNVVMModuleHandle* outModule)
{
    if (!outModule || (!moduleName && moduleNameSize))
        return SLANG_E_INVALID_ARG;

    *outModule = nullptr;
    ModuleState* state = new (std::nothrow) ModuleState(_getStringRef(moduleName, moduleNameSize));
    if (!state)
        return SLANG_E_OUT_OF_MEMORY;

    *outModule = reinterpret_cast<SlangNVVMModuleHandle>(state);
    return SLANG_OK;
}

static void SLANG_NVVM_CALL _destroyModule(SlangNVVMModuleHandle module)
{
    delete _getModule(module);
}

static SlangResult SLANG_NVVM_CALL
_getVoidType(SlangNVVMModuleHandle module, SlangNVVMTypeHandle* outType)
{
    ModuleState* state = _getModule(module);
    if (!state || !outType)
        return SLANG_E_INVALID_ARG;

    *outType = reinterpret_cast<SlangNVVMTypeHandle>(llvm::Type::getVoidTy(state->context));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_getIntegerType(SlangNVVMModuleHandle module, uint32_t bitWidth, SlangNVVMTypeHandle* outType)
{
    if (outType)
        *outType = nullptr;

    ModuleState* state = _getModule(module);
    if (!state || !outType || bitWidth == 0 || bitWidth > uint32_t(llvm::IntegerType::MAX_INT_BITS))
    {
        return SLANG_E_INVALID_ARG;
    }

    *outType =
        reinterpret_cast<SlangNVVMTypeHandle>(llvm::IntegerType::get(state->context, bitWidth));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_getFloatingPointType(SlangNVVMModuleHandle module, uint32_t bitWidth, SlangNVVMTypeHandle* outType)
{
    if (outType)
        *outType = nullptr;

    ModuleState* state = _getModule(module);
    if (!state || !outType || (bitWidth != 16 && bitWidth != 32 && bitWidth != 64))
        return SLANG_E_INVALID_ARG;

    *outType = reinterpret_cast<SlangNVVMTypeHandle>(
        bitWidth == 16   ? llvm::Type::getHalfTy(state->context)
        : bitWidth == 32 ? llvm::Type::getFloatTy(state->context)
                         : llvm::Type::getDoubleTy(state->context));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getPointerType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle pointeeType,
    SlangNVVMAddressSpace addressSpace,
    SlangNVVMTypeHandle* outType)
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

    *outType = reinterpret_cast<SlangNVVMTypeHandle>(
        llvm::PointerType::get(llvmPointeeType, addressSpace));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getArrayType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle* outType)
{
    if (outType)
        *outType = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Type* llvmElementType = _getType(elementType);
    if (!state || !llvmElementType || &llvmElementType->getContext() != &state->context ||
        !llvm::ArrayType::isValidElementType(llvmElementType) ||
        !llvm::PointerType::isLoadableOrStorableType(llvmElementType) ||
        !llvmElementType->isSized() || !elementCount || !outType)
    {
        return SLANG_E_INVALID_ARG;
    }

    *outType =
        reinterpret_cast<SlangNVVMTypeHandle>(llvm::ArrayType::get(llvmElementType, elementCount));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getVectorType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle* outType)
{
    if (outType)
        *outType = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Type* llvmElementType = _getType(elementType);
    if (!state || !llvmElementType || &llvmElementType->getContext() != &state->context ||
        !llvm::VectorType::isValidElementType(llvmElementType) || elementCount < 2 ||
        elementCount > 4 || !outType)
    {
        return SLANG_E_INVALID_ARG;
    }

    *outType = reinterpret_cast<SlangNVVMTypeHandle>(
        llvm::FixedVectorType::get(llvmElementType, elementCount));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getStructType(
    SlangNVVMModuleHandle module,
    const SlangNVVMTypeHandle* fieldTypes,
    size_t fieldCount,
    SlangNVVMTypeHandle* outType)
{
    if (outType)
        *outType = nullptr;

    ModuleState* state = _getModule(module);
    if (!state || (!fieldTypes && fieldCount) || !outType)
        return SLANG_E_INVALID_ARG;

    llvm::SmallVector<llvm::Type*, 8> llvmFieldTypes;
    llvmFieldTypes.reserve(fieldCount);
    for (size_t i = 0; i < fieldCount; ++i)
    {
        llvm::Type* fieldType = _getType(fieldTypes[i]);
        if (!fieldType || &fieldType->getContext() != &state->context ||
            !llvm::StructType::isValidElementType(fieldType))
        {
            return SLANG_E_INVALID_ARG;
        }
        llvmFieldTypes.push_back(fieldType);
    }

    *outType = reinterpret_cast<SlangNVVMTypeHandle>(
        llvm::StructType::get(state->context, llvmFieldTypes, false));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getFunctionType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle resultType,
    const SlangNVVMTypeHandle* parameterTypes,
    size_t parameterCount,
    SlangNVVMTypeHandle* outType)
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

    *outType = reinterpret_cast<SlangNVVMTypeHandle>(
        llvm::FunctionType::get(llvmResultType, llvmParameterTypes, false));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _declareFunction(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle functionType,
    SlangNVVMLinkage linkage,
    SlangNVVMFunctionFlags flags,
    const char* name,
    size_t nameSize,
    SlangNVVMValueHandle* outFunction)
{
    ModuleState* state = _getModule(module);
    llvm::FunctionType* llvmFunctionType =
        llvm::dyn_cast_or_null<llvm::FunctionType>(_getType(functionType));
    if (!state || !llvmFunctionType || &llvmFunctionType->getContext() != &state->context ||
        (linkage != SLANG_NVVM_LINKAGE_INTERNAL && linkage != SLANG_NVVM_LINKAGE_EXTERNAL) ||
        (flags & ~SLANG_NVVM_FUNCTION_FLAG_NO_INLINE) || !name || !nameSize || !outFunction)
        return SLANG_E_INVALID_ARG;

    const llvm::StringRef llvmName = _getStringRef(name, nameSize);
    if (state->module->getNamedValue(llvmName))
        return SLANG_E_INVALID_ARG;

    llvm::Function* function = llvm::Function::Create(
        llvmFunctionType,
        linkage == SLANG_NVVM_LINKAGE_INTERNAL ? llvm::GlobalValue::InternalLinkage
                                               : llvm::GlobalValue::ExternalLinkage,
        llvmName,
        *state->module);
    if (flags & SLANG_NVVM_FUNCTION_FLAG_NO_INLINE)
        function->addFnAttr(llvm::Attribute::NoInline);
    size_t parameterIndex = 0;
    for (llvm::Argument& parameter : function->args())
    {
        // LLVM 14 prints an unnamed numeric parameter as an explicit `%0` declaration. LLVM 7
        // accepts numeric parameter slots only when they are implicit, while accepting ordinary
        // named parameters. Stable provider-owned names keep the typed module and its textual
        // representation valid in both dialects without parsing a function signature later.
        parameter.setName("slangParameter" + std::to_string(parameterIndex++));
    }
    *outFunction = reinterpret_cast<SlangNVVMValueHandle>(function);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getFunctionParameter(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function,
    size_t parameterIndex,
    SlangNVVMValueHandle* outValue)
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

    *outValue = reinterpret_cast<SlangNVVMValueHandle>(
        llvmFunction->getArg(static_cast<unsigned>(parameterIndex)));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _setFunctionParameterAttributes(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function,
    size_t parameterIndex,
    SlangNVVMParameterFlags flags,
    SlangNVVMTypeHandle pointeeType,
    uint32_t alignment)
{
    ModuleState* state = _getModule(module);
    llvm::Function* llvmFunction = llvm::dyn_cast_or_null<llvm::Function>(_getValue(function));
    llvm::Type* llvmPointeeType = _getType(pointeeType);
    if (!state || !llvmFunction || llvmFunction->getParent() != state->module.get() ||
        parameterIndex >= llvmFunction->arg_size() || (flags & ~SLANG_NVVM_PARAMETER_FLAG_BY_VALUE))
    {
        return SLANG_E_INVALID_ARG;
    }

    if (flags == SLANG_NVVM_PARAMETER_FLAG_NONE)
        return !pointeeType && !alignment ? SLANG_OK : SLANG_E_INVALID_ARG;

    llvm::Argument* argument = llvmFunction->getArg(static_cast<unsigned>(parameterIndex));
    auto pointerType = llvm::dyn_cast<llvm::PointerType>(argument->getType());
    if (flags != SLANG_NVVM_PARAMETER_FLAG_BY_VALUE || !llvmPointeeType ||
        &llvmPointeeType->getContext() != &state->context || !llvmPointeeType->isSized() ||
        !pointerType || pointerType->isOpaque() ||
        pointerType->getPointerElementType() != llvmPointeeType || !alignment ||
        !llvm::isPowerOf2_32(alignment) || alignment > llvm::Value::MaximumAlignment ||
        llvmFunction->getAttributes()
                .getParamAttrs(static_cast<unsigned>(parameterIndex))
                .getNumAttributes() != 0)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvmFunction->addParamAttr(
        static_cast<unsigned>(parameterIndex),
        llvm::Attribute::getWithByValType(state->context, llvmPointeeType));
    llvmFunction->addParamAttr(
        static_cast<unsigned>(parameterIndex),
        llvm::Attribute::getWithAlignment(state->context, llvm::Align(alignment)));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _createBlock(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function,
    const char* name,
    size_t nameSize,
    SlangNVVMBlockHandle* outBlock)
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
    *outBlock = reinterpret_cast<SlangNVVMBlockHandle>(block);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_setInsertBlock(SlangNVVMModuleHandle module, SlangNVVMBlockHandle block)
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
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle pointer,
    uint32_t alignment,
    SlangNVVMLoadFlags flags,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmPointer = _getValue(pointer);
    llvm::PointerType* pointerType = _getLoadablePointerType(state, llvmPointer);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!pointerType || !insertionBlock ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmPointer) ||
        !_isValidAlignment(alignment) ||
        (flags & ~SLANG_NVVM_LOAD_FLAG_INVARIANT) != SLANG_NVVM_LOAD_FLAG_NONE || !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Type* pointeeType = pointerType->getNonOpaquePointerElementType();
    llvm::LoadInst* value =
        state->builder.CreateAlignedLoad(pointeeType, llvmPointer, llvm::Align(alignment));
    if (flags & SLANG_NVVM_LOAD_FLAG_INVARIANT)
    {
        value->setMetadata(
            llvm::LLVMContext::MD_invariant_load,
            llvm::MDNode::get(state->context, {}));
    }
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(value);
    return SLANG_OK;
}

// Returns whether a stored aggregate contains a struct with an immediate struct field.
// For example, [3 x { i16, { i16, i32 } }] qualifies, while [3 x { i16, i32 }] does not.
// Array elements are inspected once regardless of the array length; pointer pointees are not
// part of the stored value. The terminal-store caller has already split direct struct fields,
// so any remaining nested boundary is hidden inside an array.
static bool _containsNestedStructLayout(llvm::Type* type)
{
    if (auto arrayType = llvm::dyn_cast<llvm::ArrayType>(type))
        return _containsNestedStructLayout(arrayType->getElementType());
    if (auto structType = llvm::dyn_cast<llvm::StructType>(type))
    {
        for (auto fieldType : structType->elements())
        {
            if (fieldType->isStructTy() || _containsNestedStructLayout(fieldType))
                return true;
        }
    }
    return false;
}

// Preserves nested struct padding when libNVVM lowers aggregate stores. Consider these LLVM types:
//
//     Inner = { i8, i32 }
//     Outer = { i8, Inner }
//
// The two i8 fields occupy offsets zero and four. The installed libNVVM optimizer can combine
// a whole Outer store into adjacent byte stores, losing that nested boundary. Select fields with
// LLVM's canonical type and layout before handing the operation to libNVVM. Arrays retain one
// whole store, so their size never expands the provider-emitted instruction count.
static void _emitStorePreservingNestedStructLayout(
    ModuleState* state,
    llvm::Value* value,
    llvm::Value* pointer,
    llvm::Align alignment)
{
    auto structType = llvm::dyn_cast<llvm::StructType>(value->getType());
    bool hasStructField = false;
    if (structType)
    {
        for (auto fieldType : structType->elements())
        {
            if (fieldType->isStructTy())
            {
                hasStructField = true;
                break;
            }
        }
    }
    if (!hasStructField)
    {
        // Consider an array of Outer from the example above. Its whole SSA value and physical
        // layout are already correct, but libNVVM can lose the nested padding when lowering an
        // aligned aggregate store. A one-byte guarantee preserves that padding without expanding
        // the array or rereading a source pointer that may have changed since the value was loaded.
        if (_containsNestedStructLayout(value->getType()))
            alignment = llvm::Align(1);
        state->builder.CreateAlignedStore(value, pointer, alignment);
        return;
    }

    const llvm::StructLayout* layout = state->module->getDataLayout().getStructLayout(structType);
    for (unsigned i = 0; i < structType->getNumElements(); ++i)
    {
        llvm::Value* fieldValue = state->builder.CreateExtractValue(value, i);
        llvm::Value* fieldPointer = state->builder.CreateStructGEP(structType, pointer, i);
        // The caller guarantees the root alignment, not the child's nominal ABI alignment.
        // Retain only the guarantee that survives this exact offset, including underaligned roots.
        _emitStorePreservingNestedStructLayout(
            state,
            fieldValue,
            fieldPointer,
            llvm::commonAlignment(alignment, layout->getElementOffset(i)));
    }
}

static SlangResult SLANG_NVVM_CALL _emitStore(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle pointer,
    uint32_t alignment)
{
    ModuleState* state = _getModule(module);
    llvm::Value* llvmValue = _getValue(value);
    llvm::Value* llvmPointer = _getValue(pointer);
    llvm::PointerType* pointerType = _getLoadablePointerType(state, llvmPointer);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!pointerType || !llvmValue || &llvmValue->getContext() != &state->context ||
        llvmValue->getType() != pointerType->getNonOpaquePointerElementType() ||
        pointerType->getAddressSpace() == SLANG_NVVM_ADDRESS_SPACE_CONSTANT || !insertionBlock ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmValue) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmPointer) ||
        !_isValidAlignment(alignment))
    {
        return SLANG_E_INVALID_ARG;
    }

    _emitStorePreservingNestedStructLayout(state, llvmValue, llvmPointer, llvm::Align(alignment));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitLocalStorage(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle valueType,
    uint32_t alignment,
    const char* name,
    size_t nameSize,
    SlangNVVMValueHandle* outStorage)
{
    if (outStorage)
        *outStorage = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Type* llvmValueType = _getType(valueType);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::Function* function = insertionBlock ? insertionBlock->getParent() : nullptr;
    if (!state || !llvmValueType || &llvmValueType->getContext() != &state->context ||
        !llvm::PointerType::isLoadableOrStorableType(llvmValueType) || !llvmValueType->isSized() ||
        !function || function->getParent() != state->module.get() ||
        !_isValidAlignment(alignment) || (!name && nameSize) || !outStorage)
    {
        return SLANG_E_INVALID_ARG;
    }

    // A source local denotes one allocation per function activation even when its canonical IR
    // instruction is physically inside a loop block. Keep every fixed-size allocation in the
    // entry block and let ordinary dominance make the resulting pointer usable everywhere.
    llvm::IRBuilder<> entryBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    llvm::AllocaInst* storage =
        entryBuilder.CreateAlloca(llvmValueType, nullptr, _getStringRef(name, nameSize));
    storage->setAlignment(llvm::Align(alignment));
    *outStorage = reinterpret_cast<SlangNVVMValueHandle>(storage);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBinary(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmLeft = _getValue(left);
    llvm::Value* llvmRight = _getValue(right);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    const bool isSupportedOperation =
        operation == SLANG_NVVM_VALUE_OP_ADD || operation == SLANG_NVVM_VALUE_OP_SUBTRACT;
    if (!outValue || !isSupportedOperation || !insertionBlock ||
        !_areMatchingIntegerValues(state, insertionBlock, llvmLeft, llvmRight))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = operation == SLANG_NVVM_VALUE_OP_ADD
                              ? state->builder.CreateAdd(llvmLeft, llvmRight)
                              : state->builder.CreateSub(llvmLeft, llvmRight);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerMultiply(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmLeft = _getValue(left);
    llvm::Value* llvmRight = _getValue(right);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock ||
        !_areMatchingIntegerValues(state, insertionBlock, llvmLeft, llvmRight))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = state->builder.CreateMul(llvmLeft, llvmRight);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBitAnd(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmLeft = _getValue(left);
    llvm::Value* llvmRight = _getValue(right);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock ||
        !_areMatchingIntegerValues(state, insertionBlock, llvmLeft, llvmRight))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = state->builder.CreateAnd(llvmLeft, llvmRight);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBitOr(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmLeft = _getValue(left);
    llvm::Value* llvmRight = _getValue(right);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock ||
        !_areMatchingIntegerValues(state, insertionBlock, llvmLeft, llvmRight))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = state->builder.CreateOr(llvmLeft, llvmRight);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBitXor(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmLeft = _getValue(left);
    llvm::Value* llvmRight = _getValue(right);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock ||
        !_areMatchingIntegerValues(state, insertionBlock, llvmLeft, llvmRight))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = state->builder.CreateXor(llvmLeft, llvmRight);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBitNot(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmValue = _getValue(value);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock ||
        !_isIntegerValueUsableAtInsertionPoint(state, insertionBlock, llvmValue))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = state->builder.CreateNot(llvmValue);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerNegate(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmValue = _getValue(value);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock ||
        !_isIntegerValueUsableAtInsertionPoint(state, insertionBlock, llvmValue))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = state->builder.CreateNeg(llvmValue);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitAtomicOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMAtomicOperationDesc* operation,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    const size_t expectedOperandCount =
        operation && operation->operation == SLANG_NVVM_ATOMIC_OP_LOAD               ? 1
        : operation && operation->operation == SLANG_NVVM_ATOMIC_OP_COMPARE_EXCHANGE ? 3
                                                                                     : 2;
    llvm::Value* llvmPointer = operands && operandCount ? _getValue(operands[0]) : nullptr;
    llvm::PointerType* pointerType = _getLoadablePointerType(state, llvmPointer);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::Type* pointeeType = pointerType ? pointerType->getNonOpaquePointerElementType() : nullptr;
    const bool hasExpectedIntegerType =
        pointeeType &&
        (operation && (operation->valueType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                       operation->valueType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER)) &&
        pointeeType->isIntegerTy(operation->valueType.bitWidth);
    const bool hasExpectedFloatingType =
        pointeeType && operation &&
        operation->valueType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
        pointeeType->isFloatingPointTy() &&
        pointeeType->getScalarSizeInBits() == operation->valueType.bitWidth;
    auto pointeeVectorType = llvm::dyn_cast_or_null<llvm::FixedVectorType>(pointeeType);
    const bool hasExpectedHalf2Type =
        pointeeVectorType && operation &&
        operation->valueType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
        operation->valueType.bitWidth == 16 && operation->valueType.laneCount == 2 &&
        pointeeVectorType->getNumElements() == 2 && pointeeVectorType->getElementType()->isHalfTy();
    if (!operation || !Slang::NVVMSemantics::isSupported(*operation) || !outValue || !operands ||
        operandCount != expectedOperandCount || !pointerType ||
        pointerType->getAddressSpace() != operation->addressSpace || !pointeeType ||
        (!hasExpectedIntegerType && !hasExpectedFloatingType && !hasExpectedHalf2Type) ||
        !insertionBlock || !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmPointer))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* llvmValues[2] = {};
    for (size_t i = 1; i < operandCount; ++i)
    {
        llvmValues[i - 1] = _getValue(operands[i]);
        if (!llvmValues[i - 1] || llvmValues[i - 1]->getType() != pointeeType ||
            !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmValues[i - 1]))
        {
            return SLANG_E_INVALID_ARG;
        }
    }

    const llvm::Align alignment(operation->valueType.bitWidth * operation->valueType.laneCount / 8);
    const llvm::AtomicOrdering ordering = llvm::AtomicOrdering::Monotonic;
    const llvm::SyncScope::ID syncScope = llvm::SyncScope::System;

    // LLVM and libNVVM spell bitwise floating-point memory operations through an integer pointer
    // of the same width. The operation preserves every source bit and the final bitcast restores
    // the exact canonical floating result.
    llvm::Value* atomicPointer = llvmPointer;
    llvm::Value* atomicValues[2] = {llvmValues[0], llvmValues[1]};
    llvm::Type* atomicType = pointeeType;
    if (hasExpectedFloatingType && operation->operation != SLANG_NVVM_ATOMIC_OP_ADD)
    {
        atomicType = llvm::Type::getIntNTy(state->context, operation->valueType.bitWidth);
        auto atomicPointerType = llvm::PointerType::get(atomicType, operation->addressSpace);
        atomicPointer = state->builder.CreateBitCast(llvmPointer, atomicPointerType);
        for (size_t i = 0; i + 1 < operandCount; ++i)
            atomicValues[i] = state->builder.CreateBitCast(llvmValues[i], atomicType);
    }

    llvm::Value* originalValue = nullptr;
    if (hasExpectedHalf2Type && operation->operation == SLANG_NVVM_ATOMIC_OP_ADD)
    {
        // CUDA's packed Half2 atomic add crosses the PTX boundary as one 32-bit register. The
        // descriptor and LLVM values remain typed as <2 x half>; these bitcasts only establish the
        // exact register and pointer types required by the inline-assembly constraints.
        llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
        llvm::PointerType* int32PointerType =
            llvm::PointerType::get(int32Type, SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
        llvm::Value* integerPointer = state->builder.CreateBitCast(llvmPointer, int32PointerType);
        llvm::Value* integerValue = state->builder.CreateBitCast(llvmValues[0], int32Type);
        llvm::FunctionType* functionType =
            llvm::FunctionType::get(int32Type, {int32PointerType, int32Type}, false);
        llvm::InlineAsm* inlineAsm = llvm::InlineAsm::get(
            functionType,
            "atom.global.add.noftz.f16x2 $0, [$1], $2;",
            "=r,l,r",
            true);
        llvm::Value* resultBits =
            state->builder.CreateCall(inlineAsm, {integerPointer, integerValue});
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(
            state->builder.CreateBitCast(resultBits, pointeeType));
        return SLANG_OK;
    }
    if (hasExpectedFloatingType && operation->valueType.bitWidth == 16 &&
        operation->operation == SLANG_NVVM_ATOMIC_OP_ADD)
    {
        // NVVM IR's legacy floating atomic-add intrinsics cover only Float32 and Float64, while
        // CUDA's canonical scalar Half operation is the PTX `atom.add.noftz.f16` instruction.
        // Keep the provider input typed as Half, then cross the PTX inline-assembly boundary using
        // the exact i16 register bits accepted by LLVM's `h` constraint. The pointer stays in the
        // already-validated global address space and the returned bits restore the Half result.
        llvm::Type* int16Type = llvm::Type::getInt16Ty(state->context);
        llvm::PointerType* int16PointerType =
            llvm::PointerType::get(int16Type, SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
        llvm::Value* integerPointer = state->builder.CreateBitCast(llvmPointer, int16PointerType);
        llvm::Value* integerValue = state->builder.CreateBitCast(llvmValues[0], int16Type);
        llvm::FunctionType* functionType =
            llvm::FunctionType::get(int16Type, {int16PointerType, int16Type}, false);
        llvm::InlineAsm* inlineAsm = llvm::InlineAsm::get(
            functionType,
            "atom.global.add.noftz.f16 $0, [$1], $2;",
            "=h,l,h",
            true);
        llvm::Value* resultBits =
            state->builder.CreateCall(inlineAsm, {integerPointer, integerValue});
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(
            state->builder.CreateBitCast(resultBits, pointeeType));
        return SLANG_OK;
    }
    if (operation->operation == SLANG_NVVM_ATOMIC_OP_LOAD)
    {
        // libNVVM 2.0 rejects LLVM atomic load/store instructions. A relaxed compare-exchange of
        // zero with zero is the established value-preserving atomic-read idiom: it returns every
        // old bit pattern and can write only the identical zero bits already in memory.
        llvm::Constant* zero = llvm::Constant::getNullValue(atomicType);
        llvm::AtomicCmpXchgInst* compareExchange = state->builder.CreateAtomicCmpXchg(
            atomicPointer,
            zero,
            zero,
            alignment,
            ordering,
            ordering,
            syncScope);
        originalValue = state->builder.CreateExtractValue(compareExchange, 0);
    }
    else if (operation->operation == SLANG_NVVM_ATOMIC_OP_STORE)
    {
        // A fetch-exchange has the exact relaxed atomic-store effect. Its unused old SSA value is
        // not exposed, and no unsupported atomic store instruction reaches libNVVM.
        state->builder.CreateAtomicRMW(
            llvm::AtomicRMWInst::Xchg,
            atomicPointer,
            atomicValues[0],
            alignment,
            ordering,
            syncScope);
        return SLANG_OK;
    }
    else if (operation->operation == SLANG_NVVM_ATOMIC_OP_COMPARE_EXCHANGE)
    {
        llvm::AtomicCmpXchgInst* compareExchange = state->builder.CreateAtomicCmpXchg(
            atomicPointer,
            atomicValues[0],
            atomicValues[1],
            alignment,
            ordering,
            ordering,
            syncScope);
        originalValue = state->builder.CreateExtractValue(compareExchange, 0);
    }
    else
    {
        llvm::AtomicRMWInst::BinOp llvmOperation = llvm::AtomicRMWInst::BAD_BINOP;
        switch (operation->operation)
        {
        case SLANG_NVVM_ATOMIC_OP_ADD:
            llvmOperation =
                hasExpectedFloatingType ? llvm::AtomicRMWInst::FAdd : llvm::AtomicRMWInst::Add;
            break;
        case SLANG_NVVM_ATOMIC_OP_BIT_AND:
            llvmOperation = llvm::AtomicRMWInst::And;
            break;
        case SLANG_NVVM_ATOMIC_OP_BIT_OR:
            llvmOperation = llvm::AtomicRMWInst::Or;
            break;
        case SLANG_NVVM_ATOMIC_OP_BIT_XOR:
            llvmOperation = llvm::AtomicRMWInst::Xor;
            break;
        case SLANG_NVVM_ATOMIC_OP_MIN:
            llvmOperation = operation->valueType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                                ? llvm::AtomicRMWInst::Min
                                : llvm::AtomicRMWInst::UMin;
            break;
        case SLANG_NVVM_ATOMIC_OP_MAX:
            llvmOperation = operation->valueType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                                ? llvm::AtomicRMWInst::Max
                                : llvm::AtomicRMWInst::UMax;
            break;
        case SLANG_NVVM_ATOMIC_OP_EXCHANGE:
            llvmOperation = llvm::AtomicRMWInst::Xchg;
            break;
        default:
            return SLANG_E_INVALID_ARG;
        }
        originalValue = state->builder.CreateAtomicRMW(
            llvmOperation,
            atomicPointer,
            atomicValues[0],
            alignment,
            ordering,
            syncScope);
    }
    if (atomicType != pointeeType)
        originalValue = state->builder.CreateBitCast(originalValue, pointeeType);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(originalValue);
    return SLANG_OK;
}

// Normalizes a semantic 16-bit integer before an operation observes more than its low bits.
// Consider abs(int16_t(-32768)): LLVM's wrapping i16 result is still -32768, but PTX may
// implement abs.s16 on a 32-bit datapath and retain +32768 in the register. An explicit
// widen/narrow pair restores the signed or unsigned interpretation. Keep the pair in inline
// assembly so LLVM cannot fold it to an identity before PTX lowering. Half and BF16 storage
// also use i16; callers select this operation by integer semantics, never storage type alone.
static llvm::Value* _normalizeNVVMInteger16Operand(
    ModuleState* state,
    llvm::Value* value,
    bool isSigned)
{
    auto type = value->getType();
    if (!type->getScalarType()->isIntegerTy(16))
        return value;
    if (auto vectorType = llvm::dyn_cast<llvm::FixedVectorType>(type))
    {
        llvm::Value* result = llvm::UndefValue::get(vectorType);
        for (uint32_t lane = 0; lane < vectorType->getNumElements(); ++lane)
        {
            auto element = state->builder.CreateExtractElement(value, lane);
            auto normalized = _normalizeNVVMInteger16Operand(state, element, isSigned);
            result = state->builder.CreateInsertElement(result, normalized, lane);
        }
        return result;
    }
    auto signature = llvm::FunctionType::get(type, {type}, false);
    const char* assembly = isSigned ? "{ .reg .b32 narrow_tmp; cvt.s32.s16 narrow_tmp, $1; "
                                      "cvt.s16.s32 $0, narrow_tmp; }"
                                    : "{ .reg .b32 narrow_tmp; cvt.u32.u16 narrow_tmp, $1; "
                                      "cvt.u16.u32 $0, narrow_tmp; }";
    auto normalization = llvm::InlineAsm::get(signature, assembly, "=h,h", false);
    return state->builder.CreateCall(normalization, {value});
}

// Emits one scalar-integer comparison after applying the shared ownership and dominance contract.
static SlangResult _emitIntegerComparison(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    llvm::CmpInst::Predicate predicate,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmLeft = _getValue(left);
    llvm::Value* llvmRight = _getValue(right);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock ||
        !_areMatchingIntegerValues(state, insertionBlock, llvmLeft, llvmRight))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvmLeft = _normalizeNVVMInteger16Operand(state, llvmLeft, llvm::CmpInst::isSigned(predicate));
    llvmRight =
        _normalizeNVVMInteger16Operand(state, llvmRight, llvm::CmpInst::isSigned(predicate));
    llvm::Value* result = state->builder.CreateICmp(predicate, llvmLeft, llvmRight);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerSignedLessThan(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_SLT, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_EQ, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerNotEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_NE, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerSignedGreaterThan(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_SGT, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerSignedLessEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_SLE, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerSignedGreaterEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_SGE, outValue);
}

static llvm::CmpInst::Predicate _getFloatingComparePredicate(SlangNVVMValueOperation operation)
{
    switch (operation)
    {
    case SLANG_NVVM_VALUE_OP_EQUAL:
        return llvm::CmpInst::FCMP_OEQ;
    case SLANG_NVVM_VALUE_OP_NOT_EQUAL:
        return llvm::CmpInst::FCMP_UNE;
    case SLANG_NVVM_VALUE_OP_GREATER_THAN:
        return llvm::CmpInst::FCMP_OGT;
    case SLANG_NVVM_VALUE_OP_LESS_EQUAL:
        return llvm::CmpInst::FCMP_OLE;
    case SLANG_NVVM_VALUE_OP_GREATER_EQUAL:
        return llvm::CmpInst::FCMP_OGE;
    case SLANG_NVVM_VALUE_OP_LESS_THAN:
        return llvm::CmpInst::FCMP_OLT;
    default:
        return llvm::CmpInst::BAD_FCMP_PREDICATE;
    }
}

static SlangResult SLANG_NVVM_CALL
_emitBranch(SlangNVVMModuleHandle module, SlangNVVMBlockHandle targetBlock)
{
    ModuleState* state = _getModule(module);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::BasicBlock* llvmTargetBlock = _getBlock(targetBlock);
    if (!insertionBlock || !llvmTargetBlock ||
        llvmTargetBlock->getParent() != insertionBlock->getParent())
    {
        return SLANG_E_INVALID_ARG;
    }

    state->builder.CreateBr(llvmTargetBlock);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitConditionalBranch(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle condition,
    SlangNVVMBlockHandle trueBlock,
    SlangNVVMBlockHandle falseBlock)
{
    ModuleState* state = _getModule(module);
    llvm::Value* llvmCondition = _getValue(condition);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::BasicBlock* llvmTrueBlock = _getBlock(trueBlock);
    llvm::BasicBlock* llvmFalseBlock = _getBlock(falseBlock);
    if (!insertionBlock || !llvmCondition || !llvmCondition->getType()->isIntegerTy(1) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmCondition) || !llvmTrueBlock ||
        !llvmFalseBlock || llvmTrueBlock->getParent() != insertionBlock->getParent() ||
        llvmFalseBlock->getParent() != insertionBlock->getParent())
    {
        return SLANG_E_INVALID_ARG;
    }

    state->builder.CreateCondBr(llvmCondition, llvmTrueBlock, llvmFalseBlock);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitSwitch(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle condition,
    const SlangNVVMValueHandle* caseValues,
    const SlangNVVMBlockHandle* caseBlocks,
    size_t caseCount,
    SlangNVVMBlockHandle defaultBlock)
{
    ModuleState* state = _getModule(module);
    llvm::Value* llvmCondition = _getValue(condition);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::BasicBlock* llvmDefaultBlock = _getBlock(defaultBlock);
    if (!insertionBlock || !llvmCondition || !llvmCondition->getType()->isIntegerTy() ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmCondition) ||
        !llvmDefaultBlock || llvmDefaultBlock->getParent() != insertionBlock->getParent() ||
        caseCount > UINT32_MAX || (caseCount && (!caseValues || !caseBlocks)))
    {
        return SLANG_E_INVALID_ARG;
    }

    for (size_t i = 0; i < caseCount; ++i)
    {
        llvm::ConstantInt* caseValue =
            llvm::dyn_cast_or_null<llvm::ConstantInt>(_getValue(caseValues[i]));
        llvm::BasicBlock* caseBlock = _getBlock(caseBlocks[i]);
        if (!caseValue || caseValue->getType() != llvmCondition->getType() || !caseBlock ||
            caseBlock->getParent() != insertionBlock->getParent())
        {
            return SLANG_E_INVALID_ARG;
        }
        for (size_t j = 0; j < i; ++j)
        {
            llvm::ConstantInt* previousValue =
                llvm::dyn_cast_or_null<llvm::ConstantInt>(_getValue(caseValues[j]));
            if (previousValue && previousValue->getValue() == caseValue->getValue())
                return SLANG_E_INVALID_ARG;
        }
    }

    llvm::SwitchInst* switchInst =
        state->builder.CreateSwitch(llvmCondition, llvmDefaultBlock, unsigned(caseCount));
    for (size_t i = 0; i < caseCount; ++i)
    {
        switchInst->addCase(
            llvm::cast<llvm::ConstantInt>(_getValue(caseValues[i])),
            _getBlock(caseBlocks[i]));
    }
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getIntegerConstant(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle integerType,
    int64_t value,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::IntegerType* llvmIntegerType =
        llvm::dyn_cast_or_null<llvm::IntegerType>(_getType(integerType));
    if (!state || !llvmIntegerType || &llvmIntegerType->getContext() != &state->context ||
        (!llvm::isIntN(llvmIntegerType->getBitWidth(), value) &&
         (llvmIntegerType->getBitWidth() != 1 || value != 1)) ||
        !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }

    *outValue = reinterpret_cast<SlangNVVMValueHandle>(
        llvm::ConstantInt::getSigned(llvmIntegerType, value));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getFloatingPointConstant(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle floatingPointType,
    uint32_t bitWidth,
    uint64_t bitPattern,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Type* llvmFloatingPointType = _getType(floatingPointType);
    const bool isHalf = llvmFloatingPointType && llvmFloatingPointType->isHalfTy();
    const bool isFloat = llvmFloatingPointType && llvmFloatingPointType->isFloatTy();
    const bool isDouble = llvmFloatingPointType && llvmFloatingPointType->isDoubleTy();
    if (!state || (!isHalf && !isFloat && !isDouble) ||
        (bitWidth != 16 && bitWidth != 32 && bitWidth != 64) ||
        (bitWidth < 64 && (bitPattern >> bitWidth) != 0) || isHalf != (bitWidth == 16) ||
        isFloat != (bitWidth == 32) || isDouble != (bitWidth == 64) ||
        &llvmFloatingPointType->getContext() != &state->context || !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }

    const llvm::fltSemantics& semantics = bitWidth == 16   ? llvm::APFloat::IEEEhalf()
                                          : bitWidth == 32 ? llvm::APFloat::IEEEsingle()
                                                           : llvm::APFloat::IEEEdouble();
    const llvm::APFloat value(semantics, llvm::APInt(bitWidth, bitPattern));
    *outValue =
        reinterpret_cast<SlangNVVMValueHandle>(llvm::ConstantFP::get(llvmFloatingPointType, value));
    return SLANG_OK;
}

// Returns whether a first-class value can cross the generic function and control-flow boundary.
static bool _isSupportedFunctionValueType(llvm::Type* type)
{
    if (type &&
        (type->isIntegerTy() || type->isHalfTy() || type->isFloatTy() || type->isDoubleTy()))
        return true;
    if (auto vectorType = llvm::dyn_cast_or_null<llvm::FixedVectorType>(type))
    {
        return vectorType->getNumElements() >= 2 && vectorType->getNumElements() <= 4 &&
               _isSupportedFunctionValueType(vectorType->getElementType());
    }
    if (auto pointerType = llvm::dyn_cast_or_null<llvm::PointerType>(type))
    {
        return !pointerType->isOpaque() &&
               _isNVVMAddressSpace(
                   static_cast<SlangNVVMAddressSpace>(pointerType->getAddressSpace())) &&
               _isSupportedFunctionValueType(pointerType->getNonOpaquePointerElementType());
    }
    if (auto arrayType = llvm::dyn_cast_or_null<llvm::ArrayType>(type))
    {
        return arrayType->getNumElements() > 0 &&
               _isSupportedFunctionValueType(arrayType->getElementType());
    }
    if (auto structType = llvm::dyn_cast_or_null<llvm::StructType>(type))
    {
        if (structType->getNumElements() == 0)
            return false;
        for (llvm::Type* elementType : structType->elements())
        {
            if (!_isSupportedFunctionValueType(elementType))
                return false;
        }
        return true;
    }
    return false;
}

// Returns whether a direct helper parameter has one accepted physical representation.
static bool _isSupportedFunctionParameterType(llvm::Type* type)
{
    return _isSupportedFunctionValueType(type);
}

static SlangResult SLANG_NVVM_CALL _emitPhi(
    SlangNVVMModuleHandle module,
    SlangNVVMBlockHandle targetBlock,
    SlangNVVMTypeHandle type,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::BasicBlock* llvmTargetBlock = _getBlock(targetBlock);
    llvm::Type* llvmType = _getType(type);
    const bool isSupportedType = _isSupportedFunctionValueType(llvmType);
    if (!state || !llvmTargetBlock || !llvmTargetBlock->getParent() ||
        llvmTargetBlock->getParent()->getParent() != state->module.get() || !isSupportedType ||
        &llvmType->getContext() != &state->context || !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Instruction* firstNonPhi = llvmTargetBlock->getFirstNonPHI();
    llvm::PHINode* phi = firstNonPhi ? llvm::PHINode::Create(llvmType, 0, "", firstNonPhi)
                                     : llvm::PHINode::Create(llvmType, 0, "", llvmTargetBlock);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(phi);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _addPhiIncoming(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle phi,
    SlangNVVMValueHandle value,
    SlangNVVMBlockHandle predecessorBlock)
{
    ModuleState* state = _getModule(module);
    llvm::PHINode* llvmPhi = llvm::dyn_cast_or_null<llvm::PHINode>(_getValue(phi));
    llvm::Value* llvmValue = _getValue(value);
    llvm::BasicBlock* llvmPredecessorBlock = _getBlock(predecessorBlock);
    llvm::BasicBlock* llvmPhiBlock = llvmPhi ? llvmPhi->getParent() : nullptr;
    llvm::Function* llvmFunction = llvmPhiBlock ? llvmPhiBlock->getParent() : nullptr;
    llvm::Instruction* firstNonPhi = llvmPhiBlock ? llvmPhiBlock->getFirstNonPHI() : nullptr;
    const bool isSupportedType = llvmPhi && _isSupportedFunctionValueType(llvmPhi->getType());
    if (!state || !llvmPhi || !llvmValue || !llvmPredecessorBlock || !llvmPhiBlock ||
        !llvmFunction || llvmFunction->getParent() != state->module.get() || !isSupportedType ||
        &llvmValue->getContext() != &state->context || llvmValue->getType() != llvmPhi->getType() ||
        llvmPredecessorBlock->getParent() != llvmFunction ||
        (firstNonPhi && !llvmPhi->comesBefore(firstNonPhi)) ||
        llvmPhi->getBasicBlockIndex(llvmPredecessorBlock) >= 0 || !_hasCompleteCFG(llvmFunction))
    {
        return SLANG_E_INVALID_ARG;
    }

    size_t successorEdgeCount = 0;
    for (llvm::BasicBlock* successor : llvm::successors(llvmPredecessorBlock))
    {
        if (successor == llvmPhiBlock)
            ++successorEdgeCount;
    }
    if (successorEdgeCount != 1 ||
        !_isValueUsableOnIncomingEdge(state, llvmFunction, llvmPredecessorBlock, llvmValue))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvmPhi->addIncoming(llvmValue, llvmPredecessorBlock);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitCall(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle callee,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Function* llvmCallee = llvm::dyn_cast_or_null<llvm::Function>(_getValue(callee));
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::FunctionType* functionType = llvmCallee ? llvmCallee->getFunctionType() : nullptr;
    if (!state || !llvmCallee || llvmCallee->getParent() != state->module.get() ||
        !insertionBlock || !functionType || functionType->isVarArg() ||
        !(functionType->getReturnType()->isVoidTy() ||
          _isSupportedFunctionValueType(functionType->getReturnType())) ||
        functionType->getNumParams() != argumentCount || (!arguments && argumentCount) || !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::SmallVector<llvm::Value*, 8> llvmArguments;
    llvmArguments.reserve(argumentCount);
    for (size_t i = 0; i < argumentCount; ++i)
    {
        llvm::Type* parameterType = functionType->getParamType(static_cast<unsigned>(i));
        llvm::Value* argument = _getValue(arguments[i]);
        if (!_isSupportedFunctionParameterType(parameterType) || !argument ||
            argument->getType() != parameterType ||
            !_isValueUsableAtInsertionPoint(state, insertionBlock, argument))
        {
            return SLANG_E_INVALID_ARG;
        }
        llvmArguments.push_back(argument);
    }

    llvm::CallInst* call = state->builder.CreateCall(llvmCallee, llvmArguments);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(call);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_emitValueReturn(SlangNVVMModuleHandle module, SlangNVVMValueHandle value)
{
    ModuleState* state = _getModule(module);
    llvm::Value* llvmValue = _getValue(value);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::Function* function = insertionBlock ? insertionBlock->getParent() : nullptr;
    if (!state || !llvmValue || !insertionBlock || !function ||
        !_isSupportedFunctionValueType(llvmValue->getType()) ||
        function->getReturnType() != llvmValue->getType() ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmValue))
    {
        return SLANG_E_INVALID_ARG;
    }

    state->builder.CreateRet(llvmValue);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitSequentialElementExtract(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle sequentialValue,
    SlangNVVMValueHandle elementIndex,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmSequentialValue = _getValue(sequentialValue);
    llvm::Value* llvmElementIndex = _getValue(elementIndex);
    auto vectorType = llvmSequentialValue
                          ? llvm::dyn_cast<llvm::FixedVectorType>(llvmSequentialValue->getType())
                          : nullptr;
    auto arrayType = llvmSequentialValue
                         ? llvm::dyn_cast<llvm::ArrayType>(llvmSequentialValue->getType())
                         : nullptr;
    const uint64_t elementCount = vectorType  ? vectorType->getNumElements()
                                  : arrayType ? arrayType->getNumElements()
                                              : 0;
    llvm::Type* elementType = vectorType  ? vectorType->getElementType()
                              : arrayType ? arrayType->getElementType()
                                          : nullptr;
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outValue || !insertionBlock || !elementCount || !elementType ||
        !llvmElementIndex || !llvm::isa<llvm::IntegerType>(llvmElementIndex->getType()) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmSequentialValue) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmElementIndex))
    {
        return SLANG_E_INVALID_ARG;
    }
    auto constantIndex = llvm::dyn_cast<llvm::ConstantInt>(llvmElementIndex);
    if (constantIndex)
    {
        if (constantIndex->getValue().uge(elementCount))
            return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = nullptr;
    if (constantIndex && arrayType)
    {
        result = state->builder.CreateExtractValue(
            llvmSequentialValue,
            uint32_t(constantIndex->getZExtValue()));
    }
    else if (!constantIndex && (arrayType || elementType->isIntegerTy(1)))
    {
        // LLVM has no dynamic `extractvalue`, and CUDA 12.9's libNVVM mishandles dynamic extracts
        // from `<N x i1>`. Both fixed sequences are bounded in this ABI, so use constant extracts
        // and typed selects. An out-of-range index retains LLVM's undefined result.
        result = llvm::UndefValue::get(elementType);
        for (uint32_t lane = 0; lane < elementCount; ++lane)
        {
            llvm::Value* laneValue =
                arrayType ? state->builder.CreateExtractValue(llvmSequentialValue, lane)
                          : state->builder.CreateExtractElement(llvmSequentialValue, lane);
            llvm::Value* laneIndex = llvm::ConstantInt::get(llvmElementIndex->getType(), lane);
            llvm::Value* isLane = state->builder.CreateICmpEQ(llvmElementIndex, laneIndex);
            result = state->builder.CreateSelect(isLane, laneValue, result);
        }
    }
    else
    {
        result = state->builder.CreateExtractElement(llvmSequentialValue, llvmElementIndex);
    }
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

// Constructs one already-validated fixed vector in the representation accepted by libNVVM.
// Consider `half2(a, b)`: CUDA 12.9 libNVVM accepts native Half2 arithmetic at O0 but rejects a
// runtime `insertelement <2 x half>`. Inserting the exact lane bits into the corresponding i16
// vector and bitcasting only the completed value preserves lane order and every Half payload. All
// other element types retain ordinary native vector insertion.
static llvm::Value* _createNVVMVectorConstruct(
    ModuleState* state,
    llvm::FixedVectorType* vectorType,
    llvm::ArrayRef<llvm::Value*> elements)
{
    llvm::Type* constructionElementType = vectorType->getElementType();
    llvm::FixedVectorType* constructionType = vectorType;
    const bool useHalfBitTransport = constructionElementType->isHalfTy();
    if (useHalfBitTransport)
    {
        constructionElementType = llvm::Type::getInt16Ty(state->context);
        constructionType =
            llvm::FixedVectorType::get(constructionElementType, vectorType->getNumElements());
    }

    llvm::Value* result = llvm::UndefValue::get(constructionType);
    for (size_t i = 0; i < elements.size(); ++i)
    {
        llvm::Value* element =
            useHalfBitTransport ? state->builder.CreateBitCast(elements[i], constructionElementType)
                                : elements[i];
        result = state->builder.CreateInsertElement(result, element, uint64_t(i));
    }
    return useHalfBitTransport ? state->builder.CreateBitCast(result, vectorType) : result;
}

static SlangResult SLANG_NVVM_CALL _emitVectorConstruct(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle vectorType,
    const SlangNVVMValueHandle* elements,
    size_t elementCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    auto llvmVectorType = llvm::dyn_cast_or_null<llvm::FixedVectorType>(_getType(vectorType));
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outValue || !insertionBlock || !llvmVectorType ||
        &llvmVectorType->getContext() != &state->context ||
        elementCount != llvmVectorType->getNumElements() || (!elements && elementCount))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::SmallVector<llvm::Value*, 4> llvmElements;
    llvmElements.reserve(elementCount);
    for (size_t i = 0; i < elementCount; ++i)
    {
        llvm::Value* element = _getValue(elements[i]);
        if (!element || element->getType() != llvmVectorType->getElementType() ||
            !_isValueUsableAtInsertionPoint(state, insertionBlock, element))
        {
            return SLANG_E_INVALID_ARG;
        }
        llvmElements.push_back(element);
    }

    llvm::Value* result = _createNVVMVectorConstruct(state, llvmVectorType, llvmElements);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _declareGlobalStorage(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle valueType,
    SlangNVVMLinkage linkage,
    SlangNVVMAddressSpace addressSpace,
    uint32_t alignment,
    const char* name,
    size_t nameSize,
    SlangNVVMValueHandle* outStorage)
{
    if (outStorage)
        *outStorage = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Type* llvmValueType = _getType(valueType);
    const llvm::StringRef llvmName = _getStringRef(name, nameSize);
    if (!state || !llvmValueType || &llvmValueType->getContext() != &state->context ||
        !llvm::PointerType::isLoadableOrStorableType(llvmValueType) || !llvmValueType->isSized() ||
        (linkage != SLANG_NVVM_LINKAGE_INTERNAL && linkage != SLANG_NVVM_LINKAGE_EXTERNAL) ||
        !_isNVVMAddressSpace(addressSpace) || !alignment || !llvm::isPowerOf2_32(alignment) ||
        alignment > llvm::Value::MaximumAlignment || !name || !nameSize ||
        state->module->getNamedValue(llvmName) || !outStorage)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::GlobalVariable* storage = new llvm::GlobalVariable(
        *state->module,
        llvmValueType,
        false,
        linkage == SLANG_NVVM_LINKAGE_EXTERNAL ? llvm::GlobalValue::ExternalLinkage
                                               : llvm::GlobalValue::InternalLinkage,
        llvm::UndefValue::get(llvmValueType),
        llvmName,
        nullptr,
        llvm::GlobalVariable::NotThreadLocal,
        addressSpace);
    storage->setAlignment(llvm::Align(alignment));
    *outStorage = reinterpret_cast<SlangNVVMValueHandle>(storage);
    return SLANG_OK;
}

static SlangResult _emitIntrinsic(
    SlangNVVMModuleHandle module,
    const SlangNVVMValueOperationDesc& operation,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !insertionBlock || (!arguments && argumentCount) || !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Intrinsic::ID intrinsicID = llvm::Intrinsic::not_intrinsic;
    size_t expectedArgumentCount = 0;
    llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
    llvm::Type* expectedArgumentTypes[] = {int32Type, int32Type, int32Type};
    bool bitcastsMatchFloatValue = false;
    const bool hasFloatingValue =
        operation.operandCount > 1 &&
        operation.operandTypes[1].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT;
    switch (operation.operation)
    {
    case SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK:
        {
            if (argumentCount != 0)
                return SLANG_E_INVALID_ARG;
            // LLVM 14 has no intrinsic for this hardware read. Consider a call inside a divergent
            // branch: a full-mask ballot would name lanes that never execute the call. Read the
            // currently executing lanes directly, and preserve its control dependence and distinct
            // observations with convergent and sideeffect. This does not synthesize a logical mask.
            llvm::FunctionType* functionType = llvm::FunctionType::get(int32Type, false);
            llvm::InlineAsm* assembly =
                llvm::InlineAsm::get(functionType, "activemask.b32 $0;", "=r", true);
            llvm::CallInst* call = state->builder.CreateCall(assembly);
            call->setConvergent();
            *outValue = reinterpret_cast<SlangNVVMValueHandle>(call);
            return SLANG_OK;
        }
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT:
        intrinsicID = llvm::Intrinsic::nvvm_vote_ballot_sync;
        expectedArgumentCount = 2;
        expectedArgumentTypes[1] = llvm::Type::getInt1Ty(state->context);
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH:
        intrinsicID = llvm::Intrinsic::nvvm_match_any_sync_i32;
        expectedArgumentCount = 2;
        if (hasFloatingValue)
            expectedArgumentTypes[1] = llvm::Type::getFloatTy(state->context);
        bitcastsMatchFloatValue = hasFloatingValue;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }
    if (argumentCount != expectedArgumentCount)
        return SLANG_E_INVALID_ARG;

    llvm::SmallVector<llvm::Value*, 4> llvmArguments;
    llvmArguments.reserve(expectedArgumentCount + 1);
    for (size_t i = 0; i < argumentCount; ++i)
    {
        llvm::Value* argument = _getValue(arguments[i]);
        if (!argument || argument->getType() != expectedArgumentTypes[i] ||
            !_isValueUsableAtInsertionPoint(state, insertionBlock, argument))
        {
            return SLANG_E_INVALID_ARG;
        }
        llvmArguments.push_back(argument);
    }
    if (bitcastsMatchFloatValue)
        llvmArguments[1] = state->builder.CreateBitCast(llvmArguments[1], int32Type);

    llvm::Function* intrinsic = llvm::Intrinsic::getDeclaration(state->module.get(), intrinsicID);
    llvm::CallInst* call = state->builder.CreateCall(intrinsic, llvmArguments);
    llvm::Value* result = call;
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitPointerOffset(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle basePointer,
    SlangNVVMValueHandle elementOffset,
    SlangNVVMValueHandle* outPointer)
{
    if (outPointer)
        *outPointer = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmBasePointer = _getValue(basePointer);
    llvm::Value* llvmElementOffset = _getValue(elementOffset);
    llvm::PointerType* pointerType =
        llvmBasePointer ? llvm::dyn_cast<llvm::PointerType>(llvmBasePointer->getType()) : nullptr;
    llvm::Type* pointeeType = pointerType && !pointerType->isOpaque()
                                  ? pointerType->getNonOpaquePointerElementType()
                                  : nullptr;
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outPointer || !insertionBlock || !pointerType || pointerType->isOpaque() ||
        !pointeeType || !pointeeType->isSized() || !llvmElementOffset ||
        !llvm::isa<llvm::IntegerType>(llvmElementOffset->getType()) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmBasePointer) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmElementOffset))
    {
        return SLANG_E_INVALID_ARG;
    }

    // A Slang element offset does not establish LLVM's stronger inbounds provenance contract.
    llvm::Value* result = state->builder.CreateGEP(pointeeType, llvmBasePointer, llvmElementOffset);
    *outPointer = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitByteOffsetPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle basePointer,
    SlangNVVMValueHandle byteOffset,
    SlangNVVMTypeHandle resultPointeeType,
    SlangNVVMValueHandle* outPointer)
{
    if (outPointer)
        *outPointer = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmBasePointer = _getValue(basePointer);
    llvm::Value* llvmByteOffset = _getValue(byteOffset);
    llvm::Type* llvmResultPointeeType = _getType(resultPointeeType);
    llvm::PointerType* basePointerType =
        llvmBasePointer ? llvm::dyn_cast<llvm::PointerType>(llvmBasePointer->getType()) : nullptr;
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outPointer || !insertionBlock || !basePointerType ||
        basePointerType->isOpaque() || !_isNVVMAddressSpace(basePointerType->getAddressSpace()) ||
        !llvmByteOffset || !llvm::isa<llvm::IntegerType>(llvmByteOffset->getType()) ||
        !llvmResultPointeeType || &llvmResultPointeeType->getContext() != &state->context ||
        !llvm::PointerType::isLoadableOrStorableType(llvmResultPointeeType) ||
        !llvmResultPointeeType->isSized() ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmBasePointer) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmByteOffset))
    {
        return SLANG_E_INVALID_ARG;
    }

    const unsigned addressSpace = basePointerType->getAddressSpace();
    llvm::Type* byteType = llvm::Type::getInt8Ty(state->context);
    llvm::PointerType* bytePointerType = llvm::PointerType::get(byteType, addressSpace);
    llvm::Value* byteBasePointer = state->builder.CreateBitCast(llvmBasePointer, bytePointerType);
    // A Slang byte offset does not establish LLVM's stronger inbounds provenance contract.
    llvm::Value* byteAddress = state->builder.CreateGEP(byteType, byteBasePointer, llvmByteOffset);
    llvm::PointerType* resultPointerType =
        llvm::PointerType::get(llvmResultPointeeType, addressSpace);
    llvm::Value* result = state->builder.CreateBitCast(byteAddress, resultPointerType);
    *outPointer = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitSequentialElementPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle baseSequentialPointer,
    SlangNVVMValueHandle elementIndex,
    SlangNVVMValueHandle* outPointer)
{
    if (outPointer)
        *outPointer = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmBaseSequentialPointer = _getValue(baseSequentialPointer);
    llvm::Value* llvmElementIndex = _getValue(elementIndex);
    llvm::PointerType* pointerType =
        llvmBaseSequentialPointer
            ? llvm::dyn_cast<llvm::PointerType>(llvmBaseSequentialPointer->getType())
            : nullptr;
    llvm::Type* pointeeType = pointerType && !pointerType->isOpaque()
                                  ? pointerType->getNonOpaquePointerElementType()
                                  : nullptr;
    llvm::ArrayType* arrayType = llvm::dyn_cast_or_null<llvm::ArrayType>(pointeeType);
    llvm::FixedVectorType* vectorType = llvm::dyn_cast_or_null<llvm::FixedVectorType>(pointeeType);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outPointer || !insertionBlock || !pointerType || pointerType->isOpaque() ||
        !_isNVVMAddressSpace(static_cast<SlangNVVMAddressSpace>(pointerType->getAddressSpace())) ||
        ((!arrayType || !arrayType->getNumElements() || !arrayType->isSized()) &&
         (!vectorType || !vectorType->getNumElements())) ||
        !llvmElementIndex || !llvm::isa<llvm::IntegerType>(llvmElementIndex->getType()) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmBaseSequentialPointer) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmElementIndex))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* indices[] = {
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(state->context), 0),
        llvmElementIndex};
    // A Slang subscript does not establish LLVM's stronger inbounds provenance contract.
    llvm::Value* result = state->builder.CreateGEP(pointeeType, llvmBaseSequentialPointer, indices);
    *outPointer = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitStructFieldPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle baseStructPointer,
    uint32_t fieldIndex,
    SlangNVVMValueHandle* outPointer)
{
    if (outPointer)
        *outPointer = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmBaseStructPointer = _getValue(baseStructPointer);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::PointerType* pointerType =
        llvmBaseStructPointer ? llvm::dyn_cast<llvm::PointerType>(llvmBaseStructPointer->getType())
                              : nullptr;
    llvm::StructType* structType =
        pointerType ? llvm::dyn_cast<llvm::StructType>(pointerType->getPointerElementType())
                    : nullptr;
    if (!state || !outPointer || !insertionBlock || !structType ||
        fieldIndex >= structType->getNumElements() ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmBaseStructPointer))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result =
        state->builder.CreateStructGEP(structType, llvmBaseStructPointer, fieldIndex);
    *outPointer = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static size_t _getAggregateElementCount(llvm::Type* aggregateType)
{
    if (auto arrayType = llvm::dyn_cast_or_null<llvm::ArrayType>(aggregateType))
        return size_t(arrayType->getNumElements());
    if (auto structType = llvm::dyn_cast_or_null<llvm::StructType>(aggregateType))
        return size_t(structType->getNumElements());
    return 0;
}

static llvm::Type* _getAggregateElementType(llvm::Type* aggregateType, size_t elementIndex)
{
    if (auto arrayType = llvm::dyn_cast_or_null<llvm::ArrayType>(aggregateType))
    {
        return elementIndex < arrayType->getNumElements() ? arrayType->getElementType() : nullptr;
    }
    if (auto structType = llvm::dyn_cast_or_null<llvm::StructType>(aggregateType))
    {
        return elementIndex < structType->getNumElements()
                   ? structType->getElementType(unsigned(elementIndex))
                   : nullptr;
    }
    return nullptr;
}

static SlangResult SLANG_NVVM_CALL _emitAggregateConstruct(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle aggregateType,
    const SlangNVVMValueHandle* elements,
    size_t elementCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Type* llvmAggregateType = _getType(aggregateType);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outValue || !insertionBlock || !llvmAggregateType ||
        (!llvm::isa<llvm::ArrayType>(llvmAggregateType) &&
         !llvm::isa<llvm::StructType>(llvmAggregateType)) ||
        &llvmAggregateType->getContext() != &state->context ||
        elementCount != _getAggregateElementCount(llvmAggregateType) || (!elements && elementCount))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::SmallVector<llvm::Value*, 4> llvmElements;
    llvmElements.reserve(elementCount);
    for (size_t i = 0; i < elementCount; ++i)
    {
        llvm::Value* element = _getValue(elements[i]);
        if (!element || element->getType() != _getAggregateElementType(llvmAggregateType, i) ||
            !_isValueUsableAtInsertionPoint(state, insertionBlock, element))
        {
            return SLANG_E_INVALID_ARG;
        }
        llvmElements.push_back(element);
    }

    llvm::Value* result = llvm::UndefValue::get(llvmAggregateType);
    for (size_t i = 0; i < elementCount; ++i)
        result = state->builder.CreateInsertValue(result, llvmElements[i], {unsigned(i)});
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitAggregateElementExtract(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle aggregateValue,
    uint32_t elementIndex,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmAggregateValue = _getValue(aggregateValue);
    llvm::Type* aggregateType = llvmAggregateValue ? llvmAggregateValue->getType() : nullptr;
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outValue || !insertionBlock ||
        !_getAggregateElementType(aggregateType, elementIndex) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmAggregateValue))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = state->builder.CreateExtractValue(llvmAggregateValue, {elementIndex});
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

// Returns whether one first-class integer shape carries exactly one 64-bit pointer bit pattern.
static bool _isPointerBitPatternType(llvm::Type* type)
{
    if (auto integerType = llvm::dyn_cast_or_null<llvm::IntegerType>(type))
        return integerType->getBitWidth() == 64;
    auto vectorType = llvm::dyn_cast_or_null<llvm::FixedVectorType>(type);
    auto elementType =
        vectorType ? llvm::dyn_cast<llvm::IntegerType>(vectorType->getElementType()) : nullptr;
    return vectorType && elementType && vectorType->getNumElements() == 2 &&
           elementType->getBitWidth() == 32;
}

static SlangResult SLANG_NVVM_CALL _emitBitCast(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle resultType,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Type* llvmResultType = _getType(resultType);
    llvm::Value* llvmValue = _getValue(value);
    llvm::Type* llvmValueType = llvmValue ? llvmValue->getType() : nullptr;
    auto resultPointerType = llvm::dyn_cast_or_null<llvm::PointerType>(llvmResultType);
    auto valuePointerType = llvm::dyn_cast_or_null<llvm::PointerType>(llvmValueType);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    const bool isPointerToBits = valuePointerType && _isPointerBitPatternType(llvmResultType);
    const bool isBitsToPointer = resultPointerType && _isPointerBitPatternType(llvmValueType);
    if (!state || !outValue || !insertionBlock || !llvmResultType || !llvmValue ||
        &llvmResultType->getContext() != &state->context ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmValue) ||
        (!isPointerToBits && !isBitsToPointer) ||
        (valuePointerType &&
         (valuePointerType->isOpaque() || !_isNVVMAddressSpace(static_cast<SlangNVVMAddressSpace>(
                                              valuePointerType->getAddressSpace())))) ||
        (resultPointerType &&
         (resultPointerType->isOpaque() || !_isNVVMAddressSpace(static_cast<SlangNVVMAddressSpace>(
                                               resultPointerType->getAddressSpace())))))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Type* int64Type = llvm::Type::getInt64Ty(state->context);
    llvm::Value* result = nullptr;
    if (isPointerToBits)
    {
        result = state->builder.CreatePtrToInt(llvmValue, int64Type);
        if (llvmResultType != int64Type)
            result = state->builder.CreateBitCast(result, llvmResultType);
    }
    else
    {
        llvm::Value* integerBits = llvmValue;
        if (llvmValueType != int64Type)
            integerBits = state->builder.CreateBitCast(integerBits, int64Type);
        result = state->builder.CreateIntToPtr(integerBits, llvmResultType);
    }
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitPointerAddressSpaceCast(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle resultType,
    SlangNVVMValueHandle pointer,
    SlangNVVMValueHandle* outPointer)
{
    if (outPointer)
        *outPointer = nullptr;

    ModuleState* state = _getModule(module);
    auto llvmResultType = llvm::dyn_cast_or_null<llvm::PointerType>(_getType(resultType));
    llvm::Value* llvmPointer = _getValue(pointer);
    auto llvmPointerType =
        llvmPointer ? llvm::dyn_cast<llvm::PointerType>(llvmPointer->getType()) : nullptr;
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outPointer || !insertionBlock || !llvmResultType || !llvmPointerType ||
        llvmResultType->isOpaque() || llvmPointerType->isOpaque() ||
        &llvmResultType->getContext() != &state->context ||
        !_isNVVMAddressSpace(
            static_cast<SlangNVVMAddressSpace>(llvmResultType->getAddressSpace())) ||
        !_isNVVMAddressSpace(
            static_cast<SlangNVVMAddressSpace>(llvmPointerType->getAddressSpace())) ||
        llvmResultType->getAddressSpace() == llvmPointerType->getAddressSpace() ||
        llvmResultType->getNonOpaquePointerElementType() !=
            llvmPointerType->getNonOpaquePointerElementType() ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmPointer))
    {
        return SLANG_E_INVALID_ARG;
    }

    *outPointer = reinterpret_cast<SlangNVVMValueHandle>(
        state->builder.CreateAddrSpaceCast(llvmPointer, llvmResultType));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitReturnVoid(SlangNVVMModuleHandle module)
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

static SlangResult SLANG_NVVM_CALL _emitUnreachable(SlangNVVMModuleHandle module)
{
    ModuleState* state = _getModule(module);
    llvm::BasicBlock* block = state ? state->builder.GetInsertBlock() : nullptr;
    if (!state || !block || block->getTerminator() || !block->getParent() ||
        block->getParent()->getParent() != state->module.get())
    {
        return SLANG_E_INVALID_ARG;
    }

    state->builder.CreateUnreachable();
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_markFunctionAsKernel(SlangNVVMModuleHandle module, SlangNVVMValueHandle function)
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
static bool _isSerializationFormat(SlangNVVMSerializationFormat format)
{
    return format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY ||
           format == SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE;
}

static bool _isExecutionRegisterIntrinsic(llvm::Intrinsic::ID intrinsicID);

// Selects the native Half math operations whose signatures are owned by LLVM's registry.
static bool _isHalfMathIntrinsic(llvm::Intrinsic::ID intrinsicID)
{
    return intrinsicID == llvm::Intrinsic::ceil || intrinsicID == llvm::Intrinsic::floor ||
           intrinsicID == llvm::Intrinsic::trunc || intrinsicID == llvm::Intrinsic::fma;
}

// Shares admission of the qualified scalar integer intrinsics with serialization validation.
static bool _isIntegerBitIntrinsic(llvm::Intrinsic::ID intrinsicID)
{
    return intrinsicID == llvm::Intrinsic::ctpop || intrinsicID == llvm::Intrinsic::bitreverse ||
           intrinsicID == llvm::Intrinsic::ctlz || intrinsicID == llvm::Intrinsic::cttz;
}

// Classifies the synchronization operations admitted by the qualified NVVM dialect. The named
// query and serializer share this boundary; LLVM owns their signatures and effects.
static bool _isSynchronizationIntrinsic(llvm::Intrinsic::ID intrinsicID)
{
    return intrinsicID == llvm::Intrinsic::nvvm_barrier0 ||
           intrinsicID == llvm::Intrinsic::nvvm_membar_gl ||
           intrinsicID == llvm::Intrinsic::nvvm_membar_cta;
}

static void _addUniqueAttributeSet(
    llvm::SmallVectorImpl<llvm::AttributeSet>& attributeSets,
    llvm::AttributeSet attributeSet)
{
    for (const llvm::AttributeSet& existing : attributeSets)
    {
        if (existing == attributeSet)
            return;
    }
    attributeSets.push_back(attributeSet);
}

// Checks the one relaxed scalar compare-exchange shape that the provider deliberately emits.
static bool _isSelectedLegacyAtomicAccess(
    llvm::Type* valueType,
    llvm::Value* pointer,
    llvm::Align alignment,
    llvm::AtomicOrdering ordering,
    llvm::SyncScope::ID syncScope,
    bool isVolatile)
{
    auto pointerType = llvm::dyn_cast<llvm::PointerType>(pointer->getType());
    const unsigned bitWidth = valueType->getScalarSizeInBits();
    const bool hasSelectedType = valueType->isIntegerTy(32) || valueType->isIntegerTy(64);
    const bool hasNaturalAlignment = (bitWidth == 32 && alignment == llvm::Align(4)) ||
                                     (bitWidth == 64 && alignment == llvm::Align(8));
    return pointerType &&
           (pointerType->getAddressSpace() == SLANG_NVVM_ADDRESS_SPACE_GLOBAL ||
            pointerType->getAddressSpace() == SLANG_NVVM_ADDRESS_SPACE_SHARED) &&
           hasSelectedType && hasNaturalAlignment && ordering == llvm::AtomicOrdering::Monotonic &&
           syncScope == llvm::SyncScope::System && !isVolatile;
}

// Writes the legacy LLVM textual dialect accepted by libNVVM's documented LLVM 7 reader.
//
// LLVM 14 made atomic alignment explicit in assembly, but LLVM 7 gives atomicrmw and cmpxchg their
// natural alignment and rejects the suffix. LLVM 14 also prints unary negation as `fneg`, which the
// libNVVM NVVM-2.0 reader rejects; the older dialect expresses finite scalar negation as
// `fsub -0.0, value`. Finally, LLVM 14 gives NVVM special-register intrinsics function attributes
// that the LLVM 7 parser does not know. Removing optimization-only attributes retains each
// intrinsic's semantic name and type. This applies to both NVVM intrinsics and generic intrinsics
// such as scalar sqrt and Float32-to-Half conversion that survive into the module. LLVM may share
// one numbered attribute group between several declarations, so count unique validated semantic
// attribute sets. LLVM 14's scalar shuffle and synchronized-vote declarations already use the
// LLVM-7-compatible convergent/inaccessible-memory/nounwind set, but validate their exact
// signatures and attributes before serializing the mixed dialect. Generic integer scan intrinsics
// have the same LLVM 14-only optimization attributes as the special-register declarations plus an
// `immarg` parameter marker; LLVM 7 already understands their signatures and semantics once those
// newer attributes are removed. The provider exposes exactly one shape of each operation; validate
// every semantic instruction or declaration before changing its spelling.
static SlangResult _writeLegacyNVVMAssembly(
    ModuleState* state,
    llvm::SmallVectorImpl<char>& outSerializedData)
{
    size_t semanticAtomicCount = 0;
    size_t semanticFloatNegateCount = 0;
    size_t semanticIntegerScanDeclarationCount = 0;
    size_t semanticByValueParameterCount = 0;
    llvm::SmallVector<llvm::AttributeSet, 2> semanticLegacyIntrinsicAttributeSets;
    for (llvm::Function& function : *state->module)
    {
        for (llvm::Argument& argument : function.args())
        {
            if (!argument.hasByValAttr())
                continue;
            auto pointerType = llvm::dyn_cast<llvm::PointerType>(argument.getType());
            llvm::Type* byValueType = argument.getParamByValType();
            const llvm::MaybeAlign alignment = argument.getParamAlign();
            if (!pointerType || pointerType->isOpaque() || !byValueType ||
                pointerType->getPointerElementType() != byValueType || !byValueType->isSized() ||
                !alignment ||
                function.getAttributes().getParamAttrs(argument.getArgNo()).getNumAttributes() != 2)
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            ++semanticByValueParameterCount;
        }

        const llvm::Intrinsic::ID intrinsicID = function.getIntrinsicID();
        const bool isExecutionRegister = _isExecutionRegisterIntrinsic(intrinsicID);
        if (intrinsicID == llvm::Intrinsic::nvvm_read_ptx_sreg_laneid ||
            intrinsicID == llvm::Intrinsic::nvvm_read_ptx_sreg_warpsize || isExecutionRegister)
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            if (!function.isDeclaration() || !function.getReturnType()->isIntegerTy(32) ||
                function.arg_size() != 0 ||
                functionAttributes.getNumAttributes() != (isExecutionRegister ? 3u : 6u) ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind) ||
                !function.hasFnAttribute(llvm::Attribute::ReadNone) ||
                !function.hasFnAttribute(llvm::Attribute::Speculatable) ||
                (!isExecutionRegister && (!function.hasFnAttribute(llvm::Attribute::NoFree) ||
                                          !function.hasFnAttribute(llvm::Attribute::NoSync) ||
                                          !function.hasFnAttribute(llvm::Attribute::WillReturn))))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            _addUniqueAttributeSet(semanticLegacyIntrinsicAttributeSets, functionAttributes);
        }
        else if (_isSynchronizationIntrinsic(intrinsicID))
        {
            const bool isBarrier = intrinsicID == llvm::Intrinsic::nvvm_barrier0;
            if (!function.isDeclaration() || !function.getReturnType()->isVoidTy() ||
                function.arg_size() != 0 ||
                function.getAttributes().getFnAttrs().getNumAttributes() != (isBarrier ? 2u : 1u) ||
                function.hasFnAttribute(llvm::Attribute::Convergent) != isBarrier ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }
        else if (
            intrinsicID == llvm::Intrinsic::nvvm_shfl_sync_idx_i32 ||
            intrinsicID == llvm::Intrinsic::nvvm_shfl_sync_idx_f32)
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
            llvm::Type* payloadType = intrinsicID == llvm::Intrinsic::nvvm_shfl_sync_idx_f32
                                          ? llvm::Type::getFloatTy(state->context)
                                          : int32Type;
            if (!function.isDeclaration() || function.getReturnType() != payloadType ||
                function.arg_size() != 4 || functionAttributes.getNumAttributes() != 3 ||
                !function.hasFnAttribute(llvm::Attribute::Convergent) ||
                !function.hasFnAttribute(llvm::Attribute::InaccessibleMemOnly) ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            size_t argumentIndex = 0;
            for (const llvm::Argument& argument : function.args())
            {
                llvm::Type* expectedType = argumentIndex == 1 ? payloadType : int32Type;
                if (argument.getType() != expectedType)
                    return SLANG_E_NOT_AVAILABLE;
                ++argumentIndex;
            }
        }
        else if (
            intrinsicID == llvm::Intrinsic::nvvm_vote_ballot_sync ||
            intrinsicID == llvm::Intrinsic::nvvm_vote_any_sync ||
            intrinsicID == llvm::Intrinsic::nvvm_vote_all_sync)
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
            llvm::Type* expectedResultType = intrinsicID == llvm::Intrinsic::nvvm_vote_ballot_sync
                                                 ? int32Type
                                                 : llvm::Type::getInt1Ty(state->context);
            if (!function.isDeclaration() || function.getReturnType() != expectedResultType ||
                function.arg_size() != 2 || functionAttributes.getNumAttributes() != 3 ||
                !function.hasFnAttribute(llvm::Attribute::Convergent) ||
                !function.hasFnAttribute(llvm::Attribute::InaccessibleMemOnly) ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            auto argument = function.arg_begin();
            if (argument->getType() != int32Type ||
                (++argument)->getType() != llvm::Type::getInt1Ty(state->context))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }
        else if (intrinsicID == llvm::Intrinsic::nvvm_match_any_sync_i32)
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
            if (!function.isDeclaration() || function.getReturnType() != int32Type ||
                function.arg_size() != 2 || functionAttributes.getNumAttributes() != 3 ||
                !function.hasFnAttribute(llvm::Attribute::Convergent) ||
                !function.hasFnAttribute(llvm::Attribute::InaccessibleMemOnly) ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            for (const llvm::Argument& argument : function.args())
            {
                if (argument.getType() != int32Type)
                    return SLANG_E_NOT_AVAILABLE;
            }
        }
        else if (_isIntegerBitIntrinsic(intrinsicID))
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            llvm::IntegerType* integerType =
                llvm::dyn_cast<llvm::IntegerType>(function.getReturnType());
            const bool isScan =
                intrinsicID == llvm::Intrinsic::ctlz || intrinsicID == llvm::Intrinsic::cttz;
            const bool isSelectedWidth =
                integerType &&
                (integerType->getBitWidth() == 8 || integerType->getBitWidth() == 16 ||
                 integerType->getBitWidth() == 32 || integerType->getBitWidth() == 64);
            if (!function.isDeclaration() || !isSelectedWidth ||
                function.arg_size() != (isScan ? 2u : 1u) ||
                functionAttributes.getNumAttributes() != 6 ||
                !function.hasFnAttribute(llvm::Attribute::NoFree) ||
                !function.hasFnAttribute(llvm::Attribute::NoSync) ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind) ||
                !function.hasFnAttribute(llvm::Attribute::ReadNone) ||
                !function.hasFnAttribute(llvm::Attribute::Speculatable) ||
                !function.hasFnAttribute(llvm::Attribute::WillReturn) ||
                function.getAttributes().getParamAttrs(0).getNumAttributes() != 0)
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            auto argument = function.arg_begin();
            if (argument->getType() != integerType)
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            if (isScan)
            {
                ++argument;
                if (argument->getType() != llvm::Type::getInt1Ty(state->context) ||
                    function.getAttributes().getParamAttrs(1).getNumAttributes() != 1 ||
                    !function.hasParamAttribute(1, llvm::Attribute::ImmArg))
                {
                    return SLANG_E_NOT_AVAILABLE;
                }
                ++semanticIntegerScanDeclarationCount;
            }
            _addUniqueAttributeSet(semanticLegacyIntrinsicAttributeSets, functionAttributes);
        }
        else if (
            intrinsicID == llvm::Intrinsic::sqrt || intrinsicID == llvm::Intrinsic::nvvm_f2h_rn ||
            (_isHalfMathIntrinsic(intrinsicID) && intrinsicID != llvm::Intrinsic::trunc))
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            llvm::Type* resultType = function.getReturnType();
            const bool isHalfConversion = intrinsicID == llvm::Intrinsic::nvvm_f2h_rn;
            const bool isHalfMath = _isHalfMathIntrinsic(intrinsicID);
            const bool hasSelectedResult =
                isHalfConversion ? resultType->isIntegerTy(16)
                : isHalfMath     ? resultType->isHalfTy()
                                 : resultType->isFloatTy() || resultType->isDoubleTy();
            llvm::Type* operandType =
                isHalfConversion ? llvm::Type::getFloatTy(state->context) : resultType;
            if (!function.isDeclaration() || !hasSelectedResult ||
                function.arg_size() != (intrinsicID == llvm::Intrinsic::fma ? 3u : 1u) ||
                functionAttributes.getNumAttributes() != 6 ||
                !function.hasFnAttribute(llvm::Attribute::NoFree) ||
                !function.hasFnAttribute(llvm::Attribute::NoSync) ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind) ||
                !function.hasFnAttribute(llvm::Attribute::ReadNone) ||
                !function.hasFnAttribute(llvm::Attribute::Speculatable) ||
                !function.hasFnAttribute(llvm::Attribute::WillReturn))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            for (const llvm::Argument& argument : function.args())
            {
                if (argument.getType() != operandType)
                    return SLANG_E_NOT_AVAILABLE;
            }
            _addUniqueAttributeSet(semanticLegacyIntrinsicAttributeSets, functionAttributes);
        }
        else if (
            intrinsicID == llvm::Intrinsic::nvvm_txq_width ||
            intrinsicID == llvm::Intrinsic::nvvm_txq_height ||
            intrinsicID == llvm::Intrinsic::nvvm_txq_depth)
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
            llvm::Type* int64Type = llvm::Type::getInt64Ty(state->context);
            if (!function.isDeclaration() || function.getReturnType() != int32Type ||
                function.arg_size() != 1 || function.arg_begin()->getType() != int64Type ||
                functionAttributes.getNumAttributes() != 2 ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind) ||
                !function.hasFnAttribute(llvm::Attribute::ReadNone))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }
        for (llvm::BasicBlock& block : function)
        {
            for (llvm::Instruction& instruction : block)
            {
                if (instruction.getOpcode() == llvm::Instruction::FNeg)
                {
                    ++semanticFloatNegateCount;
                    if (instruction.getNumOperands() != 1 || !instruction.getType()->isFloatTy() ||
                        instruction.getOperand(0)->getType() != instruction.getType())
                    {
                        return SLANG_E_NOT_AVAILABLE;
                    }
                    continue;
                }

                if (auto atomic = llvm::dyn_cast<llvm::AtomicRMWInst>(&instruction))
                {
                    ++semanticAtomicCount;
                    const unsigned addressSpace = atomic->getPointerAddressSpace();
                    const unsigned bitWidth = atomic->getType()->getScalarSizeInBits();
                    const bool hasNaturalAlignment =
                        (bitWidth == 32 && atomic->getAlign() == llvm::Align(4)) ||
                        (bitWidth == 64 && atomic->getAlign() == llvm::Align(8));
                    const bool isAtomicAddressSpace =
                        addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL ||
                        addressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED;
                    const bool isSelectedIntegerOperation =
                        isAtomicAddressSpace && atomic->getType()->isIntegerTy() &&
                        (bitWidth == 32 || bitWidth == 64) && hasNaturalAlignment &&
                        (atomic->getOperation() == llvm::AtomicRMWInst::Add ||
                         atomic->getOperation() == llvm::AtomicRMWInst::And ||
                         atomic->getOperation() == llvm::AtomicRMWInst::Or ||
                         atomic->getOperation() == llvm::AtomicRMWInst::Xor ||
                         atomic->getOperation() == llvm::AtomicRMWInst::Min ||
                         atomic->getOperation() == llvm::AtomicRMWInst::UMin ||
                         atomic->getOperation() == llvm::AtomicRMWInst::Max ||
                         atomic->getOperation() == llvm::AtomicRMWInst::UMax ||
                         atomic->getOperation() == llvm::AtomicRMWInst::Xchg);
                    const bool isSelectedGlobalFloatingReduction =
                        addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL &&
                        atomic->getType()->isFloatingPointTy() &&
                        (bitWidth == 32 || bitWidth == 64) && hasNaturalAlignment &&
                        atomic->getOperation() == llvm::AtomicRMWInst::FAdd;
                    if ((!isSelectedIntegerOperation && !isSelectedGlobalFloatingReduction) ||
                        atomic->getOrdering() != llvm::AtomicOrdering::Monotonic ||
                        atomic->getSyncScopeID() != llvm::SyncScope::System || atomic->isVolatile())
                    {
                        return SLANG_E_NOT_AVAILABLE;
                    }
                    continue;
                }

                if (auto load = llvm::dyn_cast<llvm::LoadInst>(&instruction))
                {
                    // libNVVM 2.0 rejects atomic load/store syntax. The atomic callback lowers
                    // those source operations to cmpxchg/xchg before this dialect boundary.
                    if (load->isAtomic())
                        return SLANG_E_NOT_AVAILABLE;
                    continue;
                }
                if (auto store = llvm::dyn_cast<llvm::StoreInst>(&instruction))
                {
                    if (store->isAtomic())
                        return SLANG_E_NOT_AVAILABLE;
                    continue;
                }
                auto compareExchange = llvm::dyn_cast<llvm::AtomicCmpXchgInst>(&instruction);
                if (!compareExchange)
                    continue;
                ++semanticAtomicCount;
                if (!_isSelectedLegacyAtomicAccess(
                        compareExchange->getCompareOperand()->getType(),
                        compareExchange->getPointerOperand(),
                        compareExchange->getAlign(),
                        compareExchange->getSuccessOrdering(),
                        compareExchange->getSyncScopeID(),
                        compareExchange->isVolatile()) ||
                    compareExchange->getFailureOrdering() != llvm::AtomicOrdering::Monotonic)
                {
                    return SLANG_E_NOT_AVAILABLE;
                }
            }
        }
    }

    llvm::SmallVector<char, 0> llvm14Assembly;
    llvm::raw_svector_ostream llvm14Output(llvm14Assembly);
    state->module->print(llvm14Output, nullptr);

    const llvm::StringRef atomicMarker(" = atomicrmw ");
    const llvm::StringRef llvm14I32AlignmentSuffix(", align 4");
    const llvm::StringRef llvm14I64AlignmentSuffix(", align 8");
    const llvm::StringRef llvm14GlobalF32AtomicAddMarker(" = atomicrmw fadd float addrspace(1)* ");
    const llvm::StringRef llvm14GlobalF64AtomicAddMarker(" = atomicrmw fadd double addrspace(1)* ");
    const llvm::StringRef llvm14F32AtomicAddSuffix(" monotonic, align 4");
    const llvm::StringRef llvm14F64AtomicAddSuffix(" monotonic, align 8");
    const llvm::StringRef llvm14F32AtomicAddSeparator(", float ");
    const llvm::StringRef llvm14F64AtomicAddSeparator(", double ");
    const llvm::StringRef legacyGlobalF32AtomicAddName("@llvm.nvvm.atomic.load.add.f32.p1f32");
    const llvm::StringRef legacyGlobalF64AtomicAddName("@llvm.nvvm.atomic.load.add.f64.p1f64");
    const llvm::StringRef floatNegateMarker(" = fneg float ");
    const llvm::StringRef legacyFloatNegateMarker(" = fsub float -0.000000e+00, ");
    const llvm::StringRef llvm14SpecialRegisterAttributeMarker(
        " = { nofree nosync nounwind readnone speculatable willreturn }");
    const llvm::StringRef llvm14ExecutionRegisterAttributeMarker(
        " = { nounwind readnone speculatable }");
    const llvm::StringRef legacySpecialRegisterAttributes(" = { nounwind readnone }");
    const llvm::StringRef integerScanParameterMarker("i1 immarg");
    const llvm::StringRef legacyIntegerScanParameter("i1");
    llvm::StringRef remaining(llvm14Assembly.data(), llvm14Assembly.size());
    size_t rewrittenAtomicCount = 0;
    size_t rewrittenFloatNegateCount = 0;
    size_t rewrittenLegacyIntrinsicAttributeSetCount = 0;
    size_t rewrittenIntegerScanDeclarationCount = 0;
    size_t rewrittenByValueParameterCount = 0;
    bool needsLegacyGlobalF32AtomicAdd = false;
    bool needsLegacyGlobalF64AtomicAdd = false;
    while (!remaining.empty())
    {
        const size_t newlineIndex = remaining.find('\n');
        const bool hasNewline = newlineIndex != llvm::StringRef::npos;
        const llvm::StringRef line = hasNewline ? remaining.take_front(newlineIndex) : remaining;

        const llvm::StringRef trimmedLine = line.ltrim();
        const bool isGlobalF32AtomicAdd =
            trimmedLine.startswith("%") && line.contains(llvm14GlobalF32AtomicAddMarker);
        const bool isGlobalF64AtomicAdd =
            trimmedLine.startswith("%") && line.contains(llvm14GlobalF64AtomicAddMarker);
        if (isGlobalF32AtomicAdd || isGlobalF64AtomicAdd)
        {
            // LLVM 7/libNVVM models floating-point atomic add with an NVVM intrinsic rather than
            // the newer `atomicrmw fadd` operation. Translate the one provider-produced scalar
            // global form at the isolated dialect boundary; all other instructions stay typed IR.
            const llvm::StringRef marker = isGlobalF32AtomicAdd ? llvm14GlobalF32AtomicAddMarker
                                                                : llvm14GlobalF64AtomicAddMarker;
            const llvm::StringRef suffix =
                isGlobalF32AtomicAdd ? llvm14F32AtomicAddSuffix : llvm14F64AtomicAddSuffix;
            const llvm::StringRef separator =
                isGlobalF32AtomicAdd ? llvm14F32AtomicAddSeparator : llvm14F64AtomicAddSeparator;
            const llvm::StringRef intrinsicName =
                isGlobalF32AtomicAdd ? legacyGlobalF32AtomicAddName : legacyGlobalF64AtomicAddName;
            const llvm::StringRef valueType = isGlobalF32AtomicAdd ? "float" : "double";
            const size_t markerIndex = line.find(marker);
            if (!line.endswith(suffix))
                return SLANG_E_NOT_AVAILABLE;
            const llvm::StringRef arguments =
                line.drop_front(markerIndex + marker.size()).drop_back(suffix.size());
            const size_t separatorIndex = arguments.find(separator);
            if (separatorIndex == llvm::StringRef::npos ||
                arguments.find(separator, separatorIndex + 1) != llvm::StringRef::npos)
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            const llvm::StringRef pointer = arguments.take_front(separatorIndex);
            const llvm::StringRef value = arguments.drop_front(separatorIndex + separator.size());
            if (pointer.empty() || value.empty())
                return SLANG_E_NOT_AVAILABLE;

            const llvm::StringRef result = line.take_front(markerIndex);
            outSerializedData.append(result.begin(), result.end());
            outSerializedData.append(" = call ", " = call " + 8);
            outSerializedData.append(valueType.begin(), valueType.end());
            outSerializedData.push_back(' ');
            outSerializedData.append(intrinsicName.begin(), intrinsicName.end());
            outSerializedData.push_back('(');
            outSerializedData.append(valueType.begin(), valueType.end());
            outSerializedData.append(" addrspace(1)* ", " addrspace(1)* " + 15);
            outSerializedData.append(pointer.begin(), pointer.end());
            outSerializedData.append(separator.begin(), separator.end());
            outSerializedData.append(value.begin(), value.end());
            outSerializedData.push_back(')');
            ++rewrittenAtomicCount;
            needsLegacyGlobalF32AtomicAdd |= isGlobalF32AtomicAdd;
            needsLegacyGlobalF64AtomicAdd |= isGlobalF64AtomicAdd;
        }
        else if (
            trimmedLine.startswith("%") &&
            (trimmedLine.contains(atomicMarker) || trimmedLine.contains(" = cmpxchg ")))
        {
            const llvm::StringRef alignmentSuffix =
                line.endswith(llvm14I32AlignmentSuffix)   ? llvm14I32AlignmentSuffix
                : line.endswith(llvm14I64AlignmentSuffix) ? llvm14I64AlignmentSuffix
                                                          : llvm::StringRef();
            if (alignmentSuffix.empty())
                return SLANG_E_NOT_AVAILABLE;
            const llvm::StringRef legacyLine = line.drop_back(alignmentSuffix.size());
            outSerializedData.append(legacyLine.begin(), legacyLine.end());
            ++rewrittenAtomicCount;
        }
        else if (trimmedLine.startswith("%") && trimmedLine.contains(floatNegateMarker))
        {
            const size_t markerIndex = line.find(floatNegateMarker);
            const llvm::StringRef result = line.take_front(markerIndex);
            const llvm::StringRef operand = line.drop_front(markerIndex + floatNegateMarker.size());
            if (operand.empty())
                return SLANG_E_NOT_AVAILABLE;
            outSerializedData.append(result.begin(), result.end());
            outSerializedData.append(
                legacyFloatNegateMarker.begin(),
                legacyFloatNegateMarker.end());
            outSerializedData.append(operand.begin(), operand.end());
            ++rewrittenFloatNegateCount;
        }
        else if (
            trimmedLine.startswith("attributes #") &&
            (line.endswith(llvm14SpecialRegisterAttributeMarker) ||
             line.endswith(llvm14ExecutionRegisterAttributeMarker)))
        {
            const llvm::StringRef attributeMarker =
                line.endswith(llvm14SpecialRegisterAttributeMarker)
                    ? llvm14SpecialRegisterAttributeMarker
                    : llvm14ExecutionRegisterAttributeMarker;
            const llvm::StringRef prefix = line.drop_back(attributeMarker.size());
            outSerializedData.append(prefix.begin(), prefix.end());
            outSerializedData.append(
                legacySpecialRegisterAttributes.begin(),
                legacySpecialRegisterAttributes.end());
            ++rewrittenLegacyIntrinsicAttributeSetCount;
        }
        else if (
            trimmedLine.startswith("declare i") &&
            (trimmedLine.contains(" @llvm.ctlz.i") || trimmedLine.contains(" @llvm.cttz.i")))
        {
            const size_t markerIndex = line.find(integerScanParameterMarker);
            if (markerIndex == llvm::StringRef::npos ||
                line.find(integerScanParameterMarker, markerIndex + 1) != llvm::StringRef::npos)
                return SLANG_E_NOT_AVAILABLE;
            const llvm::StringRef prefix = line.take_front(markerIndex);
            const llvm::StringRef suffix =
                line.drop_front(markerIndex + integerScanParameterMarker.size());
            outSerializedData.append(prefix.begin(), prefix.end());
            outSerializedData.append(
                legacyIntegerScanParameter.begin(),
                legacyIntegerScanParameter.end());
            outSerializedData.append(suffix.begin(), suffix.end());
            ++rewrittenIntegerScanDeclarationCount;
        }
        else if (line.contains("byval("))
        {
            const llvm::StringRef marker("byval(");
            size_t copiedEnd = 0;
            size_t markerIndex = line.find(marker);
            while (markerIndex != llvm::StringRef::npos)
            {
                outSerializedData.append(line.begin() + copiedEnd, line.begin() + markerIndex);
                outSerializedData.append(marker.begin(), marker.begin() + 5);

                size_t cursor = markerIndex + marker.size();
                size_t depth = 1;
                while (cursor < line.size() && depth)
                {
                    if (line[cursor] == '(')
                        ++depth;
                    else if (line[cursor] == ')')
                        --depth;
                    ++cursor;
                }
                if (depth)
                    return SLANG_E_NOT_AVAILABLE;

                copiedEnd = cursor;
                markerIndex = line.find(marker, copiedEnd);
                ++rewrittenByValueParameterCount;
            }
            outSerializedData.append(line.begin() + copiedEnd, line.end());
        }
        else
        {
            outSerializedData.append(line.begin(), line.end());
        }

        if (!hasNewline)
            break;
        outSerializedData.push_back('\n');
        remaining = remaining.drop_front(newlineIndex + 1);
    }

    if (needsLegacyGlobalF32AtomicAdd)
    {
        const llvm::StringRef declaration =
            "declare float @llvm.nvvm.atomic.load.add.f32.p1f32(float addrspace(1)* nocapture, "
            "float)\n";
        outSerializedData.append(declaration.begin(), declaration.end());
    }
    if (needsLegacyGlobalF64AtomicAdd)
    {
        const llvm::StringRef declaration =
            "declare double @llvm.nvvm.atomic.load.add.f64.p1f64(double addrspace(1)* nocapture, "
            "double)\n";
        outSerializedData.append(declaration.begin(), declaration.end());
    }

    return rewrittenAtomicCount == semanticAtomicCount &&
                   rewrittenFloatNegateCount == semanticFloatNegateCount &&
                   rewrittenLegacyIntrinsicAttributeSetCount ==
                       semanticLegacyIntrinsicAttributeSets.size() &&
                   rewrittenIntegerScanDeclarationCount == semanticIntegerScanDeclarationCount &&
                   rewrittenByValueParameterCount == semanticByValueParameterCount
               ? SLANG_OK
               : SLANG_E_NOT_AVAILABLE;
}

// Verifies once and materializes the canonical byte result shared by both current serializers.
static SlangResult _materializeModule(
    ModuleState* state,
    SlangNVVMSerializationFormat format,
    bool useNVVMIR20Assembly,
    llvm::SmallVectorImpl<char>& outSerializedData,
    llvm::SmallVectorImpl<char>& outDiagnosticData,
    SlangNVVMVerificationStatus& outVerificationStatus)
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

    // Preserve verifier diagnostics even when the caller also supplied an unknown format.
    const bool isSupportedFormat =
        useNVVMIR20Assembly ? format == SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY
                            : _isSerializationFormat(format);
    if (!isSupportedFormat)
        return SLANG_E_INVALID_ARG;

    outDiagnosticData.clear();
    if (useNVVMIR20Assembly)
    {
        const SlangResult assemblyResult = _writeLegacyNVVMAssembly(state, outSerializedData);
        if (SLANG_FAILED(assemblyResult))
            return assemblyResult;
    }
    else if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
    {
        llvm::raw_svector_ostream serializedOutput(outSerializedData);
        state->module->print(serializedOutput, nullptr);
    }
    else
    {
        llvm::raw_svector_ostream serializedOutput(outSerializedData);
        llvm::WriteBitcodeToFile(*state->module, serializedOutput);
    }
    outVerificationStatus = SLANG_NVVM_VERIFICATION_VALID;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _serializeModule(
    SlangNVVMModuleHandle module,
    SlangNVVMSerializationFormat format,
    void* destination,
    size_t destinationSize,
    size_t* outSerializedSize)
{
    ModuleState* state = _getModule(module);
    llvm::SmallVector<char, 0> serializedData;
    llvm::SmallVector<char, 0> diagnosticData;
    SlangNVVMVerificationStatus verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    const SlangResult materializeResult = _materializeModule(
        state,
        format,
        false,
        serializedData,
        diagnosticData,
        verificationStatus);
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

static SlangResult _serializeModuleWithDiagnosticsImpl(
    SlangNVVMModuleHandle module,
    SlangNVVMSerializationFormat format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus* outVerificationStatus,
    bool useNVVMIR20Assembly)
{
    if (outSerializedSize)
        *outSerializedSize = 0;
    if (outDiagnosticSize)
        *outDiagnosticSize = 0;
    if (outVerificationStatus)
        *outVerificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;

    ModuleState* state = _getModule(module);
    const bool isSupportedFormat =
        useNVVMIR20Assembly ? format == SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY
                            : _isSerializationFormat(format);
    if (!state || !isSupportedFormat || !outSerializedSize || !outDiagnosticSize ||
        !outVerificationStatus || (!serializedDestination && serializedDestinationSize) ||
        (!diagnosticDestination && diagnosticDestinationSize))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::SmallVector<char, 0> serializedData;
    llvm::SmallVector<char, 0> diagnosticData;
    SlangNVVMVerificationStatus verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    const SlangResult materializeResult = _materializeModule(
        state,
        format,
        useNVVMIR20Assembly,
        serializedData,
        diagnosticData,
        verificationStatus);
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

static SlangResult SLANG_NVVM_CALL _serializeModuleWithDiagnostics(
    SlangNVVMModuleHandle module,
    SlangNVVMSerializationFormat format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus* outVerificationStatus)
{
    return _serializeModuleWithDiagnosticsImpl(
        module,
        format,
        serializedDestination,
        serializedDestinationSize,
        outSerializedSize,
        diagnosticDestination,
        diagnosticDestinationSize,
        outDiagnosticSize,
        outVerificationStatus,
        false);
}

static SlangResult SLANG_NVVM_CALL _serializeNVVMIR20AssemblyWithDiagnostics(
    SlangNVVMModuleHandle module,
    SlangNVVMSerializationFormat format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus* outVerificationStatus)
{
    return _serializeModuleWithDiagnosticsImpl(
        module,
        format,
        serializedDestination,
        serializedDestinationSize,
        outSerializedSize,
        diagnosticDestination,
        diagnosticDestinationSize,
        outDiagnosticSize,
        outVerificationStatus,
        true);
}

static SlangResult SLANG_NVVM_CALL
_isOperationSupported(const SlangNVVMValueOperationDesc* operation, uint32_t* outSupported)
{
    if (outSupported)
        *outSupported = 0;
    if (!operation || !outSupported || (!operation->operandTypes && operation->operandCount))
    {
        return SLANG_E_INVALID_ARG;
    }
    *outSupported = Slang::NVVMSemantics::isSupported(*operation) ? 1u : 0u;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_isAtomicOperationSupported(const SlangNVVMAtomicOperationDesc* operation, uint32_t* outSupported)
{
    if (outSupported)
        *outSupported = 0;
    if (!operation || !outSupported)
        return SLANG_E_INVALID_ARG;
    *outSupported = Slang::NVVMSemantics::isSupported(*operation) ? 1u : 0u;
    return SLANG_OK;
}

// These are the execution-register reads admitted by the qualified NVVM IR dialect. Both the
// named-intrinsic query and serialization validator use this classification.
static bool _isExecutionRegisterIntrinsic(llvm::Intrinsic::ID intrinsicID)
{
    switch (intrinsicID)
    {
    case llvm::Intrinsic::nvvm_read_ptx_sreg_tid_x:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_tid_y:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_tid_z:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_ctaid_x:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_ctaid_y:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_ctaid_z:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_ntid_x:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_ntid_y:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_ntid_z:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_nctaid_x:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_nctaid_y:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_nctaid_z:
        return true;
    default:
        return false;
    }
}

// Clock reads are changing observations, unlike the invariant execution-register reads above.
static bool _isClockIntrinsic(llvm::Intrinsic::ID id)
{
    return id == llvm::Intrinsic::nvvm_read_ptx_sreg_clock ||
           id == llvm::Intrinsic::nvvm_read_ptx_sreg_clock64;
}

// The registry owns signatures and convergence attributes for these scalar wave primitives.
static bool _isWaveIntrinsic(llvm::Intrinsic::ID id)
{
    switch (id)
    {
    case llvm::Intrinsic::nvvm_read_ptx_sreg_laneid:
    case llvm::Intrinsic::nvvm_read_ptx_sreg_warpsize:
    case llvm::Intrinsic::nvvm_shfl_sync_idx_i32:
    case llvm::Intrinsic::nvvm_shfl_sync_idx_f32:
    case llvm::Intrinsic::nvvm_vote_ballot_sync:
    case llvm::Intrinsic::nvvm_vote_any_sync:
    case llvm::Intrinsic::nvvm_vote_all_sync:
    case llvm::Intrinsic::nvvm_match_any_sync_i32:
        return true;
    default:
        return false;
    }
}

static llvm::Type* _getSemanticLLVMType(
    llvm::LLVMContext& context,
    const SlangNVVMValueTypeDesc& type);

// Classifies scalar Float32/Float64 values admitted by the named math paths. The LLVM registry
// or selected device-library definition still owns the complete function signature.
static bool _isScalarFloat32Or64Type(const SlangNVVMValueTypeDesc& type)
{
    return type.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT && type.laneCount == 1 &&
           (type.bitWidth == 32 || type.bitWidth == 64);
}

// Resolves a borrowed signature through LLVM's registry without creating any module state.
// Consider `llvm.ctlz` with (i32, i1): IIT matching derives the i32 overload and LLVM's ImmArg
// attribute requires the second operand's constant guarantee. The same path handles other
// admitted intrinsics, so neither signature constraints nor immediate positions are duplicated.
static llvm::Intrinsic::ID _resolveNamedIntrinsic(
    llvm::LLVMContext& context,
    const SlangNVVMNamedIntrinsicDesc& intrinsic,
    llvm::FunctionType*& outType,
    llvm::SmallVectorImpl<llvm::Type*>& outOverloadTypes)
{
    outType = nullptr;
    outOverloadTypes.clear();
    if (!intrinsic.name || !intrinsic.nameSize || (!intrinsic.operands && intrinsic.operandCount))
        return llvm::Intrinsic::not_intrinsic;
    llvm::StringRef name(intrinsic.name, intrinsic.nameSize);
    auto id = llvm::Function::lookupIntrinsicID(name);
    if (id == llvm::Intrinsic::not_intrinsic || llvm::Intrinsic::getBaseName(id) != name ||
        (!_isExecutionRegisterIntrinsic(id) && !_isSynchronizationIntrinsic(id) &&
         !_isIntegerBitIntrinsic(id) && !_isClockIntrinsic(id) && !_isWaveIntrinsic(id) &&
         !_isHalfMathIntrinsic(id) && id != llvm::Intrinsic::sqrt))
        return llvm::Intrinsic::not_intrinsic;

    const bool isSqrt = id == llvm::Intrinsic::sqrt;
    const bool isHalfMath = _isHalfMathIntrinsic(id);
    const auto& result = intrinsic.resultType;
    const bool isIntegerResult = (result.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                                  result.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
                                 result.laneCount == 1;
    if ((isSqrt && !_isScalarFloat32Or64Type(result)) ||
        (isHalfMath &&
         !Slang::NVVMSemantics::areSameType(result, Slang::NVVMSemantics::kFloat16)) ||
        ((_isIntegerBitIntrinsic(id) || _isClockIntrinsic(id)) && !isIntegerResult) ||
        (_isExecutionRegisterIntrinsic(id) && (!isIntegerResult || result.bitWidth != 32)) ||
        (_isSynchronizationIntrinsic(id) &&
         !Slang::NVVMSemantics::areSameType(result, Slang::NVVMSemantics::kVoid)))
        return llvm::Intrinsic::not_intrinsic;
    llvm::Type* resultType = _getSemanticLLVMType(context, result);
    if (!resultType)
        return llvm::Intrinsic::not_intrinsic;

    llvm::SmallVector<llvm::Type*, 2> parameterTypes;
    for (size_t i = 0; i < intrinsic.operandCount; ++i)
    {
        const auto& operand = intrinsic.operands[i];
        if (isSqrt || isHalfMath)
        {
            if (operand.kind != SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE ||
                (isHalfMath ? !Slang::NVVMSemantics::areSameType(
                                  operand.type,
                                  Slang::NVVMSemantics::kFloat16)
                            : !_isScalarFloat32Or64Type(operand.type)))
                return llvm::Intrinsic::not_intrinsic;
        }
        else
        {
            const bool isFloatingValue = operand.kind == SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE &&
                                         _isScalarFloat32Or64Type(operand.type);
            const bool isIntegerValueOrConstant =
                (operand.kind == SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE ||
                 operand.kind == SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT) &&
                operand.type.laneCount == 1 &&
                (operand.type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                 operand.type.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER ||
                 operand.type.kind == SLANG_NVVM_VALUE_TYPE_BOOL);
            if (!isFloatingValue && !isIntegerValueOrConstant)
                return llvm::Intrinsic::not_intrinsic;
        }
        llvm::Type* type = _getSemanticLLVMType(context, operand.type);
        if (!type)
            return llvm::Intrinsic::not_intrinsic;
        parameterTypes.push_back(type);
    }
    auto type = llvm::FunctionType::get(resultType, parameterTypes, false);
    llvm::SmallVector<llvm::Intrinsic::IITDescriptor, 8> descriptors;
    llvm::Intrinsic::getIntrinsicInfoTableEntries(id, descriptors);
    llvm::ArrayRef<llvm::Intrinsic::IITDescriptor> remaining = descriptors;
    if (llvm::Intrinsic::matchIntrinsicSignature(type, remaining, outOverloadTypes) !=
            llvm::Intrinsic::MatchIntrinsicTypes_Match ||
        llvm::Intrinsic::matchIntrinsicVarArg(false, remaining))
        return llvm::Intrinsic::not_intrinsic;
    const auto attributes = llvm::Intrinsic::getAttributes(context, id);
    for (size_t i = 0; i < intrinsic.operandCount; ++i)
    {
        if (attributes.hasParamAttr(unsigned(i), llvm::Attribute::ImmArg) &&
            intrinsic.operands[i].kind != SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT)
            return llvm::Intrinsic::not_intrinsic;
    }
    outType = type;
    return id;
}

static SlangResult SLANG_NVVM_CALL
_isNamedIntrinsicSupported(const SlangNVVMNamedIntrinsicDesc* intrinsic, uint32_t* outSupported)
{
    if (outSupported)
        *outSupported = 0;
    if (!intrinsic || !outSupported || (!intrinsic->operands && intrinsic->operandCount))
        return SLANG_E_INVALID_ARG;
    llvm::LLVMContext context;
    llvm::FunctionType* type = nullptr;
    llvm::SmallVector<llvm::Type*, 1> overloadTypes;
    *outSupported = _resolveNamedIntrinsic(context, *intrinsic, type, overloadTypes) !=
                    llvm::Intrinsic::not_intrinsic;
    return SLANG_OK;
}

// Preserve distinct per-SM observations after the named signature has passed LLVM registry checks.
// libNVVM 12.9 commons plain LLVM clock reads at O0 and hoists loop reads at O3. This
// side-effecting PTX preserves observations without promising a memory fence, synchronized lanes or
// cross-SM time.
static llvm::Value* _emitClockObservation(ModuleState* state, llvm::FunctionType* type)
{
    const bool is64Bit = type->getReturnType()->isIntegerTy(64);
    auto assembly = llvm::InlineAsm::get(
        type,
        is64Bit ? "mov.u64 $0, %clock64;" : "mov.u32 $0, %clock;",
        is64Bit ? "=l" : "=r",
        true);
    return state->builder.CreateCall(assembly);
}

static SlangResult SLANG_NVVM_CALL _emitNamedIntrinsic(
    SlangNVVMModuleHandle module,
    const SlangNVVMNamedIntrinsicDesc* intrinsic,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    auto state = _getModule(module);
    auto block = _getValidInsertionBlock(state);
    if (!intrinsic || !outValue || !block || (!intrinsic->operands && intrinsic->operandCount) ||
        (!operands && operandCount) || operandCount != intrinsic->operandCount)
        return SLANG_E_INVALID_ARG;
    llvm::FunctionType* type = nullptr;
    llvm::SmallVector<llvm::Type*, 1> overloadTypes;
    auto id = _resolveNamedIntrinsic(state->context, *intrinsic, type, overloadTypes);
    if (id == llvm::Intrinsic::not_intrinsic)
        return SLANG_E_NOT_AVAILABLE;
    llvm::SmallVector<llvm::Value*, 2> values;
    for (size_t i = 0; i < operandCount; ++i)
    {
        auto value = _getValue(operands[i]);
        if (!_isValueUsableAtInsertionPoint(state, block, value) ||
            value->getType() != type->getParamType(unsigned(i)) ||
            (intrinsic->operands[i].kind == SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT &&
             !llvm::isa<llvm::ConstantInt>(value)))
            return SLANG_E_INVALID_ARG;
        values.push_back(value);
    }
    const auto name = llvm::Intrinsic::getNameNoUnnamedTypes(id, overloadTypes);
    if (auto existing = state->module->getNamedValue(name))
    {
        auto function = llvm::dyn_cast<llvm::Function>(existing);
        if (!function || !function->isDeclaration() || function->getFunctionType() != type ||
            function->getIntrinsicID() != id)
            return SLANG_E_INVALID_ARG;
    }
    if (_isClockIntrinsic(id))
    {
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(_emitClockObservation(state, type));
        return SLANG_OK;
    }
    if (id == llvm::Intrinsic::trunc)
    {
        // libNVVM 12.9 rejects llvm.trunc.f16 even though LLVM's registry admits it.
        // The checked scalar Half operation maps directly to the pure PTX primitive, without
        // creating an unsupported declaration or weakening whole-module verification.
        // Its inline-assembly dialect also excludes Half values, so transport the same bits in
        // i16 registers on both sides of the instruction. These are not numeric conversions.
        auto bitsType = llvm::Type::getInt16Ty(state->context);
        auto signature = llvm::FunctionType::get(bitsType, {bitsType}, false);
        auto assembly = llvm::InlineAsm::get(signature, "cvt.rzi.f16.f16 $0, $1;", "=h,h", false);
        auto bits = state->builder.CreateBitCast(values[0], bitsType);
        auto resultBits = state->builder.CreateCall(assembly, {bits});
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(
            state->builder.CreateBitCast(resultBits, type->getReturnType()));
        return SLANG_OK;
    }
    auto declaration = llvm::Intrinsic::getDeclaration(state->module.get(), id, overloadTypes);
    auto call = state->builder.CreateCall(declaration, values);
    if (!call->getType()->isVoidTy())
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(call);
    return SLANG_OK;
}

// Maps each checked libdevice operand role once, for both capability queries and emission.
// For example, frexp's output operand carries SignedI32 as its pointee and lowers to i32* in
// generic address space. The immutable selected definition still decides the complete signature.
static llvm::Type* _getDeviceLibraryOperandType(
    llvm::LLVMContext& context,
    const SlangNVVMNamedIntrinsicOperandDesc& operand)
{
    if (operand.kind == SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE)
        return _isScalarFloat32Or64Type(operand.type) ? _getSemanticLLVMType(context, operand.type)
                                                      : nullptr;
    if (operand.kind == SLANG_NVVM_NAMED_INTRINSIC_OPERAND_OUT_POINTER &&
        (_isScalarFloat32Or64Type(operand.type) ||
         Slang::NVVMSemantics::areSameType(operand.type, Slang::NVVMSemantics::kSignedI32)))
        return llvm::PointerType::getUnqual(_getSemanticLLVMType(context, operand.type));
    return nullptr;
}

// The selected definition is the signature authority. The admitted names bound the supported
// scalar math functions; no parallel name-to-signature mapping exists here. For example,
// __nv_roundf accepts float(float) only because that is the definition present in the immutable
// selected bitcode.
static llvm::FunctionType* _resolveDeviceLibraryFunction(
    DeviceLibraryState* library,
    const SlangNVVMNamedIntrinsicDesc& desc)
{
    if (!library || !desc.name || (!desc.operands && desc.operandCount))
        return nullptr;
    llvm::StringRef name(desc.name, desc.nameSize);
    if (name != "__nv_roundf" && name != "__nv_round" && name != "__nv_ceilf" &&
        name != "__nv_ceil" && name != "__nv_floorf" && name != "__nv_floor" &&
        name != "__nv_truncf" && name != "__nv_trunc" && name != "__nv_rsqrtf" &&
        name != "__nv_rsqrt" && name != "__nv_expf" && name != "__nv_exp" && name != "__nv_exp2f" &&
        name != "__nv_exp2" && name != "__nv_logf" && name != "__nv_log" && name != "__nv_log2f" &&
        name != "__nv_log2" && name != "__nv_log10f" && name != "__nv_log10" &&
        name != "__nv_sinf" && name != "__nv_sin" && name != "__nv_cosf" && name != "__nv_cos" &&
        name != "__nv_acosf" && name != "__nv_acos" && name != "__nv_asinf" &&
        name != "__nv_asin" && name != "__nv_atanf" && name != "__nv_atan" &&
        name != "__nv_atan2f" && name != "__nv_atan2" && name != "__nv_powf" &&
        name != "__nv_pow" && name != "__nv_tanf" && name != "__nv_tan" && name != "__nv_sinhf" &&
        name != "__nv_sinh" && name != "__nv_coshf" && name != "__nv_cosh" &&
        name != "__nv_tanhf" && name != "__nv_tanh" && name != "__nv_fmaf" && name != "__nv_fma" &&
        name != "__nv_fmodf" && name != "__nv_fmod" && name != "__nv_fabsf" &&
        name != "__nv_fabs" && name != "__nv_fminf" && name != "__nv_fmin" &&
        name != "__nv_fmaxf" && name != "__nv_fmax" && name != "__nv_frexpf" &&
        name != "__nv_frexp" && name != "__nv_modff" && name != "__nv_modf")
        return nullptr;
    auto function = library->module->getFunction(name);
    if (!function || function->isDeclaration() || !function->hasExternalLinkage() ||
        function->getVisibility() != llvm::GlobalValue::DefaultVisibility || function->isVarArg() ||
        function->getCallingConv() != llvm::CallingConv::C)
        return nullptr;
    const auto attributes = function->getAttributes();
    if (attributes.getRetAttrs().hasAttributes())
        return nullptr;
    if (!_isScalarFloat32Or64Type(desc.resultType))
        return nullptr;
    llvm::SmallVector<llvm::Type*, 3> parameters;
    for (size_t i = 0; i < desc.operandCount; ++i)
    {
        auto type = _getDeviceLibraryOperandType(library->context, desc.operands[i]);
        if (!type || attributes.getParamAttrs(unsigned(i)).hasAttributes())
            return nullptr;
        parameters.push_back(type);
    }
    auto requested = llvm::FunctionType::get(
        _getSemanticLLVMType(library->context, desc.resultType),
        parameters,
        false);
    return requested == function->getFunctionType() ? requested : nullptr;
}

static SlangResult SLANG_NVVM_CALL _isDeviceLibraryFunctionSupported(
    SlangNVVMDeviceLibraryHandle library,
    const SlangNVVMNamedIntrinsicDesc* function,
    uint32_t* outSupported)
{
    if (outSupported)
        *outSupported = 0;
    if (!library || !function || !outSupported)
        return SLANG_E_INVALID_ARG;
    *outSupported =
        _resolveDeviceLibraryFunction(reinterpret_cast<DeviceLibraryState*>(library), *function) !=
        nullptr;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitDeviceLibraryFunction(
    SlangNVVMDeviceLibraryHandle library,
    SlangNVVMModuleHandle module,
    const SlangNVVMNamedIntrinsicDesc* function,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    auto state = _getModule(module);
    auto block = _getValidInsertionBlock(state);
    if (!function || !outValue || !block || (!operands && operandCount) ||
        operandCount != function->operandCount ||
        !_resolveDeviceLibraryFunction(reinterpret_cast<DeviceLibraryState*>(library), *function))
        return SLANG_E_INVALID_ARG;
    llvm::SmallVector<llvm::Value*, 3> values;
    llvm::SmallVector<llvm::Type*, 3> parameters;
    for (size_t i = 0; i < operandCount; ++i)
    {
        auto type = _getDeviceLibraryOperandType(state->context, function->operands[i]);
        auto value = _getValue(operands[i]);
        if (!_isValueUsableAtInsertionPoint(state, block, value) || value->getType() != type)
            return SLANG_E_INVALID_ARG;
        parameters.push_back(type);
        values.push_back(value);
    }
    auto type = llvm::FunctionType::get(
        _getSemanticLLVMType(state->context, function->resultType),
        parameters,
        false);
    llvm::StringRef name(function->name, function->nameSize);
    llvm::Function* declaration = nullptr;
    if (auto existing = state->module->getNamedValue(name))
    {
        declaration = llvm::dyn_cast<llvm::Function>(existing);
        if (!declaration || !declaration->isDeclaration() || !declaration->hasExternalLinkage() ||
            declaration->getVisibility() != llvm::GlobalValue::DefaultVisibility ||
            declaration->getFunctionType() != type ||
            declaration->getCallingConv() != llvm::CallingConv::C ||
            declaration->getAttributes().getRetAttrs().hasAttributes())
            return SLANG_E_INVALID_ARG;
        for (size_t i = 0; i < operandCount; ++i)
            if (declaration->getAttributes().getParamAttrs(unsigned(i)).hasAttributes())
                return SLANG_E_INVALID_ARG;
    }
    if (!declaration)
        declaration = llvm::Function::Create(
            type,
            llvm::GlobalValue::ExternalLinkage,
            name,
            state->module.get());
    *outValue =
        reinterpret_cast<SlangNVVMValueHandle>(state->builder.CreateCall(declaration, values));
    return SLANG_OK;
}

static const char* _getLibdeviceFunctionName(
    SlangNVVMValueOperation operation,
    uint32_t bitWidth,
    size_t operandCount)
{
    const bool isFloat32 = bitWidth == 32;
    const bool isFloat64 = bitWidth == 64;
    if (!isFloat32 && !isFloat64)
        return nullptr;
    if (operandCount == 2)
    {
        switch (operation)
        {
        case SLANG_NVVM_VALUE_OP_FMOD:
            return isFloat32 ? "__nv_fmodf" : "__nv_fmod";
        default:
            return nullptr;
        }
    }
    return nullptr;
}

// Emits a selected scalar libdevice operation without reconstructing its type from LLVM text.
static SlangResult _emitLibdeviceOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMValueOperationDesc& operation,
    const SlangNVVMValueHandle* operands,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    const bool isFloat32 = operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                           operation.resultType.bitWidth == 32 &&
                           operation.resultType.laneCount == 1;
    const bool isFloat64 = operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                           operation.resultType.bitWidth == 64 &&
                           operation.resultType.laneCount == 1;
    const char* functionName = _getLibdeviceFunctionName(
        operation.operation,
        operation.resultType.bitWidth,
        operation.operandCount);
    if (!state || !insertionBlock || !operands || !outValue || !functionName ||
        (!isFloat32 && !isFloat64) || operation.operandCount < 1 || operation.operandCount > 3)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Type* resultType = isFloat32 ? llvm::Type::getFloatTy(state->context)
                                       : llvm::Type::getDoubleTy(state->context);
    llvm::SmallVector<llvm::Value*, 3> llvmOperands;
    llvm::SmallVector<llvm::Type*, 3> parameterTypes;
    for (size_t i = 0; i < operation.operandCount; ++i)
    {
        llvm::Value* operand = _getValue(operands[i]);
        if (!Slang::NVVMSemantics::areSameType(operation.resultType, operation.operandTypes[i]) ||
            !operand || operand->getType() != resultType ||
            !_isValueUsableAtInsertionPoint(state, insertionBlock, operand))
        {
            return SLANG_E_INVALID_ARG;
        }
        llvmOperands.push_back(operand);
        parameterTypes.push_back(resultType);
    }

    llvm::FunctionType* functionType = llvm::FunctionType::get(resultType, parameterTypes, false);
    llvm::Function* function = state->module->getFunction(functionName);
    if (function && (function->getFunctionType() != functionType || !function->isDeclaration()))
        return SLANG_E_INVALID_ARG;
    if (!function)
    {
        function = llvm::Function::Create(
            functionType,
            llvm::GlobalValue::ExternalLinkage,
            functionName,
            state->module.get());
    }

    llvm::Value* result = state->builder.CreateCall(function, llvmOperands);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult _emitCatalogOperation(
    SlangNVVMModuleHandle module,
    const Slang::NVVMSemantics::CatalogEntry& entry,
    const SlangNVVMValueHandle* operands,
    SlangNVVMValueHandle* outValue)
{
    const SlangNVVMValueOperationDesc operation = Slang::NVVMSemantics::getOperationDesc(entry);
    switch (entry.operation)
    {
    case SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK:
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT:
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH:
        return _emitIntrinsic(module, operation, operands, entry.operandCount, outValue);
    default:
        return SLANG_E_INVALID_ARG;
    }
}

// Selects the target's round-to-nearest-even conversion for Float32-to-Half values. Consider
// `half(asfloat(0x7f800001u))`: libNVVM 12.9 at O3 folds fptrunc of this signaling NaN to infinity
// when its retained payload becomes zero. The target conversion preserves NaN classification.
// This selection belongs to generic floating conversion; surfaces only supply a FloatCast in IR.
static llvm::Value* _emitFloat32ToHalf(
    ModuleState* state,
    llvm::Value* value,
    llvm::Type* resultType)
{
    auto intrinsic =
        llvm::Intrinsic::getDeclaration(state->module.get(), llvm::Intrinsic::nvvm_f2h_rn);
    if (resultType->isHalfTy())
        return state->builder.CreateBitCast(
            state->builder.CreateCall(intrinsic, {value}),
            resultType);

    auto vectorType = llvm::cast<llvm::FixedVectorType>(resultType);
    llvm::SmallVector<llvm::Value*, 4> elements;
    for (uint32_t lane = 0; lane < vectorType->getNumElements(); ++lane)
    {
        auto operand = state->builder.CreateExtractElement(value, lane);
        auto bits = state->builder.CreateCall(intrinsic, {operand});
        elements.push_back(state->builder.CreateBitCast(bits, vectorType->getElementType()));
    }
    return _createNVVMVectorConstruct(state, vectorType, elements);
}

// Converts one qualified BF16/Float32 lane using the scalar SM80 contract. Vector
// conversion extracts each canonical lane and reuses this recipe, so widening preserves
// signaling-NaN payloads and narrowing rounds exactly once, including subnormals.
static llvm::Value* _emitBFloat16ConvertLane(
    ModuleState* state,
    llvm::Value* operand,
    llvm::Type* resultType,
    bool narrow)
{
    if (narrow)
    {
        // SM80 narrows once with round-to-nearest-even, including subnormals.
        auto signature = llvm::FunctionType::get(resultType, {operand->getType()}, false);
        auto assembly = llvm::InlineAsm::get(signature, "cvt.rn.bf16.f32 $0, $1;", "=h,f", false);
        return state->builder.CreateCall(assembly, {operand});
    }
    else
    {
        // CUDA's SM80 BF16 expansion places every payload, including signaling NaNs,
        // in the high word of Float32. It is bit transport, not integer conversion.
        auto int32Type = llvm::Type::getInt32Ty(state->context);
        auto bits = state->builder.CreateZExt(operand, int32Type);
        bits = state->builder.CreateShl(bits, llvm::ConstantInt::get(int32Type, 16));
        return state->builder.CreateBitCast(bits, resultType);
    }
}

// Widens a qualified scalar FP8 encoding to exact Float32 bits. Consider E4M3 byte 1:
// its zero exponent selects the subnormal fraction 1 * 2^-9. Multiplying this small
// integer by a power of two is exact in Float32, and applying the sign afterward also
// preserves negative zero. Normal encodings only adjust exponent/fraction positions.
// E4M3's final exponent still has finite values; only magnitude 127 is NaN. E5M2's
// final exponent distinguishes infinity from NaN. NaN payload and sign are unspecified.
static llvm::Value* _emitFloat8Widen(
    ModuleState* state,
    llvm::Value* operand,
    llvm::Type* resultType,
    bool isE4M3)
{
    const uint32_t fractionBitCount = isE4M3 ? 3 : 2;
    const uint32_t exponentBias = isE4M3 ? 7 : 15;
    const uint32_t lastFiniteMagnitude = isE4M3 ? 126 : 123;
    auto int32Type = llvm::Type::getInt32Ty(state->context);
    auto bits = state->builder.CreateZExt(operand, int32Type);
    auto sign = state->builder.CreateShl(state->builder.CreateAnd(bits, 128), 24);
    auto magnitude = state->builder.CreateAnd(bits, 127);
    auto exponent = state->builder.CreateLShr(magnitude, fractionBitCount);
    auto fraction = state->builder.CreateAnd(magnitude, (1u << fractionBitCount) - 1);

    auto subnormal = state->builder.CreateFMul(
        state->builder.CreateUIToFP(fraction, resultType),
        llvm::ConstantFP::get(resultType, isE4M3 ? 0.001953125 : 0.0000152587890625));
    auto subnormalBits = state->builder.CreateBitCast(subnormal, int32Type);
    auto normalBits = state->builder.CreateOr(
        state->builder.CreateShl(
            state->builder.CreateAdd(
                exponent,
                llvm::ConstantInt::get(int32Type, 127 - exponentBias)),
            23),
        state->builder.CreateShl(fraction, 23 - fractionBitCount));
    auto finiteBits = state->builder.CreateOr(
        state->builder.CreateSelect(
            state->builder.CreateICmpEQ(exponent, llvm::ConstantInt::get(int32Type, 0)),
            subnormalBits,
            normalBits),
        sign);

    llvm::Value* specialBits = llvm::ConstantInt::get(int32Type, 0x7fc00000);
    if (!isE4M3)
    {
        specialBits = state->builder.CreateSelect(
            state->builder.CreateICmpEQ(fraction, llvm::ConstantInt::get(int32Type, 0)),
            state->builder.CreateOr(sign, 0x7f800000),
            specialBits);
    }
    auto resultBits = state->builder.CreateSelect(
        state->builder.CreateICmpUGT(
            magnitude,
            llvm::ConstantInt::get(int32Type, lastFiniteMagnitude)),
        specialBits,
        finiteBits);
    return state->builder.CreateBitCast(resultBits, resultType);
}

// Emits one scalar BF16 rounding boundary. SM80 supports this FMA even though native BF16
// add/multiply require SM90; core composition supplies their exact FMA identities and order.
static llvm::Value* _emitBFloat16Fma(
    ModuleState* state,
    llvm::Value* left,
    llvm::Value* right,
    llvm::Value* addend)
{
    auto scalarType = llvm::Type::getInt16Ty(state->context);
    auto signature =
        llvm::FunctionType::get(scalarType, {scalarType, scalarType, scalarType}, false);
    auto fma = llvm::InlineAsm::get(signature, "fma.rn.bf16 $0, $1, $2, $3;", "=h,h,h,h", false);
    return state->builder.CreateCall(fma, {left, right, addend});
}

static llvm::Type* _getSemanticLLVMType(
    llvm::LLVMContext& context,
    const SlangNVVMValueTypeDesc& type)
{
    llvm::Type* scalarType = nullptr;
    switch (type.kind)
    {
    case SLANG_NVVM_VALUE_TYPE_VOID:
        return Slang::NVVMSemantics::areSameType(type, Slang::NVVMSemantics::kVoid)
                   ? llvm::Type::getVoidTy(context)
                   : nullptr;
    case SLANG_NVVM_VALUE_TYPE_BOOL:
        if (type.bitWidth == 1)
            scalarType = llvm::Type::getInt1Ty(context);
        break;
    case SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER:
    case SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER:
        if (type.bitWidth == 8 || type.bitWidth == 16 || type.bitWidth == 32 || type.bitWidth == 64)
        {
            scalarType = llvm::IntegerType::get(context, type.bitWidth);
        }
        break;
    case SLANG_NVVM_VALUE_TYPE_FLOAT_E4M3:
    case SLANG_NVVM_VALUE_TYPE_FLOAT_E5M2:
        if (type.bitWidth == 8 && type.laneCount == 1)
            scalarType = llvm::Type::getInt8Ty(context);
        break;
    case SLANG_NVVM_VALUE_TYPE_BFLOAT16:
        if (type.bitWidth == 16)
            scalarType = llvm::Type::getInt16Ty(context);
        break;
    case SLANG_NVVM_VALUE_TYPE_FLOATING_POINT:
        if (type.bitWidth == 16)
            scalarType = llvm::Type::getHalfTy(context);
        else if (type.bitWidth == 32)
            scalarType = llvm::Type::getFloatTy(context);
        else if (type.bitWidth == 64)
            scalarType = llvm::Type::getDoubleTy(context);
        break;
    default:
        break;
    }
    if (!scalarType)
        return nullptr;
    if (type.laneCount == 1)
        return scalarType;
    if ((type.kind == SLANG_NVVM_VALUE_TYPE_BOOL ||
         type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
         type.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER ||
         type.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ||
         type.kind == SLANG_NVVM_VALUE_TYPE_BFLOAT16) &&
        type.laneCount >= 2 && type.laneCount <= 4)
    {
        return llvm::FixedVectorType::get(scalarType, type.laneCount);
    }
    return nullptr;
}

static llvm::Type* _getSemanticLLVMType(ModuleState* state, const SlangNVVMValueTypeDesc& type)
{
    return state ? _getSemanticLLVMType(state->context, type) : nullptr;
}

// Materializes the physical vector operand required by LLVM for one validated scalar broadcast.
// LLVM 14's `CreateVectorSplat` starts from `poison`, which libNVVM's LLVM 7 reader cannot parse.
// Inserting the scalar into every bounded lane from `undef` preserves the exact splat semantics in
// both textual dialects and lets libNVVM optimize the ordinary vector construction.
static llvm::Value* _materializeBroadcastOperand(
    ModuleState* state,
    llvm::Value* value,
    const SlangNVVMValueTypeDesc& declaredType,
    uint32_t operationLaneCount)
{
    if (!state || !value)
        return nullptr;
    if (declaredType.laneCount == operationLaneCount)
        return value;
    if (declaredType.laneCount != 1 || operationLaneCount < 2 || operationLaneCount > 4)
        return nullptr;
    llvm::SmallVector<llvm::Value*, 4> elements;
    for (uint32_t lane = 0; lane < operationLaneCount; ++lane)
        elements.push_back(value);
    llvm::FixedVectorType* vectorType =
        llvm::FixedVectorType::get(value->getType(), operationLaneCount);
    return _createNVVMVectorConstruct(state, vectorType, elements);
}

static SlangResult _emitValueOperationFamily(
    SlangNVVMModuleHandle module,
    const SlangNVVMValueOperationDesc& operation,
    Slang::NVVMSemantics::ValueOperationFamily family,
    const SlangNVVMValueHandle* operands,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    ModuleState* state = _getModule(module);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::Type* resultType = _getSemanticLLVMType(state, operation.resultType);
    if (!state || !insertionBlock || !resultType || !outValue)
        return SLANG_E_INVALID_ARG;

    llvm::Value* llvmOperands[3] = {};
    for (size_t i = 0; i < operation.operandCount; ++i)
    {
        llvmOperands[i] = _getValue(operands[i]);
        llvm::Type* expectedType = _getSemanticLLVMType(state, operation.operandTypes[i]);
        if (!llvmOperands[i] || llvmOperands[i]->getType() != expectedType ||
            !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmOperands[i]))
        {
            return SLANG_E_INVALID_ARG;
        }
    }

    const bool isBroadcastFamily =
        family == Slang::NVVMSemantics::ValueOperationFamily::IntegerBinary ||
        family == Slang::NVVMSemantics::ValueOperationFamily::IntegerCompare ||
        family == Slang::NVVMSemantics::ValueOperationFamily::FloatBinary ||
        family == Slang::NVVMSemantics::ValueOperationFamily::FloatCompare ||
        family == Slang::NVVMSemantics::ValueOperationFamily::BooleanBinary ||
        family == Slang::NVVMSemantics::ValueOperationFamily::BooleanCompare;
    if (isBroadcastFamily)
    {
        for (size_t i = 0; i < operation.operandCount; ++i)
        {
            llvmOperands[i] = _materializeBroadcastOperand(
                state,
                llvmOperands[i],
                operation.operandTypes[i],
                operation.resultType.laneCount);
            if (!llvmOperands[i])
                return SLANG_E_INVALID_ARG;
        }
    }

    const bool isIntegerShift =
        family == Slang::NVVMSemantics::ValueOperationFamily::IntegerBinary &&
        (operation.operation == SLANG_NVVM_VALUE_OP_SHIFT_LEFT ||
         operation.operation == SLANG_NVVM_VALUE_OP_SHIFT_RIGHT);
    if (isIntegerShift)
    {
        // Slang preserves the source shift-count width independently from the shifted value. LLVM
        // requires both operands to have the same physical type, so normalize the already-checked
        // scalar or component-wise count without changing the source operation's signedness.
        llvmOperands[1] = _normalizeNVVMInteger16Operand(state, llvmOperands[1], false);
        if (llvmOperands[1]->getType() != resultType)
        {
            llvmOperands[1] = state->builder.CreateZExtOrTrunc(llvmOperands[1], resultType);
            llvmOperands[1] = _normalizeNVVMInteger16Operand(state, llvmOperands[1], false);
        }
    }

    // Low-bit arithmetic, storage, selects and same-width casts preserve modular bits directly.
    // Comparisons, division, right shifts and widening instead consume the represented value.
    // Normalize at those consumers so a signed-to-unsigned reinterpretation uses its new role,
    // including values arriving through loads, parameters, helper results and vector lanes.
    const bool normalizeIntegerOperands =
        family == Slang::NVVMSemantics::ValueOperationFamily::IntegerCompare ||
        (family == Slang::NVVMSemantics::ValueOperationFamily::IntegerBinary &&
         (operation.operation == SLANG_NVVM_VALUE_OP_DIVIDE ||
          operation.operation == SLANG_NVVM_VALUE_OP_REMAINDER ||
          operation.operation == SLANG_NVVM_VALUE_OP_SHIFT_RIGHT));
    const bool normalizeIntegerConversion =
        family == Slang::NVVMSemantics::ValueOperationFamily::IntegerToFloat ||
        (family == Slang::NVVMSemantics::ValueOperationFamily::IntegerConvert &&
         operation.resultType.bitWidth > operation.operandTypes[0].bitWidth);
    if (normalizeIntegerOperands || normalizeIntegerConversion)
    {
        const size_t operandCount =
            normalizeIntegerConversion || isIntegerShift ? 1 : operation.operandCount;
        for (size_t i = 0; i < operandCount; ++i)
        {
            llvmOperands[i] = _normalizeNVVMInteger16Operand(
                state,
                llvmOperands[i],
                operation.operandTypes[i].kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER);
        }
    }

    llvm::Value* result = nullptr;
    switch (family)
    {
    case Slang::NVVMSemantics::ValueOperationFamily::IntegerUnary:
        result = operation.operation == SLANG_NVVM_VALUE_OP_BIT_NOT
                     ? state->builder.CreateNot(llvmOperands[0])
                     : state->builder.CreateNeg(llvmOperands[0]);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::IntegerBinary:
        switch (operation.operation)
        {
        case SLANG_NVVM_VALUE_OP_ADD:
            result = state->builder.CreateAdd(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_SUBTRACT:
            result = state->builder.CreateSub(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_MULTIPLY:
            result = state->builder.CreateMul(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_DIVIDE:
            result = operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                         ? state->builder.CreateSDiv(llvmOperands[0], llvmOperands[1])
                         : state->builder.CreateUDiv(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_BIT_AND:
            result = state->builder.CreateAnd(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_BIT_OR:
            result = state->builder.CreateOr(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_BIT_XOR:
            result = state->builder.CreateXor(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_REMAINDER:
            result = operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                         ? state->builder.CreateSRem(llvmOperands[0], llvmOperands[1])
                         : state->builder.CreateURem(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_SHIFT_LEFT:
            result = state->builder.CreateShl(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_SHIFT_RIGHT:
            result = operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                         ? state->builder.CreateAShr(llvmOperands[0], llvmOperands[1])
                         : state->builder.CreateLShr(llvmOperands[0], llvmOperands[1]);
            break;
        default:
            return SLANG_E_INVALID_ARG;
        }
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::FloatUnary:
        // LLVM 14 prints fneg, which libNVVM's LLVM 7 reader cannot parse.
        result = state->builder.CreateFSub(
            llvm::ConstantFP::getNegativeZero(resultType),
            llvmOperands[0]);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::FloatBinary:
        switch (operation.operation)
        {
        case SLANG_NVVM_VALUE_OP_ADD:
            result = state->builder.CreateFAdd(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_SUBTRACT:
            result = state->builder.CreateFSub(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_MULTIPLY:
            result = state->builder.CreateFMul(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_DIVIDE:
            result = state->builder.CreateFDiv(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_REMAINDER:
            result = state->builder.CreateFRem(llvmOperands[0], llvmOperands[1]);
            break;
        case SLANG_NVVM_VALUE_OP_FMOD:
            return _emitLibdeviceOperation(module, operation, operands, outValue);
        default:
            return SLANG_E_INVALID_ARG;
        }
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::IntegerCompare:
        {
            llvm::CmpInst::Predicate predicate = llvm::CmpInst::BAD_ICMP_PREDICATE;
            const bool isSigned =
                operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER;
            switch (operation.operation)
            {
            case SLANG_NVVM_VALUE_OP_EQUAL:
                predicate = llvm::CmpInst::ICMP_EQ;
                break;
            case SLANG_NVVM_VALUE_OP_NOT_EQUAL:
                predicate = llvm::CmpInst::ICMP_NE;
                break;
            case SLANG_NVVM_VALUE_OP_LESS_THAN:
                predicate = isSigned ? llvm::CmpInst::ICMP_SLT : llvm::CmpInst::ICMP_ULT;
                break;
            case SLANG_NVVM_VALUE_OP_GREATER_THAN:
                predicate = isSigned ? llvm::CmpInst::ICMP_SGT : llvm::CmpInst::ICMP_UGT;
                break;
            case SLANG_NVVM_VALUE_OP_LESS_EQUAL:
                predicate = isSigned ? llvm::CmpInst::ICMP_SLE : llvm::CmpInst::ICMP_ULE;
                break;
            case SLANG_NVVM_VALUE_OP_GREATER_EQUAL:
                predicate = isSigned ? llvm::CmpInst::ICMP_SGE : llvm::CmpInst::ICMP_UGE;
                break;
            default:
                return SLANG_E_INVALID_ARG;
            }
            result = state->builder.CreateICmp(predicate, llvmOperands[0], llvmOperands[1]);
        }
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::FloatCompare:
        {
            const llvm::CmpInst::Predicate predicate =
                _getFloatingComparePredicate(operation.operation);
            if (predicate == llvm::CmpInst::BAD_FCMP_PREDICATE)
                return SLANG_E_INVALID_ARG;
            result = state->builder.CreateFCmp(predicate, llvmOperands[0], llvmOperands[1]);
        }
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::BooleanUnary:
        result = state->builder.CreateNot(llvmOperands[0]);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::BooleanBinary:
        result = operation.operation == SLANG_NVVM_VALUE_OP_BIT_AND
                     ? state->builder.CreateAnd(llvmOperands[0], llvmOperands[1])
                     : state->builder.CreateOr(llvmOperands[0], llvmOperands[1]);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::BooleanCompare:
        result = state->builder.CreateICmp(
            operation.operation == SLANG_NVVM_VALUE_OP_EQUAL ? llvm::CmpInst::ICMP_EQ
                                                             : llvm::CmpInst::ICMP_NE,
            llvmOperands[0],
            llvmOperands[1]);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::IntegerConvert:
        if (operation.resultType.bitWidth == operation.operandTypes[0].bitWidth)
        {
            result = llvmOperands[0];
        }
        else if (operation.resultType.bitWidth < operation.operandTypes[0].bitWidth)
        {
            result = state->builder.CreateTrunc(llvmOperands[0], resultType);
        }
        else if (operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER)
        {
            result = state->builder.CreateSExt(llvmOperands[0], resultType);
        }
        else
        {
            result = state->builder.CreateZExt(llvmOperands[0], resultType);
        }
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::IntegerToFloat:
        result = operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                     ? state->builder.CreateSIToFP(llvmOperands[0], resultType)
                     : state->builder.CreateUIToFP(llvmOperands[0], resultType);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::FloatToInteger:
        result = operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                     ? state->builder.CreateFPToSI(llvmOperands[0], resultType)
                     : state->builder.CreateFPToUI(llvmOperands[0], resultType);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::BFloat16Convert:
        {
            const bool narrow = operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_BFLOAT16;
            if (operation.resultType.laneCount == 1)
                result = _emitBFloat16ConvertLane(state, llvmOperands[0], resultType, narrow);
            else
            {
                auto vectorType = llvm::cast<llvm::FixedVectorType>(resultType);
                result = llvm::UndefValue::get(vectorType);
                for (uint32_t lane = 0; lane < operation.resultType.laneCount; ++lane)
                {
                    auto operand = state->builder.CreateExtractElement(llvmOperands[0], lane);
                    auto converted = _emitBFloat16ConvertLane(
                        state,
                        operand,
                        vectorType->getElementType(),
                        narrow);
                    result = state->builder.CreateInsertElement(result, converted, lane);
                }
            }
        }
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::Float8Widen:
        result = _emitFloat8Widen(
            state,
            llvmOperands[0],
            resultType,
            operation.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_FLOAT_E4M3);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::BFloat16Fma:
        result = _emitBFloat16Fma(state, llvmOperands[0], llvmOperands[1], llvmOperands[2]);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::FloatConvert:
        if (operation.resultType.bitWidth == 16 && operation.operandTypes[0].bitWidth == 32)
            result = _emitFloat32ToHalf(state, llvmOperands[0], resultType);
        else
            result = operation.resultType.bitWidth < operation.operandTypes[0].bitWidth
                         ? state->builder.CreateFPTrunc(llvmOperands[0], resultType)
                         : state->builder.CreateFPExt(llvmOperands[0], resultType);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::BitReinterpret:
        result = llvmOperands[0]->getType() == resultType
                     ? llvmOperands[0]
                     : state->builder.CreateBitCast(llvmOperands[0], resultType);
        break;
    case Slang::NVVMSemantics::ValueOperationFamily::Select:
        result = state->builder.CreateSelect(llvmOperands[0], llvmOperands[1], llvmOperands[2]);
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }

    if (!result || result->getType() != resultType)
        return SLANG_E_INVALID_ARG;
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMValueOperationDesc* operation,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    if (!operation || !outValue || operation->operandCount != operandCount ||
        (!operands && operandCount))
    {
        return SLANG_E_INVALID_ARG;
    }

    Slang::NVVMSemantics::ValueOperationFamilyResolution resolution;
    if (Slang::NVVMSemantics::resolveValueOperationFamily(*operation, resolution))
        return _emitValueOperationFamily(module, *operation, resolution.family, operands, outValue);

    const Slang::NVVMSemantics::CatalogEntry* entry = Slang::NVVMSemantics::find(*operation);
    return entry ? _emitCatalogOperation(module, *entry, operands, outValue) : SLANG_E_INVALID_ARG;
}

static bool _isSurfaceOperationSupported(const SlangNVVMSurfaceOperationDesc& operation)
{
    const bool isSupportedShape = operation.shape == SLANG_NVVM_TEXTURE_SHAPE_1D ||
                                  operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D ||
                                  operation.shape == SLANG_NVVM_TEXTURE_SHAPE_3D;
    const bool is32BitNumeric =
        operation.elementType.bitWidth == 32 &&
        (operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ||
         operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
         operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER);
    if ((operation.operation != SLANG_NVVM_SURFACE_OP_LOAD &&
         operation.operation != SLANG_NVVM_SURFACE_OP_STORE) ||
        !isSupportedShape || operation.isArray > 1 ||
        (operation.isArray && operation.shape != SLANG_NVVM_TEXTURE_SHAPE_1D &&
         operation.shape != SLANG_NVVM_TEXTURE_SHAPE_2D) ||
        (operation.elementType.laneCount != 1 && operation.elementType.laneCount != 2 &&
         operation.elementType.laneCount != 4) ||
        operation.boundaryMode != SLANG_NVVM_SURFACE_BOUNDARY_ZERO)
    {
        return false;
    }
    return is32BitNumeric || (operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                              operation.elementType.bitWidth == 16 && !operation.isArray &&
                              operation.shape != SLANG_NVVM_TEXTURE_SHAPE_3D);
}

static SlangResult SLANG_NVVM_CALL
_isSurfaceOperationSupported(const SlangNVVMSurfaceOperationDesc* operation, uint32_t* outSupported)
{
    if (outSupported)
        *outSupported = 0;
    if (!operation || !outSupported)
        return SLANG_E_INVALID_ARG;
    *outSupported = _isSurfaceOperationSupported(*operation) ? 1u : 0u;
    return SLANG_OK;
}

static llvm::Intrinsic::ID _getSurfaceIntrinsicID(const SlangNVVMSurfaceOperationDesc& operation)
{
    if (!_isSurfaceOperationSupported(operation))
        return llvm::Intrinsic::not_intrinsic;
    static const llvm::Intrinsic::ID kLoadI16[2][3] = {
        {llvm::Intrinsic::nvvm_suld_1d_i16_zero,
         llvm::Intrinsic::nvvm_suld_1d_v2i16_zero,
         llvm::Intrinsic::nvvm_suld_1d_v4i16_zero},
        {llvm::Intrinsic::nvvm_suld_2d_i16_zero,
         llvm::Intrinsic::nvvm_suld_2d_v2i16_zero,
         llvm::Intrinsic::nvvm_suld_2d_v4i16_zero},
    };
    static const llvm::Intrinsic::ID kStoreI16[2][3] = {
        {llvm::Intrinsic::nvvm_sust_b_1d_i16_zero,
         llvm::Intrinsic::nvvm_sust_b_1d_v2i16_zero,
         llvm::Intrinsic::nvvm_sust_b_1d_v4i16_zero},
        {llvm::Intrinsic::nvvm_sust_b_2d_i16_zero,
         llvm::Intrinsic::nvvm_sust_b_2d_v2i16_zero,
         llvm::Intrinsic::nvvm_sust_b_2d_v4i16_zero},
    };
    static const llvm::Intrinsic::ID kLoadI32[3][3] = {
        {llvm::Intrinsic::nvvm_suld_1d_i32_zero,
         llvm::Intrinsic::nvvm_suld_1d_v2i32_zero,
         llvm::Intrinsic::nvvm_suld_1d_v4i32_zero},
        {llvm::Intrinsic::nvvm_suld_2d_i32_zero,
         llvm::Intrinsic::nvvm_suld_2d_v2i32_zero,
         llvm::Intrinsic::nvvm_suld_2d_v4i32_zero},
        {llvm::Intrinsic::nvvm_suld_3d_i32_zero,
         llvm::Intrinsic::nvvm_suld_3d_v2i32_zero,
         llvm::Intrinsic::nvvm_suld_3d_v4i32_zero},
    };
    static const llvm::Intrinsic::ID kStoreI32[3][3] = {
        {llvm::Intrinsic::nvvm_sust_b_1d_i32_zero,
         llvm::Intrinsic::nvvm_sust_b_1d_v2i32_zero,
         llvm::Intrinsic::nvvm_sust_b_1d_v4i32_zero},
        {llvm::Intrinsic::nvvm_sust_b_2d_i32_zero,
         llvm::Intrinsic::nvvm_sust_b_2d_v2i32_zero,
         llvm::Intrinsic::nvvm_sust_b_2d_v4i32_zero},
        {llvm::Intrinsic::nvvm_sust_b_3d_i32_zero,
         llvm::Intrinsic::nvvm_sust_b_3d_v2i32_zero,
         llvm::Intrinsic::nvvm_sust_b_3d_v4i32_zero},
    };
    static const llvm::Intrinsic::ID kLoadI32Array[2][3] = {
        {llvm::Intrinsic::nvvm_suld_1d_array_i32_zero,
         llvm::Intrinsic::nvvm_suld_1d_array_v2i32_zero,
         llvm::Intrinsic::nvvm_suld_1d_array_v4i32_zero},
        {llvm::Intrinsic::nvvm_suld_2d_array_i32_zero,
         llvm::Intrinsic::nvvm_suld_2d_array_v2i32_zero,
         llvm::Intrinsic::nvvm_suld_2d_array_v4i32_zero},
    };
    static const llvm::Intrinsic::ID kStoreI32Array[2][3] = {
        {llvm::Intrinsic::nvvm_sust_b_1d_array_i32_zero,
         llvm::Intrinsic::nvvm_sust_b_1d_array_v2i32_zero,
         llvm::Intrinsic::nvvm_sust_b_1d_array_v4i32_zero},
        {llvm::Intrinsic::nvvm_sust_b_2d_array_i32_zero,
         llvm::Intrinsic::nvvm_sust_b_2d_array_v2i32_zero,
         llvm::Intrinsic::nvvm_sust_b_2d_array_v4i32_zero},
    };

    const uint32_t laneIndex = operation.elementType.laneCount == 1   ? 0
                               : operation.elementType.laneCount == 2 ? 1
                                                                      : 2;
    const uint32_t dimensionIndex = operation.shape - SLANG_NVVM_TEXTURE_SHAPE_1D;
    const uint32_t physicalBitWidth = operation.elementType.bitWidth;
    if (physicalBitWidth == 16)
    {
        return operation.operation == SLANG_NVVM_SURFACE_OP_LOAD
                   ? kLoadI16[dimensionIndex][laneIndex]
                   : kStoreI16[dimensionIndex][laneIndex];
    }
    if (operation.isArray)
    {
        return operation.operation == SLANG_NVVM_SURFACE_OP_LOAD
                   ? kLoadI32Array[dimensionIndex][laneIndex]
                   : kStoreI32Array[dimensionIndex][laneIndex];
    }
    return operation.operation == SLANG_NVVM_SURFACE_OP_LOAD ? kLoadI32[dimensionIndex][laneIndex]
                                                             : kStoreI32[dimensionIndex][laneIndex];
}

static SlangResult SLANG_NVVM_CALL _emitSurfaceOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMSurfaceOperationDesc* operation,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    const size_t expectedOperandCount =
        operation && operation->operation == SLANG_NVVM_SURFACE_OP_LOAD    ? 2
        : operation && operation->operation == SLANG_NVVM_SURFACE_OP_STORE ? 3
                                                                           : 0;
    ModuleState* state = _getModule(module);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !insertionBlock || !operation || !outValue || !operands ||
        !expectedOperandCount || operandCount != expectedOperandCount ||
        !_isSurfaceOperationSupported(*operation))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* surface = _getValue(operands[0]);
    llvm::Value* coordinate = _getValue(operands[1]);
    llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
    const uint32_t coordinateLaneCount = uint32_t(operation->shape) + operation->isArray;
    llvm::Type* expectedCoordinateType =
        coordinateLaneCount == 1 ? int32Type
                                 : llvm::FixedVectorType::get(int32Type, coordinateLaneCount);
    if (!surface || !surface->getType()->isIntegerTy(64) || !coordinate ||
        coordinate->getType() != expectedCoordinateType ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, surface) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, coordinate))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Type* halfType = llvm::Type::getHalfTy(state->context);
    llvm::Type* floatType = llvm::Type::getFloatTy(state->context);
    llvm::Type* semanticScalarType =
        operation->elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT
            ? (operation->elementType.bitWidth == 16 ? halfType : floatType)
            : int32Type;
    llvm::Type* semanticElementType =
        operation->elementType.laneCount == 1
            ? semanticScalarType
            : llvm::FixedVectorType::get(semanticScalarType, operation->elementType.laneCount);
    llvm::Value* storedValue = nullptr;
    if (operation->operation == SLANG_NVVM_SURFACE_OP_STORE)
    {
        storedValue = _getValue(operands[2]);
        if (!storedValue || storedValue->getType() != semanticElementType ||
            !_isValueUsableAtInsertionPoint(state, insertionBlock, storedValue))
        {
            return SLANG_E_INVALID_ARG;
        }
    }

    const uint32_t physicalBitWidth = operation->elementType.bitWidth;
    llvm::Type* physicalIntegerType =
        physicalBitWidth == 16 ? llvm::Type::getInt16Ty(state->context) : int32Type;
    llvm::Intrinsic::ID intrinsicID = _getSurfaceIntrinsicID(*operation);
    if (intrinsicID == llvm::Intrinsic::not_intrinsic)
        return SLANG_E_INVALID_ARG;

    llvm::SmallVector<llvm::Value*, 7> arguments;
    arguments.push_back(surface);
    // Physical IR keeps (byteX, y, layer), but NVVM array intrinsics take the layer first.
    // Keep this calling convention conversion here so coordinate planning stays shape-independent.
    if (operation->isArray)
        arguments.push_back(
            state->builder.CreateExtractElement(coordinate, coordinateLaneCount - 1));
    llvm::Value* x = coordinateLaneCount == 1
                         ? coordinate
                         : state->builder.CreateExtractElement(coordinate, uint64_t(0));
    arguments.push_back(x);
    const uint32_t spatialLaneCount = coordinateLaneCount - operation->isArray;
    for (uint32_t dimension = 1; dimension < spatialLaneCount; ++dimension)
        arguments.push_back(state->builder.CreateExtractElement(coordinate, dimension));

    if (operation->operation == SLANG_NVVM_SURFACE_OP_STORE)
    {
        for (uint32_t lane = 0; lane < operation->elementType.laneCount; ++lane)
        {
            llvm::Value* laneValue = operation->elementType.laneCount == 1
                                         ? storedValue
                                         : state->builder.CreateExtractElement(storedValue, lane);
            arguments.push_back(state->builder.CreateBitCast(laneValue, physicalIntegerType));
        }
    }

    llvm::Function* intrinsic = llvm::Intrinsic::getDeclaration(state->module.get(), intrinsicID);
    llvm::CallInst* call = state->builder.CreateCall(intrinsic, arguments);
    if (operation->operation == SLANG_NVVM_SURFACE_OP_STORE)
        return SLANG_OK;

    llvm::Value* result = nullptr;
    if (operation->elementType.laneCount == 1)
    {
        result = state->builder.CreateBitCast(call, semanticScalarType);
    }
    else
    {
        llvm::SmallVector<llvm::Value*, 4> laneValues;
        for (uint32_t lane = 0; lane < operation->elementType.laneCount; ++lane)
        {
            llvm::Value* bits = state->builder.CreateExtractValue(call, {lane});
            llvm::Value* laneValue = state->builder.CreateBitCast(bits, semanticScalarType);
            laneValues.push_back(laneValue);
        }
        result = _createNVVMVectorConstruct(
            state,
            llvm::cast<llvm::FixedVectorType>(semanticElementType),
            laneValues);
    }
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static uint32_t _getTextureCoordinateLaneCount(const SlangNVVMTextureOperationDesc& operation)
{
    uint32_t coordinateLaneCount = 0;
    switch (operation.shape)
    {
    case SLANG_NVVM_TEXTURE_SHAPE_1D:
        coordinateLaneCount = 1;
        break;
    case SLANG_NVVM_TEXTURE_SHAPE_2D:
        coordinateLaneCount = 2;
        break;
    case SLANG_NVVM_TEXTURE_SHAPE_3D:
    case SLANG_NVVM_TEXTURE_SHAPE_CUBE:
        coordinateLaneCount = 3;
        break;
    }
    return coordinateLaneCount ? coordinateLaneCount + operation.isArray : 0;
}

static bool _isTextureOperationSupported(const SlangNVVMTextureOperationDesc& operation)
{
    if (operation.operation != SLANG_NVVM_TEXTURE_OP_GATHER && operation.component != 0)
        return false;
    const bool isSupportedShape = operation.shape == SLANG_NVVM_TEXTURE_SHAPE_1D ||
                                  operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D ||
                                  operation.shape == SLANG_NVVM_TEXTURE_SHAPE_3D ||
                                  operation.shape == SLANG_NVVM_TEXTURE_SHAPE_CUBE;
    if (!isSupportedShape || operation.isArray > 1 ||
        (operation.shape == SLANG_NVVM_TEXTURE_SHAPE_3D && operation.isArray))
    {
        return false;
    }
    const bool isSampleElement =
        operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
        operation.elementType.bitWidth == 32 &&
        (operation.elementType.laneCount == 1 || operation.elementType.laneCount == 2 ||
         operation.elementType.laneCount == 4);
    const bool isNumericElement =
        (operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ||
         operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
         operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
        operation.elementType.bitWidth == 32 &&
        (operation.elementType.laneCount == 1 || operation.elementType.laneCount == 2 ||
         operation.elementType.laneCount == 4);
    const bool isGatherElement = isNumericElement && operation.elementType.laneCount == 4;
    switch (operation.operation)
    {
    case SLANG_NVVM_TEXTURE_OP_SAMPLE:
    case SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL:
        return isSampleElement;
    case SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH:
        return isNumericElement;
    case SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT:
        return isNumericElement &&
               (operation.shape != SLANG_NVVM_TEXTURE_SHAPE_1D || operation.isArray);
    case SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH:
        return isNumericElement &&
               (operation.shape == SLANG_NVVM_TEXTURE_SHAPE_3D ||
                (operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D && operation.isArray));
    case SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL:
        return isNumericElement &&
               (operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D ||
                (operation.shape == SLANG_NVVM_TEXTURE_SHAPE_3D && !operation.isArray));
    case SLANG_NVVM_TEXTURE_OP_GATHER:
        return isGatherElement && operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D &&
               !operation.isArray && operation.component < 4;
    default:
        return false;
    }
}

static SlangResult SLANG_NVVM_CALL
_isTextureOperationSupported(const SlangNVVMTextureOperationDesc* operation, uint32_t* outSupported)
{
    if (outSupported)
        *outSupported = 0;
    if (!operation || !outSupported)
        return SLANG_E_INVALID_ARG;
    *outSupported = _isTextureOperationSupported(*operation) ? 1u : 0u;
    return SLANG_OK;
}

static llvm::Intrinsic::ID _getTextureIntrinsicID(const SlangNVVMTextureOperationDesc& operation)
{
    if (!_isTextureOperationSupported(operation))
        return llvm::Intrinsic::not_intrinsic;

    switch (operation.operation)
    {
    case SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH:
        return llvm::Intrinsic::nvvm_txq_width;
    case SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT:
        return llvm::Intrinsic::nvvm_txq_height;
    case SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH:
        return llvm::Intrinsic::nvvm_txq_depth;
    }
    if (operation.operation != SLANG_NVVM_TEXTURE_OP_SAMPLE &&
        operation.operation != SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL)
    {
        return llvm::Intrinsic::not_intrinsic;
    }

    const bool hasExplicitLevel = operation.operation == SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL;
    if (operation.isArray)
    {
        switch (operation.shape)
        {
        case SLANG_NVVM_TEXTURE_SHAPE_1D:
            return hasExplicitLevel ? llvm::Intrinsic::nvvm_tex_unified_1d_array_level_v4f32_f32
                                    : llvm::Intrinsic::nvvm_tex_unified_1d_array_v4f32_f32;
        case SLANG_NVVM_TEXTURE_SHAPE_2D:
            return hasExplicitLevel ? llvm::Intrinsic::nvvm_tex_unified_2d_array_level_v4f32_f32
                                    : llvm::Intrinsic::nvvm_tex_unified_2d_array_v4f32_f32;
        case SLANG_NVVM_TEXTURE_SHAPE_CUBE:
            return hasExplicitLevel ? llvm::Intrinsic::nvvm_tex_unified_cube_array_level_v4f32_f32
                                    : llvm::Intrinsic::nvvm_tex_unified_cube_array_v4f32_f32;
        }
    }
    else
    {
        switch (operation.shape)
        {
        case SLANG_NVVM_TEXTURE_SHAPE_1D:
            return hasExplicitLevel ? llvm::Intrinsic::nvvm_tex_unified_1d_level_v4f32_f32
                                    : llvm::Intrinsic::nvvm_tex_unified_1d_v4f32_f32;
        case SLANG_NVVM_TEXTURE_SHAPE_2D:
            return hasExplicitLevel ? llvm::Intrinsic::nvvm_tex_unified_2d_level_v4f32_f32
                                    : llvm::Intrinsic::nvvm_tex_unified_2d_v4f32_f32;
        case SLANG_NVVM_TEXTURE_SHAPE_3D:
            return hasExplicitLevel ? llvm::Intrinsic::nvvm_tex_unified_3d_level_v4f32_f32
                                    : llvm::Intrinsic::nvvm_tex_unified_3d_v4f32_f32;
        case SLANG_NVVM_TEXTURE_SHAPE_CUBE:
            return hasExplicitLevel ? llvm::Intrinsic::nvvm_tex_unified_cube_level_v4f32_f32
                                    : llvm::Intrinsic::nvvm_tex_unified_cube_v4f32_f32;
        }
    }
    return llvm::Intrinsic::not_intrinsic;
}

static SlangResult SLANG_NVVM_CALL _emitTextureOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMTextureOperationDesc* operation,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    ModuleState* state = _getModule(module);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    const bool isGather = operation && operation->operation == SLANG_NVVM_TEXTURE_OP_GATHER;
    const bool isSample = operation && operation->operation == SLANG_NVVM_TEXTURE_OP_SAMPLE;
    const size_t expectedOperandCount =
        isGather || isSample ? 2
        : operation && (operation->operation == SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL ||
                        operation->operation == SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL)
            ? 3
            : 1;
    if (!state || !insertionBlock || !operation || !operands ||
        operandCount != expectedOperandCount || !outValue ||
        !_isTextureOperationSupported(*operation))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* texture = _getValue(operands[0]);
    const bool isSampleLevel = operation->operation == SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL;
    const bool isFetchLevel = operation->operation == SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL;
    const bool hasCoordinate = isSample || isSampleLevel || isFetchLevel || isGather;
    llvm::Value* coordinate = hasCoordinate ? _getValue(operands[1]) : nullptr;
    llvm::Value* level = isSampleLevel || isFetchLevel ? _getValue(operands[2]) : nullptr;
    llvm::Type* floatType = llvm::Type::getFloatTy(state->context);
    llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
    const uint32_t coordinateLaneCount = _getTextureCoordinateLaneCount(*operation);
    llvm::Type* coordinateScalarType = isFetchLevel ? int32Type : floatType;
    llvm::Type* expectedCoordinateType =
        coordinateLaneCount == 1
            ? coordinateScalarType
            : llvm::FixedVectorType::get(coordinateScalarType, coordinateLaneCount);
    const llvm::Intrinsic::ID intrinsicID =
        isFetchLevel ? llvm::Intrinsic::not_intrinsic : _getTextureIntrinsicID(*operation);
    if (!texture || !texture->getType()->isIntegerTy(64) ||
        (hasCoordinate && (!coordinate || coordinate->getType() != expectedCoordinateType ||
                           ((isSampleLevel || isFetchLevel) &&
                            (!level || level->getType() != (isFetchLevel ? int32Type : floatType) ||
                             !_isValueUsableAtInsertionPoint(state, insertionBlock, level))) ||
                           !_isValueUsableAtInsertionPoint(state, insertionBlock, coordinate))) ||
        (!isFetchLevel && !isGather && intrinsicID == llvm::Intrinsic::not_intrinsic) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, texture))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::SmallVector<llvm::Value*, 6> arguments;
    arguments.push_back(texture);
    if (!hasCoordinate)
    {
        llvm::Function* intrinsic =
            llvm::Intrinsic::getDeclaration(state->module.get(), intrinsicID);
        *outValue =
            reinterpret_cast<SlangNVVMValueHandle>(state->builder.CreateCall(intrinsic, arguments));
        return SLANG_OK;
    }

    if (isGather)
    {
        const bool isFloat = operation->elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT;
        const char* componentNames[] = {"r", "g", "b", "a"};
        const char* dataType = isFloat ? "f32"
                               : operation->elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                                   ? "s32"
                                   : "u32";
        const std::string assembly = std::string("tld4.") + componentNames[operation->component] +
                                     ".2d.v4." + dataType +
                                     ".f32 {$0, $1, $2, $3}, [$4, {$5, $6}];";
        const std::string constraints = isFloat ? "=f,=f,=f,=f,l,f,f" : "=r,=r,=r,=r,l,f,f";
        arguments.push_back(state->builder.CreateExtractElement(coordinate, uint64_t(0)));
        arguments.push_back(state->builder.CreateExtractElement(coordinate, uint64_t(1)));

        llvm::Type* physicalScalarType = isFloat ? floatType : int32Type;
        llvm::SmallVector<llvm::Type*, 3> argumentTypes;
        for (llvm::Value* argument : arguments)
            argumentTypes.push_back(argument->getType());
        llvm::StructType* physicalResultType = llvm::StructType::get(
            state->context,
            {physicalScalarType, physicalScalarType, physicalScalarType, physicalScalarType});
        llvm::FunctionType* functionType =
            llvm::FunctionType::get(physicalResultType, argumentTypes, false);
        llvm::InlineAsm* inlineAsm =
            llvm::InlineAsm::get(functionType, assembly, constraints, false);
        llvm::CallInst* call = state->builder.CreateCall(inlineAsm, arguments);
        llvm::Type* resultType = llvm::FixedVectorType::get(physicalScalarType, 4);
        llvm::Value* result = llvm::UndefValue::get(resultType);
        for (uint32_t lane = 0; lane < 4; ++lane)
        {
            result = state->builder.CreateInsertElement(
                result,
                state->builder.CreateExtractValue(call, {lane}),
                lane);
        }
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
        return SLANG_OK;
    }

    if (isFetchLevel)
    {
        const bool isFloat = operation->elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT;
        const char* dataType = isFloat ? "f32"
                               : operation->elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                                   ? "s32"
                                   : "u32";
        std::string assembly;
        std::string constraints = isFloat ? "=f,=f,=f,=f,l" : "=r,=r,=r,=r,l";
        if (operation->shape == SLANG_NVVM_TEXTURE_SHAPE_2D && !operation->isArray)
        {
            assembly = std::string("tex.level.2d.v4.") + dataType +
                       ".s32 {$0, $1, $2, $3}, [$4, {$5, $6}], $7;";
            constraints += ",r,r,r";
            arguments.push_back(state->builder.CreateExtractElement(coordinate, uint64_t(0)));
            arguments.push_back(state->builder.CreateExtractElement(coordinate, uint64_t(1)));
        }
        else if (operation->shape == SLANG_NVVM_TEXTURE_SHAPE_3D)
        {
            assembly = std::string("tex.level.3d.v4.") + dataType +
                       ".s32 {$0, $1, $2, $3}, [$4, {$5, $6, $7, $8}], $9;";
            constraints += ",r,r,r,r,r";
            for (uint32_t lane = 0; lane < 3; ++lane)
                arguments.push_back(
                    state->builder.CreateExtractElement(coordinate, uint64_t(lane)));
            arguments.push_back(arguments.back());
        }
        else if (operation->shape == SLANG_NVVM_TEXTURE_SHAPE_2D && operation->isArray)
        {
            assembly = std::string("tex.level.a2d.v4.") + dataType +
                       ".s32 {$0, $1, $2, $3}, [$4, {$5, $6, $7, $8}], $9;";
            constraints += ",r,r,r,r,r";
            llvm::Value* x = state->builder.CreateExtractElement(coordinate, uint64_t(0));
            llvm::Value* y = state->builder.CreateExtractElement(coordinate, uint64_t(1));
            llvm::Value* layer = state->builder.CreateExtractElement(coordinate, uint64_t(2));
            arguments.push_back(layer);
            arguments.push_back(x);
            arguments.push_back(y);
            arguments.push_back(layer);
        }
        else
            return SLANG_E_INVALID_ARG;
        arguments.push_back(level);

        llvm::Type* physicalScalarType = isFloat ? floatType : int32Type;
        llvm::SmallVector<llvm::Type*, 6> argumentTypes;
        for (llvm::Value* argument : arguments)
            argumentTypes.push_back(argument->getType());
        llvm::StructType* physicalResultType = llvm::StructType::get(
            state->context,
            {physicalScalarType, physicalScalarType, physicalScalarType, physicalScalarType});
        llvm::FunctionType* functionType =
            llvm::FunctionType::get(physicalResultType, argumentTypes, false);
        llvm::InlineAsm* inlineAsm =
            llvm::InlineAsm::get(functionType, assembly, constraints, false);
        llvm::CallInst* call = state->builder.CreateCall(inlineAsm, arguments);
        llvm::Value* result = state->builder.CreateExtractValue(call, {0});
        if (operation->elementType.laneCount > 1)
        {
            llvm::Type* resultType =
                llvm::FixedVectorType::get(physicalScalarType, operation->elementType.laneCount);
            result = llvm::UndefValue::get(resultType);
            for (uint32_t lane = 0; lane < operation->elementType.laneCount; ++lane)
            {
                result = state->builder.CreateInsertElement(
                    result,
                    state->builder.CreateExtractValue(call, {lane}),
                    lane);
            }
        }
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
        return SLANG_OK;
    }

    const uint32_t ordinaryCoordinateLaneCount = coordinateLaneCount - operation->isArray;
    if (operation->isArray)
    {
        llvm::Value* layer =
            state->builder.CreateExtractElement(coordinate, uint64_t(coordinateLaneCount - 1));
        arguments.push_back(
            state->builder.CreateFPToSI(layer, llvm::Type::getInt32Ty(state->context)));
    }
    for (uint32_t lane = 0; lane < ordinaryCoordinateLaneCount; ++lane)
    {
        arguments.push_back(
            coordinateLaneCount == 1
                ? coordinate
                : state->builder.CreateExtractElement(coordinate, uint64_t(lane)));
    }
    if (isSampleLevel)
        arguments.push_back(level);

    llvm::Function* intrinsic = llvm::Intrinsic::getDeclaration(state->module.get(), intrinsicID);
    llvm::CallInst* call = state->builder.CreateCall(intrinsic, arguments);
    llvm::Value* result = state->builder.CreateExtractValue(call, {0});
    if (operation->elementType.laneCount > 1)
    {
        llvm::Type* resultType =
            llvm::FixedVectorType::get(floatType, operation->elementType.laneCount);
        result = llvm::UndefValue::get(resultType);
        for (uint32_t lane = 0; lane < operation->elementType.laneCount; ++lane)
        {
            result = state->builder.CreateInsertElement(
                result,
                state->builder.CreateExtractValue(call, {lane}),
                lane);
        }
    }
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static void _fillBuilderFoundationAPI(SlangNVVMBuilderFoundationAPI& api)
{
    api = {};
    api.createModule = _createModule;
    api.destroyModule = _destroyModule;
    api.serializeModuleWithDiagnostics = _serializeModuleWithDiagnostics;
    api.serializeNVVMIR20AssemblyWithDiagnostics = _serializeNVVMIR20AssemblyWithDiagnostics;
}

static void _fillBuilderConstructionAPI(SlangNVVMBuilderConstructionAPI& api)
{
    api = {};
    api.getVoidType = _getVoidType;
    api.getIntegerType = _getIntegerType;
    api.getFloatingPointType = _getFloatingPointType;
    api.getPointerType = _getPointerType;
    api.getFunctionType = _getFunctionType;
    api.getArrayType = _getArrayType;
    api.getVectorType = _getVectorType;
    api.getStructType = _getStructType;
    api.declareFunction = _declareFunction;
    api.getFunctionParameter = _getFunctionParameter;
    api.setFunctionParameterAttributes = _setFunctionParameterAttributes;
    api.createBlock = _createBlock;
    api.setInsertBlock = _setInsertBlock;
    api.emitLoad = _emitLoad;
    api.emitStore = _emitStore;
    api.emitLocalStorage = _emitLocalStorage;
    api.emitBranch = _emitBranch;
    api.emitConditionalBranch = _emitConditionalBranch;
    api.emitSwitch = _emitSwitch;
    api.getIntegerConstant = _getIntegerConstant;
    api.getFloatingPointConstant = _getFloatingPointConstant;
    api.emitPhi = _emitPhi;
    api.addPhiIncoming = _addPhiIncoming;
    api.emitCall = _emitCall;
    api.emitValueReturn = _emitValueReturn;
    api.emitReturnVoid = _emitReturnVoid;
    api.emitUnreachable = _emitUnreachable;
    api.emitPointerOffset = _emitPointerOffset;
    api.emitByteOffsetPointer = _emitByteOffsetPointer;
    api.emitSequentialElementPointer = _emitSequentialElementPointer;
    api.emitStructFieldPointer = _emitStructFieldPointer;
    api.emitAggregateConstruct = _emitAggregateConstruct;
    api.emitAggregateElementExtract = _emitAggregateElementExtract;
    api.emitBitCast = _emitBitCast;
    api.emitPointerAddressSpaceCast = _emitPointerAddressSpaceCast;
    api.emitVectorConstruct = _emitVectorConstruct;
    api.emitSequentialElementExtract = _emitSequentialElementExtract;
    api.declareGlobalStorage = _declareGlobalStorage;
    api.markFunctionAsKernel = _markFunctionAsKernel;
}

static void _fillBuilderValueOperationsAPI(SlangNVVMBuilderValueOperationsAPI& api)
{
    api = {};
    api.isOperationSupported = _isOperationSupported;
    api.emitOperation = _emitOperation;
    api.isNamedIntrinsicSupported = _isNamedIntrinsicSupported;
    api.emitNamedIntrinsic = _emitNamedIntrinsic;
    api.loadDeviceLibrary = _loadDeviceLibrary;
    api.destroyDeviceLibrary = _destroyDeviceLibrary;
    api.isDeviceLibraryFunctionSupported = _isDeviceLibraryFunctionSupported;
    api.emitDeviceLibraryFunction = _emitDeviceLibraryFunction;
}

static void _fillBuilderAtomicOperationsAPI(SlangNVVMBuilderAtomicOperationsAPI& api)
{
    api = {};
    api.isOperationSupported = _isAtomicOperationSupported;
    api.emitOperation = _emitAtomicOperation;
}

static void _fillBuilderSurfaceOperationsAPI(SlangNVVMBuilderSurfaceOperationsAPI& api)
{
    api = {};
    api.isOperationSupported = _isSurfaceOperationSupported;
    api.emitOperation = _emitSurfaceOperation;
}

static void _fillBuilderTextureOperationsAPI(SlangNVVMBuilderTextureOperationsAPI& api)
{
    api = {};
    api.isOperationSupported = _isTextureOperationSupported;
    api.emitOperation = _emitTextureOperation;
}

static SlangResult SLANG_NVVM_CALL
_queryBuilderInterface(SlangNVVMBuilderInterfaceID interfaceID, const void** outInterface)
{
    if (outInterface)
        *outInterface = nullptr;
    if (!outInterface)
        return SLANG_E_INVALID_ARG;

    static const SlangNVVMBuilderFoundationAPI foundation = []
    {
        SlangNVVMBuilderFoundationAPI api;
        _fillBuilderFoundationAPI(api);
        return api;
    }();
    static const SlangNVVMBuilderConstructionAPI construction = []
    {
        SlangNVVMBuilderConstructionAPI api;
        _fillBuilderConstructionAPI(api);
        return api;
    }();
    static const SlangNVVMBuilderValueOperationsAPI valueOperations = []
    {
        SlangNVVMBuilderValueOperationsAPI api;
        _fillBuilderValueOperationsAPI(api);
        return api;
    }();
    static const SlangNVVMBuilderAtomicOperationsAPI atomicOperations = []
    {
        SlangNVVMBuilderAtomicOperationsAPI api;
        _fillBuilderAtomicOperationsAPI(api);
        return api;
    }();
    static const SlangNVVMBuilderSurfaceOperationsAPI surfaceOperations = []
    {
        SlangNVVMBuilderSurfaceOperationsAPI api;
        _fillBuilderSurfaceOperationsAPI(api);
        return api;
    }();
    static const SlangNVVMBuilderTextureOperationsAPI textureOperations = []
    {
        SlangNVVMBuilderTextureOperationsAPI api;
        _fillBuilderTextureOperationsAPI(api);
        return api;
    }();

    switch (interfaceID)
    {
    case SLANG_NVVM_BUILDER_INTERFACE_FOUNDATION:
        *outInterface = &foundation;
        return SLANG_OK;
    case SLANG_NVVM_BUILDER_INTERFACE_CONSTRUCTION:
        *outInterface = &construction;
        return SLANG_OK;
    case SLANG_NVVM_BUILDER_INTERFACE_VALUE_OPERATIONS:
        *outInterface = &valueOperations;
        return SLANG_OK;
    case SLANG_NVVM_BUILDER_INTERFACE_ATOMIC_OPERATIONS:
        *outInterface = &atomicOperations;
        return SLANG_OK;
    case SLANG_NVVM_BUILDER_INTERFACE_SURFACE_OPERATIONS:
        *outInterface = &surfaceOperations;
        return SLANG_OK;
    case SLANG_NVVM_BUILDER_INTERFACE_TEXTURE_OPERATIONS:
        *outInterface = &textureOperations;
        return SLANG_OK;
    default:
        return SLANG_E_NO_INTERFACE;
    }
}

static void _fillBuilderAPI(SlangNVVMBuilderAPI& api)
{
    api = {};
    api.llvmVersionMajor = LLVM_VERSION_MAJOR;
    api.llvmVersionMinor = LLVM_VERSION_MINOR;
    api.llvmVersionPatch = LLVM_VERSION_PATCH;
    api.nvvmIRVersionMajor = 2;
    api.nvvmIRVersionMinor = 0;
    api.pointerModel = SLANG_NVVM_POINTER_MODEL_TYPED;
    api.queryInterface = _queryBuilderInterface;
}

} // namespace

extern "C" SLANG_NVVM_BUILDER_API SlangResult SLANG_NVVM_CALL
slang_getNVVMBuilderAPI(uint32_t abiRevision, SlangNVVMBuilderAPI* outAPI)
{
    if (!outAPI || abiRevision != SLANG_NVVM_BUILDER_ABI_REVISION)
        return SLANG_E_NO_INTERFACE;

    _fillBuilderAPI(*outAPI);
    return SLANG_OK;
}
