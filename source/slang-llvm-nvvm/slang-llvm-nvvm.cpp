#include "compiler-core/slang-nvvm-ir-builder-api.h"
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

static SlangResult SLANG_NVVM_CALL _getFloatingPointType(
    SlangNVVMModuleHandle_1 module,
    uint32_t bitWidth,
    SlangNVVMTypeHandle_1* outType)
{
    if (outType)
        *outType = nullptr;

    ModuleState* state = _getModule(module);
    if (!state || !outType || bitWidth != 32)
        return SLANG_E_INVALID_ARG;

    *outType = reinterpret_cast<SlangNVVMTypeHandle_1>(llvm::Type::getFloatTy(state->context));
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

static SlangResult SLANG_NVVM_CALL _getArrayType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle_1* outType)
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

    *outType = reinterpret_cast<SlangNVVMTypeHandle_1>(
        llvm::ArrayType::get(llvmElementType, elementCount));
    return SLANG_OK;
}

// Returns the one structural source of truth for the raw CUDA `RWStructuredBuffer<int>` ABI.
static llvm::StructType* _getRawRWStructuredBufferI32LLVMType(ModuleState* state)
{
    if (!state)
        return nullptr;

    llvm::Type* fields[] = {
        llvm::PointerType::get(
            llvm::Type::getInt32Ty(state->context),
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL),
        llvm::Type::getInt64Ty(state->context),
    };
    return llvm::StructType::get(state->context, fields, false);
}

