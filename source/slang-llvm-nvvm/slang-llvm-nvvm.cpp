#include "compiler-core/slang-nvvm-ir-builder-api.h"
#include "compiler-core/slang-nvvm-semantic-catalog.h"
#include "slang.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsNVPTX.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/MathExtras.h"
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
    if (!state || !outType || bitWidth != 32)
        return SLANG_E_INVALID_ARG;

    *outType = reinterpret_cast<SlangNVVMTypeHandle>(llvm::Type::getFloatTy(state->context));
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

    state->builder.CreateAlignedStore(llvmValue, llvmPointer, llvm::Align(alignment));
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

static SlangResult SLANG_NVVM_CALL _emitRelaxedGlobalI32AtomicAdd(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle pointer,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outOriginalValue)
{
    if (outOriginalValue)
        *outOriginalValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmPointer = _getValue(pointer);
    llvm::Value* llvmValue = _getValue(value);
    llvm::PointerType* pointerType = _getLoadablePointerType(state, llvmPointer);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::Type* pointeeType = pointerType ? pointerType->getNonOpaquePointerElementType() : nullptr;
    if (!outOriginalValue || !pointerType ||
        pointerType->getAddressSpace() != SLANG_NVVM_ADDRESS_SPACE_GLOBAL || !pointeeType ||
        !pointeeType->isIntegerTy(32) || !insertionBlock || !llvmValue ||
        llvmValue->getType() != pointeeType ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmPointer) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmValue))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* originalValue = state->builder.CreateAtomicRMW(
        llvm::AtomicRMWInst::Add,
        llvmPointer,
        llvmValue,
        llvm::Align(4),
        llvm::AtomicOrdering::Monotonic,
        llvm::SyncScope::System);
    *outOriginalValue = reinterpret_cast<SlangNVVMValueHandle>(originalValue);
    return SLANG_OK;
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