static SlangResult SLANG_NVVM_CALL
_getRawRWStructuredBufferI32Type(SlangNVVMModuleHandle_1 module, SlangNVVMTypeHandle_1* outType)
{
    if (outType)
        *outType = nullptr;

    ModuleState* state = _getModule(module);
    if (!state || !outType)
        return SLANG_E_INVALID_ARG;

    *outType = reinterpret_cast<SlangNVVMTypeHandle_1>(_getRawRWStructuredBufferI32LLVMType(state));
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
    size_t parameterIndex = 0;
    for (llvm::Argument& parameter : function->args())
    {
        // LLVM 14 prints an unnamed numeric parameter as an explicit `%0` declaration. LLVM 7
        // accepts numeric parameter slots only when they are implicit, while accepting ordinary
        // named parameters. Stable provider-owned names keep the typed module and its textual
        // representation valid in both dialects without parsing a function signature later.
        parameter.setName("slangParameter" + std::to_string(parameterIndex++));
    }
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
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!pointerType || !insertionBlock ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmPointer) ||
        !_isValidAlignment(alignment) || !outValue)
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
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerBinaryOp_2 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmLeft = _getValue(left);
    llvm::Value* llvmRight = _getValue(right);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    const bool isSupportedOperation = operation == SLANG_NVVM_INTEGER_BINARY_OP_ADD ||
                                      operation == SLANG_NVVM_INTEGER_BINARY_OP_SUB;
    if (!outValue || !isSupportedOperation || !insertionBlock ||
        !_areMatchingIntegerValues(state, insertionBlock, llvmLeft, llvmRight))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* result = operation == SLANG_NVVM_INTEGER_BINARY_OP_ADD
                              ? state->builder.CreateAdd(llvmLeft, llvmRight)
                              : state->builder.CreateSub(llvmLeft, llvmRight);
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerMultiply(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
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
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBitAnd(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
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
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBitOr(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
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
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBitXor(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
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
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBitNot(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue)
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
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerNegate(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue)
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
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitRelaxedGlobalI32AtomicAdd(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 pointer,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outOriginalValue)
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
    *outOriginalValue = reinterpret_cast<SlangNVVMValueHandle_1>(originalValue);
    return SLANG_OK;
}

// Emits one scalar-integer comparison after applying the shared ownership and dominance contract.
static SlangResult _emitIntegerComparison(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    llvm::CmpInst::Predicate predicate,
    SlangNVVMValueHandle_1* outValue)
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
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerSignedLessThan(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_SLT, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_EQ, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerNotEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_NE, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerSignedGreaterThan(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_SGT, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerSignedLessEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_SLE, outValue);
}

static SlangResult SLANG_NVVM_CALL _emitIntegerSignedGreaterEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitIntegerComparison(module, left, right, llvm::CmpInst::ICMP_SGE, outValue);
}

// Dispatches the stable V3 scalar families to the same validated producers used by frozen V2.
static SlangResult SLANG_NVVM_CALL _emitIntegerUnaryV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerUnaryOp_3 operation,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue)
{
    if (outValue)
        *outValue = nullptr;
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT:
        return _emitIntegerBitNot(module, value, outValue);
    case SLANG_NVVM_INTEGER_UNARY_OP_NEGATE:
        return _emitIntegerNegate(module, value, outValue);
    default:
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _emitIntegerBinaryV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerBinaryOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    if (outValue)
        *outValue = nullptr;
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_BINARY_OP_3_ADD:
        return _emitIntegerBinary(module, SLANG_NVVM_INTEGER_BINARY_OP_ADD, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_SUB:
        return _emitIntegerBinary(module, SLANG_NVVM_INTEGER_BINARY_OP_SUB, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY:
        return _emitIntegerMultiply(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_AND:
        return _emitIntegerBitAnd(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_OR:
        return _emitIntegerBitOr(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_XOR:
        return _emitIntegerBitXor(module, left, right, outValue);
    default:
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _emitFloatingBinaryV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMFloatingBinaryOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmLeft = _getValue(left);
    llvm::Value* llvmRight = _getValue(right);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock ||
        (operation != SLANG_NVVM_FLOATING_BINARY_OP_ADD &&
         operation != SLANG_NVVM_FLOATING_BINARY_OP_SUBTRACT &&
         operation != SLANG_NVVM_FLOATING_BINARY_OP_MULTIPLY &&
         operation != SLANG_NVVM_FLOATING_BINARY_OP_DIVIDE) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmLeft) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmRight) ||
        llvmLeft->getType() != llvm::Type::getFloatTy(state->context) ||
        llvmRight->getType() != llvmLeft->getType())
    {
        return SLANG_E_INVALID_ARG;
    }

    switch (operation)
    {
    case SLANG_NVVM_FLOATING_BINARY_OP_ADD:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFAdd(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_FLOATING_BINARY_OP_SUBTRACT:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFSub(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_FLOATING_BINARY_OP_MULTIPLY:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFMul(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_FLOATING_BINARY_OP_DIVIDE:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFDiv(llvmLeft, llvmRight));
        return SLANG_OK;
    default:
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _emitFloatingUnaryV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMFloatingUnaryOp_3 operation,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmValue = _getValue(value);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    if (!outValue || !insertionBlock || operation != SLANG_NVVM_FLOATING_UNARY_OP_NEGATE ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmValue) ||
        llvmValue->getType() != llvm::Type::getFloatTy(state->context))
    {
        return SLANG_E_INVALID_ARG;
    }

    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(state->builder.CreateFNeg(llvmValue));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitFloatingCompareV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMFloatingCompareOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
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
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_EQUAL:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFCmpOEQ(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_FLOATING_COMPARE_OP_UNORDERED_NOT_EQUAL:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFCmpUNE(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_THAN:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFCmpOGT(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_EQUAL:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFCmpOLE(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_EQUAL:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFCmpOGE(llvmLeft, llvmRight));
        return SLANG_OK;
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_THAN:
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
            state->builder.CreateFCmpOLT(llvmLeft, llvmRight));
        return SLANG_OK;
    default:
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _emitIntegerCompareV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerCompareOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    if (outValue)
        *outValue = nullptr;
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN:
        return _emitIntegerSignedLessThan(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL:
        return _emitIntegerEqual(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL:
        return _emitIntegerNotEqual(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN:
        return _emitIntegerSignedGreaterThan(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL:
        return _emitIntegerSignedLessEqual(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL:
        return _emitIntegerSignedGreaterEqual(module, left, right, outValue);
    default:
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL
_emitBranch(SlangNVVMModuleHandle_1 module, SlangNVVMBlockHandle_1 targetBlock)
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
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 condition,
    SlangNVVMBlockHandle_1 trueBlock,
    SlangNVVMBlockHandle_1 falseBlock)
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
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 integerType,
    int64_t value,
    SlangNVVMValueHandle_1* outValue)
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

    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
        llvm::ConstantInt::getSigned(llvmIntegerType, value));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _getFloatingPointConstantV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 floatingPointType,
    uint32_t bitWidth,
    uint64_t bitPattern,
    SlangNVVMValueHandle_1* outValue)
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
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(
        llvm::ConstantFP::get(llvmFloatingPointType, value));
    return SLANG_OK;
}

static SlangResult _emitPhiImpl(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMBlockHandle_1 targetBlock,
    SlangNVVMTypeHandle_1 type,
    SlangNVVMValueHandle_1* outValue,
    bool requireInteger)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::BasicBlock* llvmTargetBlock = _getBlock(targetBlock);
    llvm::Type* llvmType = _getType(type);
    const bool isSupportedType =
        llvmType && (llvmType->isIntegerTy() || (!requireInteger && llvmType->isFloatTy()));
    if (!state || !llvmTargetBlock || !llvmTargetBlock->getParent() ||
        llvmTargetBlock->getParent()->getParent() != state->module.get() || !isSupportedType ||
        &llvmType->getContext() != &state->context || !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Instruction* firstNonPhi = llvmTargetBlock->getFirstNonPHI();
    llvm::PHINode* phi = firstNonPhi ? llvm::PHINode::Create(llvmType, 0, "", firstNonPhi)
                                     : llvm::PHINode::Create(llvmType, 0, "", llvmTargetBlock);
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(phi);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerPhi(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMBlockHandle_1 targetBlock,
    SlangNVVMTypeHandle_1 integerType,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitPhiImpl(module, targetBlock, integerType, outValue, true);
}

static SlangResult SLANG_NVVM_CALL _emitPhiV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMBlockHandle_1 targetBlock,
    SlangNVVMTypeHandle_1 type,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitPhiImpl(module, targetBlock, type, outValue, false);
}

static SlangResult _addPhiIncomingImpl(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 phi,
    SlangNVVMValueHandle_1 value,
    SlangNVVMBlockHandle_1 predecessorBlock,
    bool requireInteger)
{
    ModuleState* state = _getModule(module);
    llvm::PHINode* llvmPhi = llvm::dyn_cast_or_null<llvm::PHINode>(_getValue(phi));
    llvm::Value* llvmValue = _getValue(value);
    llvm::BasicBlock* llvmPredecessorBlock = _getBlock(predecessorBlock);
    llvm::BasicBlock* llvmPhiBlock = llvmPhi ? llvmPhi->getParent() : nullptr;
    llvm::Function* llvmFunction = llvmPhiBlock ? llvmPhiBlock->getParent() : nullptr;
    llvm::Instruction* firstNonPhi = llvmPhiBlock ? llvmPhiBlock->getFirstNonPHI() : nullptr;
    const bool isSupportedType = llvmPhi && (llvmPhi->getType()->isIntegerTy() ||
                                             (!requireInteger && llvmPhi->getType()->isFloatTy()));
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

static SlangResult SLANG_NVVM_CALL _addIntegerPhiIncoming(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 phi,
    SlangNVVMValueHandle_1 value,
    SlangNVVMBlockHandle_1 predecessorBlock)
{
    return _addPhiIncomingImpl(module, phi, value, predecessorBlock, true);
}

static SlangResult SLANG_NVVM_CALL _addPhiIncomingV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 phi,
    SlangNVVMValueHandle_1 value,
    SlangNVVMBlockHandle_1 predecessorBlock)
{
    return _addPhiIncomingImpl(module, phi, value, predecessorBlock, false);
}

static bool _isSupportedScalarFunctionType(llvm::Type* type, bool requireInteger)
{
    return type && (type->isIntegerTy() || (!requireInteger && type->isFloatTy()));
}

static SlangResult _emitCallImpl(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 callee,
    const SlangNVVMValueHandle_1* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle_1* outValue,
    bool requireInteger)
{
    if (outValue)
        *outValue = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Function* llvmCallee = llvm::dyn_cast_or_null<llvm::Function>(_getValue(callee));
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::FunctionType* functionType = llvmCallee ? llvmCallee->getFunctionType() : nullptr;
    if (!state || !llvmCallee || llvmCallee->getParent() != state->module.get() ||
        !insertionBlock || !functionType || functionType->isVarArg() ||
        !_isSupportedScalarFunctionType(functionType->getReturnType(), requireInteger) ||
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
        if (!_isSupportedScalarFunctionType(parameterType, requireInteger) || !argument ||
            argument->getType() != parameterType ||
            !_isValueUsableAtInsertionPoint(state, insertionBlock, argument))
        {
            return SLANG_E_INVALID_ARG;
        }
        llvmArguments.push_back(argument);
    }

    llvm::CallInst* call = state->builder.CreateCall(llvmCallee, llvmArguments);
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(call);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitIntegerCall(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 callee,
    const SlangNVVMValueHandle_1* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitCallImpl(module, callee, arguments, argumentCount, outValue, true);
}

static SlangResult SLANG_NVVM_CALL _emitCallV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 callee,
    const SlangNVVMValueHandle_1* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle_1* outValue)
{
    return _emitCallImpl(module, callee, arguments, argumentCount, outValue, false);
}

static SlangResult _emitValueReturnImpl(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    bool requireInteger)
{
    ModuleState* state = _getModule(module);
    llvm::Value* llvmValue = _getValue(value);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::Function* function = insertionBlock ? insertionBlock->getParent() : nullptr;
    if (!state || !llvmValue || !insertionBlock || !function ||
        !_isSupportedScalarFunctionType(llvmValue->getType(), requireInteger) ||
        function->getReturnType() != llvmValue->getType() ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmValue))
    {
        return SLANG_E_INVALID_ARG;
    }

    state->builder.CreateRet(llvmValue);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_emitIntegerReturn(SlangNVVMModuleHandle_1 module, SlangNVVMValueHandle_1 value)
{
    return _emitValueReturnImpl(module, value, true);
}

static SlangResult SLANG_NVVM_CALL
_emitValueReturnV3(SlangNVVMModuleHandle_1 module, SlangNVVMValueHandle_1 value)
{
    return _emitValueReturnImpl(module, value, false);
}

static SlangResult SLANG_NVVM_CALL _emitIntrinsicV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntrinsicOp_3 operation,
    const SlangNVVMValueHandle_1* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle_1* outValue)
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
    switch (operation)
    {
    case SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_INDEX:
        intrinsicID = llvm::Intrinsic::nvvm_read_ptx_sreg_laneid;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_COUNT:
        intrinsicID = llvm::Intrinsic::nvvm_read_ptx_sreg_warpsize;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_UINT:
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_INT:
        intrinsicID = llvm::Intrinsic::nvvm_shfl_sync_idx_i32;
        expectedArgumentCount = 3;
        appendsShuffleClamp = true;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_FLOAT:
        intrinsicID = llvm::Intrinsic::nvvm_shfl_sync_idx_f32;
        expectedArgumentCount = 3;
        expectedArgumentTypes[1] = llvm::Type::getFloatTy(state->context);
        appendsShuffleClamp = true;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_BALLOT:
        intrinsicID = llvm::Intrinsic::nvvm_vote_ballot_sync;
        expectedArgumentCount = 2;
        expectedArgumentTypes[1] = llvm::Type::getInt1Ty(state->context);
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_UINT:
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_INT:
        intrinsicID = llvm::Intrinsic::nvvm_shfl_sync_idx_i32;
        expectedArgumentCount = 2;
        derivesFirstActiveLane = true;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_FLOAT:
        intrinsicID = llvm::Intrinsic::nvvm_shfl_sync_idx_f32;
        expectedArgumentCount = 2;
        expectedArgumentTypes[1] = llvm::Type::getFloatTy(state->context);
        derivesFirstActiveLane = true;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_IS_FIRST_LANE:
        expectedArgumentCount = 1;
        derivesFirstLanePredicate = true;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ANY_TRUE:
        intrinsicID = llvm::Intrinsic::nvvm_vote_any_sync;
        expectedArgumentCount = 2;
        expectedArgumentTypes[1] = llvm::Type::getInt1Ty(state->context);
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_TRUE:
        intrinsicID = llvm::Intrinsic::nvvm_vote_all_sync;
        expectedArgumentCount = 2;
        expectedArgumentTypes[1] = llvm::Type::getInt1Ty(state->context);
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_INT:
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_UINT:
        intrinsicID = llvm::Intrinsic::nvvm_match_all_sync_i32p;
        expectedArgumentCount = 2;
        extractsMatchAllPredicate = true;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_FLOAT:
        intrinsicID = llvm::Intrinsic::nvvm_match_all_sync_i32p;
        expectedArgumentCount = 2;
        expectedArgumentTypes[1] = llvm::Type::getFloatTy(state->context);
        extractsMatchAllPredicate = true;
        bitcastsMatchAllFloatValue = true;
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
        *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(predicate);
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
    *outValue = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitPointerOffset(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 basePointer,
    SlangNVVMValueHandle_1 elementOffset,
    SlangNVVMValueHandle_1* outPointer)
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
    *outPointer = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitArrayElementPointer(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 baseArrayPointer,
    SlangNVVMValueHandle_1 elementIndex,
    SlangNVVMValueHandle_1* outPointer)
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
        !_isNVVMAddressSpace(
            static_cast<SlangNVVMAddressSpace_2>(pointerType->getAddressSpace())) ||
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
    *outPointer = reinterpret_cast<SlangNVVMValueHandle_1>(result);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitRawRWStructuredBufferI32ElementPointer(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 buffer,
    SlangNVVMValueHandle_1 elementIndex,
    SlangNVVMValueHandle_1* outPointer)
{
    if (outPointer)
        *outPointer = nullptr;

    ModuleState* state = _getModule(module);
    llvm::Value* llvmBuffer = _getValue(buffer);
    llvm::Value* llvmElementIndex = _getValue(elementIndex);
    llvm::BasicBlock* insertionBlock = _getValidInsertionBlock(state);
    llvm::StructType* bufferType = _getRawRWStructuredBufferI32LLVMType(state);
    if (!state || !outPointer || !insertionBlock || !llvmBuffer || !llvmElementIndex ||
        llvmBuffer->getType() != bufferType || !llvmElementIndex->getType()->isIntegerTy(32) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmBuffer) ||
        !_isValueUsableAtInsertionPoint(state, insertionBlock, llvmElementIndex))
    {
        return SLANG_E_INVALID_ARG;
    }

    llvm::Value* dataPointer = state->builder.CreateExtractValue(llvmBuffer, {0});
    // A Slang structured-buffer index does not establish LLVM's stronger inbounds provenance.
    llvm::Value* result = state->builder.CreateGEP(
        llvm::Type::getInt32Ty(state->context),
        dataPointer,
        llvmElementIndex);
    *outPointer = reinterpret_cast<SlangNVVMValueHandle_1>(result);
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
    llvm::SmallVector<llvm::AttributeSet, 2> semanticLegacyIntrinsicAttributeSets;
    for (llvm::Function& function : *state->module)
    {
        const llvm::Intrinsic::ID intrinsicID = function.getIntrinsicID();
        if (intrinsicID == llvm::Intrinsic::nvvm_read_ptx_sreg_laneid ||
            intrinsicID == llvm::Intrinsic::nvvm_read_ptx_sreg_warpsize)
        {
            const llvm::AttributeSet functionAttributes = function.getAttributes().getFnAttrs();
            if (!function.isDeclaration() || !function.getReturnType()->isIntegerTy(32) ||
                function.arg_size() != 0 || functionAttributes.getNumAttributes() != 6 ||
                !function.hasFnAttribute(llvm::Attribute::NoFree) ||
                !function.hasFnAttribute(llvm::Attribute::NoSync) ||
                !function.hasFnAttribute(llvm::Attribute::NoUnwind) ||
                !function.hasFnAttribute(llvm::Attribute::ReadNone) ||
                !function.hasFnAttribute(llvm::Attribute::Speculatable) ||
                !function.hasFnAttribute(llvm::Attribute::WillReturn))
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
    const llvm::StringRef legacySpecialRegisterAttributes(" = { nounwind readnone }");
    const llvm::StringRef countTrailingZerosDeclarationMarker("@llvm.cttz.i32(i32, i1 immarg)");
    const llvm::StringRef legacyCountTrailingZerosDeclaration("@llvm.cttz.i32(i32, i1)");
    llvm::StringRef remaining(llvm14Assembly.data(), llvm14Assembly.size());
    size_t rewrittenAtomicCount = 0;
    size_t rewrittenFloatNegateCount = 0;
    size_t rewrittenLegacyIntrinsicAttributeSetCount = 0;
    size_t rewrittenCountTrailingZerosDeclarationCount = 0;
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
            line.endswith(llvm14SpecialRegisterAttributeMarker))
        {
            const llvm::StringRef prefix =
                line.drop_back(llvm14SpecialRegisterAttributeMarker.size());
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
                       semanticCountTrailingZerosDeclarationCount
               ? SLANG_OK
               : SLANG_E_NOT_AVAILABLE;
}

// Verifies once and materializes the one canonical byte result shared by the V1 and V2 getters.
static SlangResult _materializeModule(
    ModuleState* state,
    SlangNVVMSerializationFormat_1 format,
    bool useNVVMIR20Assembly,
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
    SlangNVVMModuleHandle_1 module,
    SlangNVVMSerializationFormat_1 format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus_2* outVerificationStatus,
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
    SlangNVVMVerificationStatus_2 verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
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

// Fills the complete frozen V2 table. Both standalone V2 and V3 use this exact compatibility core.
static void _fillBuilderAPIV2(SlangNVVMBuilderAPI_V2& api)
{
    api = {};
    api.structureSize = uint32_t(sizeof(api));
    api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_2;
    _fillBuilderAPIV1(api.baseAPI);
    api.serializeModuleWithDiagnostics = _serializeModuleWithDiagnostics;
    api.getIntegerType = _getIntegerType;
    api.getPointerType = _getPointerType;
    api.getFunctionParameter = _getFunctionParameter;
    api.emitLoad = _emitLoad;
    api.emitStore = _emitStore;
    api.emitIntegerBinary = _emitIntegerBinary;
    api.emitIntegerSignedLessThan = _emitIntegerSignedLessThan;
    api.emitBranch = _emitBranch;
    api.emitConditionalBranch = _emitConditionalBranch;
    api.getIntegerConstant = _getIntegerConstant;
    api.emitIntegerPhi = _emitIntegerPhi;
    api.addIntegerPhiIncoming = _addIntegerPhiIncoming;
    api.emitIntegerCall = _emitIntegerCall;
    api.emitIntegerReturn = _emitIntegerReturn;
    api.emitPointerOffset = _emitPointerOffset;
    api.getArrayType = _getArrayType;
    api.emitArrayElementPointer = _emitArrayElementPointer;
    api.emitIntegerMultiply = _emitIntegerMultiply;
    api.emitIntegerBitAnd = _emitIntegerBitAnd;
    api.emitIntegerBitOr = _emitIntegerBitOr;
    api.emitIntegerBitXor = _emitIntegerBitXor;
    api.emitIntegerBitNot = _emitIntegerBitNot;
    api.emitIntegerNegate = _emitIntegerNegate;
    api.emitRelaxedGlobalI32AtomicAdd = _emitRelaxedGlobalI32AtomicAdd;
    api.serializeNVVMIR20AssemblyWithDiagnostics = _serializeNVVMIR20AssemblyWithDiagnostics;
    api.emitIntegerEqual = _emitIntegerEqual;
    api.emitIntegerNotEqual = _emitIntegerNotEqual;
    api.emitIntegerSignedGreaterThan = _emitIntegerSignedGreaterThan;
    api.emitIntegerSignedLessEqual = _emitIntegerSignedLessEqual;
    api.emitIntegerSignedGreaterEqual = _emitIntegerSignedGreaterEqual;
    api.getRawRWStructuredBufferI32Type = _getRawRWStructuredBufferI32Type;
    api.emitRawRWStructuredBufferI32ElementPointer = _emitRawRWStructuredBufferI32ElementPointer;
}

static void _addFeature(SlangNVVMBuilderFeatureSet_3& features, SlangNVVMBuilderFeature_3 feature)
{
    features.words[feature / 64u] |= uint64_t(1) << (feature % 64u);
}

static void _fillBuilderAPIV3(SlangNVVMBuilderAPI_V3& api)
{
    api = {};
    api.structureSize = uint32_t(sizeof(api));
    api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_3;
    _fillBuilderAPIV2(api.compatibilityAPI);
    for (SlangNVVMBuilderFeature_3 feature = 0; feature < SLANG_NVVM_BUILDER_FEATURE_COUNT_3;
         ++feature)
    {
        _addFeature(api.features, feature);
    }
    api.emitIntegerUnary = _emitIntegerUnaryV3;
    api.emitIntegerBinary = _emitIntegerBinaryV3;
    api.emitIntegerCompare = _emitIntegerCompareV3;
    api.getFloatingPointType = _getFloatingPointType;
    api.emitFloatingBinary = _emitFloatingBinaryV3;
    api.emitFloatingUnary = _emitFloatingUnaryV3;
    api.emitFloatingCompare = _emitFloatingCompareV3;
    api.getFloatingPointConstant = _getFloatingPointConstantV3;
    api.emitPhi = _emitPhiV3;
    api.addPhiIncoming = _addPhiIncomingV3;
    api.emitCall = _emitCallV3;
    api.emitValueReturn = _emitValueReturnV3;
    api.emitIntrinsic = _emitIntrinsicV3;
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
    SlangNVVMBuilderAPI_V2 api;
    _fillBuilderAPIV2(api);

    const size_t copySize = callerCapacity < sizeof(api) ? callerCapacity : sizeof(api);
    std::memcpy(outAPI, &api, copySize);
    return SLANG_OK;
}

extern "C" SLANG_NVVM_BUILDER_API SlangResult SLANG_NVVM_CALL
slang_getNVVMBuilderAPI_V3(SlangNVVMBuilderAPI_V3* outAPI)
{
    if (!outAPI || outAPI->structureSize < SLANG_NVVM_BUILDER_API_V3_MIN_SIZE ||
        outAPI->abiVersion != SLANG_NVVM_BUILDER_ABI_VERSION_3)
    {
        return SLANG_E_NO_INTERFACE;
    }

    const size_t callerCapacity = outAPI->structureSize;
    SlangNVVMBuilderAPI_V3 api;
    _fillBuilderAPIV3(api);
    const size_t copySize = callerCapacity < sizeof(api) ? callerCapacity : sizeof(api);
    std::memcpy(outAPI, &api, copySize);
    return SLANG_OK;
}