static SlangResult _emitFloatingBinary(
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
    if (!outValue || !insertionBlock ||
        (operation != SLANG_NVVM_VALUE_OP_ADD && operation != SLANG_NVVM_VALUE_OP_SUBTRACT &&
         operation != SLANG_NVVM_VALUE_OP_MULTIPLY && operation != SLANG_NVVM_VALUE_OP_DIVIDE) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmLeft) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmRight) ||
        llvmLeft->getType() != llvm::Type::getFloatTy(state->context) ||
        llvmRight->getType() != llvmLeft->getType())
    {
        return SLANG_E_INVALID_ARG;
    }

    switch (operation)
    {
    case SLANG_NVVM_VALUE_OP_ADD:
        *outValue =
            reinterpret_cast<SlangNVVMValueHandle>(state->builder.CreateFAdd(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_VALUE_OP_SUBTRACT:
        *outValue =
            reinterpret_cast<SlangNVVMValueHandle>(state->builder.CreateFSub(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_VALUE_OP_MULTIPLY:
        *outValue =
            reinterpret_cast<SlangNVVMValueHandle>(state->builder.CreateFMul(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_VALUE_OP_DIVIDE:
        *outValue =
            reinterpret_cast<SlangNVVMValueHandle>(state->builder.CreateFDiv(llvmLeft, llvmRight));
        return SLANG_OK;
    default:
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult _emitFloatingUnary(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmValue = _getValue(value);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock || operation != SLANG_NVVM_VALUE_OP_NEGATE ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmValue) ||
        llvmValue->getType() != llvm::Type::getFloatTy(state->context))
    {
        return SLANG_E_INVALID_ARG;
    }

    *outValue = reinterpret_cast<SlangNVVMValueHandle>(state->builder.CreateFNeg(llvmValue));
    return SLANG_OK;
}

static SlangResult _emitFloatingCompare(
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
    if (!outValue || !insertionBlock ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmLeft) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmRight) ||
        llvmLeft->getType() != llvm::Type::getFloatTy(state->context) ||
        llvmRight->getType() != llvmLeft->getType())
    {
        return SLANG_E_INVALID_ARG;
    }

    switch (operation)
    {
    case SLANG_NVVM_VALUE_OP_EQUAL:
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(
            state->builder.CreateFCmpOEQ(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_VALUE_OP_NOT_EQUAL:
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(
            state->builder.CreateFCmpUNE(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_VALUE_OP_GREATER_THAN:
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(
            state->builder.CreateFCmpOGT(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_VALUE_OP_LESS_EQUAL:
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(
            state->builder.CreateFCmpOLE(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_VALUE_OP_GREATER_EQUAL:
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(
            state->builder.CreateFCmpOGE(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_VALUE_OP_LESS_THAN:
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(
            state->builder.CreateFCmpOLT(llvmLeft, llvmRight));
        return SLANG_OK;
    default:
        return SLANG_E_INVALID_ARG;
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
    if (!state || !llvmFloatingPointType || !llvmFloatingPointType->isFloatTy() || bitWidth != 32 ||
        (bitPattern >> 32) != 0 || &llvmFloatingPointType->getContext() != &state->context ||
        !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }

    const llvm::APFloat value(llvm::APFloat::IEEEsingle(), llvm::APInt(32, uint32_t(bitPattern)));
    *outValue =
        reinterpret_cast<SlangNVVMValueHandle>(llvm::ConstantFP::get(llvmFloatingPointType, value));
    return SLANG_OK;
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
    const bool isSupportedType = llvmType && (llvmType->isIntegerTy() || llvmType->isFloatTy());
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
    const bool isSupportedType =
        llvmPhi && (llvmPhi->getType()->isIntegerTy() || llvmPhi->getType()->isFloatTy());
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

static bool _isSupportedFunctionValueType(llvm::Type* type)
{
    if (type && (type->isIntegerTy() || type->isFloatTy()))
        return true;
    auto vectorType = llvm::dyn_cast_or_null<llvm::FixedVectorType>(type);
    return vectorType && vectorType->getNumElements() >= 2 && vectorType->getNumElements() <= 4 &&
           _isSupportedFunctionValueType(vectorType->getElementType());
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
        if (!_isSupportedFunctionValueType(parameterType) || !argument ||
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

static SlangResult SLANG_NVVM_CALL _emitVectorElementExtract(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle vector,
    uint32_t elementIndex,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmVector = _getValue(vector);
    auto vectorType =
        llvmVector ? llvm::dyn_cast<llvm::FixedVectorType>(llvmVector->getType()) : nullptr;
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outValue || !insertionBlock || !vectorType ||
        elementIndex >= vectorType->getNumElements() ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmVector))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = state->builder.CreateExtractElement(llvmVector, elementIndex);
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
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

    llvm::Value* result = llvm::UndefValue::get(llvmVectorType);
    for (size_t i = 0; i < elementCount; ++i)
        result = state->builder.CreateInsertElement(result, llvmElements[i], uint64_t(i));
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
    bool appendsShuffleClamp = false;
    bool derivesFirstActiveLane = false;
    bool derivesFirstLanePredicate = false;
    bool extractsMatchAllPredicate = false;
    bool bitcastsMatchAllFloatValue = false;
    const bool hasFloatingValue =
        operation.operandCount > 1 &&
        operation.operandTypes[1].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT;
    switch (operation.operation)
    {
    case SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX:
        intrinsicID = llvm::Intrinsic::nvvm_read_ptx_sreg_laneid;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_LANE_COUNT:
        intrinsicID = llvm::Intrinsic::nvvm_read_ptx_sreg_warpsize;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT:
        intrinsicID = hasFloatingValue ? llvm::Intrinsic::nvvm_shfl_sync_idx_f32
                                       : llvm::Intrinsic::nvvm_shfl_sync_idx_i32;
        expectedArgumentCount = 3;
        if (hasFloatingValue)
            expectedArgumentTypes[1] = llvm::Type::getFloatTy(state->context);
        appendsShuffleClamp = true;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT:
        intrinsicID = llvm::Intrinsic::nvvm_vote_ballot_sync;
        expectedArgumentCount = 2;
        expectedArgumentTypes[1] = llvm::Type::getInt1Ty(state->context);
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST:
        intrinsicID = hasFloatingValue ? llvm::Intrinsic::nvvm_shfl_sync_idx_f32
                                       : llvm::Intrinsic::nvvm_shfl_sync_idx_i32;
        expectedArgumentCount = 2;
        if (hasFloatingValue)
            expectedArgumentTypes[1] = llvm::Type::getFloatTy(state->context);
        derivesFirstActiveLane = true;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_IS_FIRST_LANE:
        expectedArgumentCount = 1;
        derivesFirstLanePredicate = true;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_ANY_TRUE:
        intrinsicID = llvm::Intrinsic::nvvm_vote_any_sync;
        expectedArgumentCount = 2;
        expectedArgumentTypes[1] = llvm::Type::getInt1Ty(state->context);
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_TRUE:
        intrinsicID = llvm::Intrinsic::nvvm_vote_all_sync;
        expectedArgumentCount = 2;
        expectedArgumentTypes[1] = llvm::Type::getInt1Ty(state->context);
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL:
        intrinsicID = llvm::Intrinsic::nvvm_match_all_sync_i32p;
        expectedArgumentCount = 2;
        if (hasFloatingValue)
            expectedArgumentTypes[1] = llvm::Type::getFloatTy(state->context);
        extractsMatchAllPredicate = true;
        bitcastsMatchAllFloatValue = hasFloatingValue;
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
    if (bitcastsMatchAllFloatValue)
        llvmArguments[1] = state->builder.CreateBitCast(llvmArguments[1], int32Type);
    if (derivesFirstLanePredicate)
    {
        llvm::Function* laneIDIntrinsic = llvm::Intrinsic::getDeclaration(
            state->module.get(),
            llvm::Intrinsic::nvvm_read_ptx_sreg_laneid);
        llvm::Value* laneID = state->builder.CreateCall(laneIDIntrinsic);
        llvm::Value* firstMaskBit =
            state->builder.CreateAnd(llvmArguments[0], state->builder.CreateNeg(llvmArguments[0]));
        llvm::Value* laneBit =
            state->builder.CreateShl(llvm::ConstantInt::get(int32Type, 1), laneID);
        llvm::Value* predicate = state->builder.CreateICmpEQ(firstMaskBit, laneBit);
        *outValue = reinterpret_cast<SlangNVVMValueHandle>(predicate);
        return SLANG_OK;
    }
    if (derivesFirstActiveLane)
    {
        llvm::Function* countTrailingZeros = llvm::Intrinsic::getDeclaration(
            state->module.get(),
            llvm::Intrinsic::cttz,
            {int32Type});
        llvm::Value* firstActiveLane = state->builder.CreateCall(
            countTrailingZeros,
            {llvmArguments[0], llvm::ConstantInt::getTrue(state->context)});
        llvmArguments.push_back(firstActiveLane);
        llvmArguments.push_back(llvm::ConstantInt::get(int32Type, 31));
    }
    else if (appendsShuffleClamp)
    {
        llvmArguments.push_back(llvm::ConstantInt::get(int32Type, 31));
    }

    llvm::Function* intrinsic = llvm::Intrinsic::getDeclaration(state->module.get(), intrinsicID);
    llvm::CallInst* call = state->builder.CreateCall(intrinsic, llvmArguments);
    llvm::Value* result = extractsMatchAllPredicate ? state->builder.CreateExtractValue(call, {1})
                                                    : static_cast<llvm::Value*>(call);
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

static SlangResult SLANG_NVVM_CALL _emitArrayElementPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle baseArrayPointer,
    SlangNVVMValueHandle elementIndex,
    SlangNVVMValueHandle* outPointer)
{
    if (outPointer)
        *outPointer = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmBaseArrayPointer = _getValue(baseArrayPointer);
    llvm::Value* llvmElementIndex = _getValue(elementIndex);
    llvm::PointerType* pointerType =
        llvmBaseArrayPointer ? llvm::dyn_cast<llvm::PointerType>(llvmBaseArrayPointer->getType())
                             : nullptr;
    llvm::Type* pointeeType = pointerType && !pointerType->isOpaque()
                                  ? pointerType->getNonOpaquePointerElementType()
                                  : nullptr;
    llvm::ArrayType* arrayType = llvm::dyn_cast_or_null<llvm::ArrayType>(pointeeType);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outPointer || !insertionBlock || !pointerType || pointerType->isOpaque() ||
        !_isNVVMAddressSpace(static_cast<SlangNVVMAddressSpace>(pointerType->getAddressSpace())) ||
        !arrayType || !arrayType->getNumElements() || !arrayType->isSized() || !llvmElementIndex ||
        !llvm::isa<llvm::IntegerType>(llvmElementIndex->getType()) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmBaseArrayPointer) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmElementIndex))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* indices[] = {
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(state->context), 0),
        llvmElementIndex};
    // A Slang subscript does not establish LLVM's stronger inbounds provenance contract.
    llvm::Value* result = state->builder.CreateGEP(arrayType, llvmBaseArrayPointer, indices);
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

static SlangResult SLANG_NVVM_CALL _emitStructFieldValue(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle structValue,
    uint32_t fieldIndex,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmStructValue = _getValue(structValue);
    llvm::StructType* structType =
        llvmStructValue ? llvm::dyn_cast<llvm::StructType>(llvmStructValue->getType()) : nullptr;
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outValue || !insertionBlock || !structType ||
        fieldIndex >= structType->getNumElements() ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmStructValue))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = state->builder.CreateExtractValue(llvmStructValue, {fieldIndex});
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
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

// Writes the legacy LLVM textual dialect accepted by libNVVM's documented LLVM 7 reader.
//
// LLVM 14 made atomic alignment explicit in assembly, but LLVM 7 gives atomicrmw its natural
// alignment and rejects the suffix. LLVM 14 also prints unary negation as `fneg`, which the
// libNVVM NVVM-2.0 reader rejects; the older dialect expresses finite scalar negation as
// `fsub -0.0, value`. Finally, LLVM 14 gives NVVM special-register intrinsics function attributes
// that the LLVM 7 parser does not know. Removing optimization-only attributes retains each
// intrinsic's semantic name and type. LLVM may share one numbered attribute group between several
// declarations, so count unique validated semantic attribute sets. LLVM 14's scalar shuffle and
// synchronized-vote declarations already use the LLVM-7-compatible
// convergent/inaccessible-memory/nounwind set, but validate their exact signatures and attributes
// before serializing the mixed dialect. Generic count-trailing-zeros has the same LLVM 14-only
// optimization attributes as the special-register declarations plus an `immarg` parameter marker;
// LLVM 7 already understands the intrinsic's signature and semantics once those newer attributes
// are removed. The provider exposes exactly one shape of each operation; validate every semantic
// instruction or declaration before changing its spelling.
static SlangResult _writeLegacyNVVMAssembly(
    ModuleState* state,
    llvm::SmallVectorImpl<char>& outSerializedData)
{
    size_t semanticAtomicCount = 0;
    size_t semanticFloatNegateCount = 0;
    size_t semanticCountTrailingZerosDeclarationCount = 0;
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
            bool hasAttributeSet = false;
            for (const llvm::AttributeSet& attributeSet : semanticLegacyIntrinsicAttributeSets)
            {
                if (attributeSet == functionAttributes)
                {
                    hasAttributeSet = true;
                    break;
                }
            }
            if (!hasAttributeSet)
                semanticLegacyIntrinsicAttributeSets.push_back(functionAttributes);
        }
        else if (intrinsicID == llvm::Intrinsic::nvvm_barrier0)
        {
            if (!function.isDeclaration() || !function.getReturnType()->isVoidTy() ||
                function.arg_size() != 0 ||
                function.getAttributes().getFnAttrs().getNumAttributes() != 2 ||
                !function.hasFnAttribute(llvm::Attribute::Convergent) ||
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
        else if (intrinsicID == llvm::Intrinsic::nvvm_match_all_sync_i32p)
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
            llvm::StructType* resultType =
                llvm::dyn_cast<llvm::StructType>(function.getReturnType());
            if (!function.isDeclaration() || !resultType || resultType->getNumElements() != 2 ||
                resultType->getElementType(0) != int32Type ||
                resultType->getElementType(1) != llvm::Type::getInt1Ty(state->context) ||
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
        else if (intrinsicID == llvm::Intrinsic::cttz)
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
            if (!function.isDeclaration() || function.getReturnType() != int32Type ||
                function.arg_size() != 2 || functionAttributes.getNumAttributes() != 6 ||
                !function.hasFnAttribute(llvm::Attribute::NoFree) ||
                !function.hasFnAttribute(llvm::Attribute::NoSync) ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind) ||
                !function.hasFnAttribute(llvm::Attribute::ReadNone) ||
                !function.hasFnAttribute(llvm::Attribute::Speculatable) ||
                !function.hasFnAttribute(llvm::Attribute::WillReturn) ||
                function.getAttributes().getParamAttrs(0).getNumAttributes() != 0 ||
                function.getAttributes().getParamAttrs(1).getNumAttributes() != 1 ||
                !function.hasParamAttribute(1, llvm::Attribute::ImmArg))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            auto argument = function.arg_begin();
            if (argument->getType() != int32Type ||
                (++argument)->getType() != llvm::Type::getInt1Ty(state->context))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
            bool hasAttributeSet = false;
            for (const llvm::AttributeSet& attributeSet : semanticLegacyIntrinsicAttributeSets)
            {
                if (attributeSet == functionAttributes)
                {
                    hasAttributeSet = true;
                    break;
                }
            }
            if (!hasAttributeSet)
                semanticLegacyIntrinsicAttributeSets.push_back(functionAttributes);
            ++semanticCountTrailingZerosDeclarationCount;
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

                auto atomic = llvm::dyn_cast<llvm::AtomicRMWInst>(&instruction);
                if (!atomic)
                    continue;

                ++semanticAtomicCount;
                if (atomic->getOperation() != llvm::AtomicRMWInst::Add ||
                    !atomic->getType()->isIntegerTy(32) ||
                    atomic->getPointerAddressSpace() != SLANG_NVVM_ADDRESS_SPACE_GLOBAL ||
                    atomic->getAlign() != llvm::Align(4) ||
                    atomic->getOrdering() != llvm::AtomicOrdering::Monotonic ||
                    atomic->getSyncScopeID() != llvm::SyncScope::System || atomic->isVolatile())
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
    const llvm::StringRef llvm14AlignmentSuffix(", align 4");
    const llvm::StringRef floatNegateMarker(" = fneg float ");
    const llvm::StringRef legacyFloatNegateMarker(" = fsub float -0.000000e+00, ");
    const llvm::StringRef llvm14SpecialRegisterAttributeMarker(
        " = { nofree nosync nounwind readnone speculatable willreturn }");
    const llvm::StringRef llvm14ExecutionRegisterAttributeMarker(
        " = { nounwind readnone speculatable }");
    const llvm::StringRef legacySpecialRegisterAttributes(" = { nounwind readnone }");
    const llvm::StringRef countTrailingZerosDeclarationMarker("@llvm.cttz.i32(i32, i1 immarg)");
    const llvm::StringRef legacyCountTrailingZerosDeclaration("@llvm.cttz.i32(i32, i1)");
    llvm::StringRef remaining(llvm14Assembly.data(), llvm14Assembly.size());
    size_t rewrittenAtomicCount = 0;
    size_t rewrittenFloatNegateCount = 0;
    size_t rewrittenLegacyIntrinsicAttributeSetCount = 0;
    size_t rewrittenCountTrailingZerosDeclarationCount = 0;
    size_t rewrittenByValueParameterCount = 0;
    while (!remaining.empty())
    {
        const size_t newlineIndex = remaining.find('\n');
        const bool hasNewline = newlineIndex != llvm::StringRef::npos;
        const llvm::StringRef line = hasNewline ? remaining.take_front(newlineIndex) : remaining;

        const llvm::StringRef trimmedLine = line.ltrim();
        if (trimmedLine.startswith("%") && trimmedLine.contains(atomicMarker))
        {
            if (!line.endswith(llvm14AlignmentSuffix))
                return SLANG_E_NOT_AVAILABLE;
            const llvm::StringRef legacyLine = line.drop_back(llvm14AlignmentSuffix.size());
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
        else if (trimmedLine.startswith("declare i32 @llvm.cttz.i32("))
        {
            const size_t markerIndex = line.find(countTrailingZerosDeclarationMarker);
            if (markerIndex == llvm::StringRef::npos)
                return SLANG_E_NOT_AVAILABLE;
            const llvm::StringRef prefix = line.take_front(markerIndex);
            const llvm::StringRef suffix =
                line.drop_front(markerIndex + countTrailingZerosDeclarationMarker.size());
            outSerializedData.append(prefix.begin(), prefix.end());
            outSerializedData.append(
                legacyCountTrailingZerosDeclaration.begin(),
                legacyCountTrailingZerosDeclaration.end());
            outSerializedData.append(suffix.begin(), suffix.end());
            ++rewrittenCountTrailingZerosDeclarationCount;
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

    return rewrittenAtomicCount == semanticAtomicCount &&
                   rewrittenFloatNegateCount == semanticFloatNegateCount &&
                   rewrittenLegacyIntrinsicAttributeSetCount ==
                       semanticLegacyIntrinsicAttributeSets.size() &&
                   rewrittenCountTrailingZerosDeclarationCount ==
                       semanticCountTrailingZerosDeclarationCount &&
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

static bool _getExecutionRegisterIntrinsicIDs(
    SlangNVVMValueOperation operation,
    llvm::Intrinsic::ID (&outIntrinsicIDs)[3])
{
    switch (operation)
    {
    case SLANG_NVVM_VALUE_OP_THREAD_INDEX:
        outIntrinsicIDs[0] = llvm::Intrinsic::nvvm_read_ptx_sreg_tid_x;
        outIntrinsicIDs[1] = llvm::Intrinsic::nvvm_read_ptx_sreg_tid_y;
        outIntrinsicIDs[2] = llvm::Intrinsic::nvvm_read_ptx_sreg_tid_z;
        return true;
    case SLANG_NVVM_VALUE_OP_BLOCK_INDEX:
        outIntrinsicIDs[0] = llvm::Intrinsic::nvvm_read_ptx_sreg_ctaid_x;
        outIntrinsicIDs[1] = llvm::Intrinsic::nvvm_read_ptx_sreg_ctaid_y;
        outIntrinsicIDs[2] = llvm::Intrinsic::nvvm_read_ptx_sreg_ctaid_z;
        return true;
    case SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS:
        outIntrinsicIDs[0] = llvm::Intrinsic::nvvm_read_ptx_sreg_ntid_x;
        outIntrinsicIDs[1] = llvm::Intrinsic::nvvm_read_ptx_sreg_ntid_y;
        outIntrinsicIDs[2] = llvm::Intrinsic::nvvm_read_ptx_sreg_ntid_z;
        return true;
    case SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS:
        outIntrinsicIDs[0] = llvm::Intrinsic::nvvm_read_ptx_sreg_nctaid_x;
        outIntrinsicIDs[1] = llvm::Intrinsic::nvvm_read_ptx_sreg_nctaid_y;
        outIntrinsicIDs[2] = llvm::Intrinsic::nvvm_read_ptx_sreg_nctaid_z;
        return true;
    default:
        return false;
    }
}

static bool _isExecutionRegisterIntrinsic(llvm::Intrinsic::ID intrinsicID)
{
    for (SlangNVVMValueOperation operation = SLANG_NVVM_VALUE_OP_THREAD_INDEX;
         operation <= SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS;
         ++operation)
    {
        llvm::Intrinsic::ID registerIntrinsics[3];
        if (!_getExecutionRegisterIntrinsicIDs(operation, registerIntrinsics))
            return false;
        for (llvm::Intrinsic::ID registerIntrinsic : registerIntrinsics)
        {
            if (registerIntrinsic == intrinsicID)
                return true;
        }
    }
    return false;
}

static SlangResult _emitExecutionRegister(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    ModuleState* state = _getModule(module);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::Intrinsic::ID registerIntrinsics[3];
    if (!state || !outValue || !insertionBlock ||
        !_getExecutionRegisterIntrinsicIDs(operation, registerIntrinsics))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Type* int32Type = llvm::Type::getInt32Ty(state->context);
    llvm::Value* result = llvm::UndefValue::get(llvm::FixedVectorType::get(int32Type, 3));
    for (uint32_t axis = 0; axis < 3; ++axis)
    {
        llvm::Function* intrinsic =
            llvm::Intrinsic::getDeclaration(state->module.get(), registerIntrinsics[axis]);
        llvm::Value* component = state->builder.CreateCall(intrinsic);
        result = state->builder.CreateInsertElement(result, component, axis);
    }
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}

static SlangResult _emitWorkgroupBarrier(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    ModuleState* state = _getModule(module);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!state || !outValue || !insertionBlock)
        return SLANG_E_INVALID_ARG;

    llvm::Function* barrier =
        llvm::Intrinsic::getDeclaration(state->module.get(), llvm::Intrinsic::nvvm_barrier0);
    state->builder.CreateCall(barrier);
    return SLANG_OK;
}

static SlangResult _emitCatalogOperation(
    SlangNVVMModuleHandle module,
    const Slang::NVVMSemantics::CatalogEntry& entry,
    const SlangNVVMValueHandle* operands,
    SlangNVVMValueHandle* outValue)
{
    const SlangNVVMValueOperationDesc operation = Slang::NVVMSemantics::getOperationDesc(entry);
    if (entry.operandCount && entry.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT)
    {
        if (entry.operandCount == 1)
            return _emitFloatingUnary(module, entry.operation, operands[0], outValue);
        if (entry.resultType.kind == SLANG_NVVM_VALUE_TYPE_BOOL)
            return _emitFloatingCompare(
                module,
                entry.operation,
                operands[0],
                operands[1],
                outValue);
        return _emitFloatingBinary(module, entry.operation, operands[0], operands[1], outValue);
    }

    switch (entry.operation)
    {
    case SLANG_NVVM_VALUE_OP_THREAD_INDEX:
    case SLANG_NVVM_VALUE_OP_BLOCK_INDEX:
    case SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS:
    case SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS:
        return _emitExecutionRegister(module, entry.operation, outValue);
    case SLANG_NVVM_VALUE_OP_WORKGROUP_BARRIER:
        return _emitWorkgroupBarrier(module, outValue);
    case SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX:
    case SLANG_NVVM_VALUE_OP_WAVE_LANE_COUNT:
    case SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT:
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT:
    case SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST:
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_IS_FIRST_LANE:
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_ANY_TRUE:
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_TRUE:
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL:
        return _emitIntrinsic(module, operation, operands, entry.operandCount, outValue);
    default:
        return SLANG_E_INVALID_ARG;
    }
}

static llvm::Type* _getSemanticLLVMType(ModuleState* state, const SlangNVVMValueTypeDesc& type)
{
    if (!state)
        return nullptr;

    llvm::Type* scalarType = nullptr;
    switch (type.kind)
    {
    case SLANG_NVVM_VALUE_TYPE_BOOL:
        if (type.bitWidth == 1)
            scalarType = llvm::Type::getInt1Ty(state->context);
        break;
    case SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER:
    case SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER:
        if (type.bitWidth == 8 || type.bitWidth == 16 || type.bitWidth == 32 || type.bitWidth == 64)
        {
            scalarType = llvm::IntegerType::get(state->context, type.bitWidth);
        }
        break;
    case SLANG_NVVM_VALUE_TYPE_FLOATING_POINT:
        if (type.bitWidth == 32)
            scalarType = llvm::Type::getFloatTy(state->context);
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
         type.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT) &&
        type.laneCount >= 2 && type.laneCount <= 4)
    {
        return llvm::FixedVectorType::get(scalarType, type.laneCount);
    }
    return nullptr;
}

static SlangResult _emitNumericFamily(
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

    llvm::Value* llvmOperands[2] = {};
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
        return _emitNumericFamily(module, *operation, resolution.family, operands, outValue);

    const Slang::NVVMSemantics::CatalogEntry* entry = Slang::NVVMSemantics::find(*operation);
    return entry ? _emitCatalogOperation(module, *entry, operands, outValue) : SLANG_E_INVALID_ARG;
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
    api.emitBranch = _emitBranch;
    api.emitConditionalBranch = _emitConditionalBranch;
    api.getIntegerConstant = _getIntegerConstant;
    api.getFloatingPointConstant = _getFloatingPointConstant;
    api.emitPhi = _emitPhi;
    api.addPhiIncoming = _addPhiIncoming;
    api.emitCall = _emitCall;
    api.emitValueReturn = _emitValueReturn;
    api.emitReturnVoid = _emitReturnVoid;
    api.emitPointerOffset = _emitPointerOffset;
    api.emitByteOffsetPointer = _emitByteOffsetPointer;
    api.emitArrayElementPointer = _emitArrayElementPointer;
    api.emitStructFieldPointer = _emitStructFieldPointer;
    api.emitStructFieldValue = _emitStructFieldValue;
    api.emitVectorConstruct = _emitVectorConstruct;
    api.emitVectorElementExtract = _emitVectorElementExtract;
    api.emitRelaxedGlobalI32AtomicAdd = _emitRelaxedGlobalI32AtomicAdd;
    api.declareGlobalStorage = _declareGlobalStorage;
    api.markFunctionAsKernel = _markFunctionAsKernel;
}

static void _fillBuilderValueOperationsAPI(SlangNVVMBuilderValueOperationsAPI& api)
{
    api = {};
    api.isOperationSupported = _isOperationSupported;
    api.emitOperation = _emitOperation;
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
