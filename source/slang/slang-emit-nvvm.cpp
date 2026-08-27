#include "slang-emit-nvvm.h"

#include "compiler-core/slang-artifact-impl.h"
#include "compiler-core/slang-artifact-util.h"
#include "core/slang-dictionary.h"
#include "slang-code-gen.h"
#include "slang-diagnostics.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
namespace
{

// The direct scalar ABI accepts only CUDA `int`, whose width and natural alignment are four bytes.
static const uint32_t kNVVMI32Alignment = 4;
static const IRIntegerValue kNVVMI32Min = -2147483647 - 1;
static const IRIntegerValue kNVVMI32Max = 2147483647;

struct ScopedNVVMModule
{
    const NVVMIRBuilder* builder = nullptr;
    SlangNVVMModuleHandle_1 module = nullptr;

    ~ScopedNVVMModule()
    {
        if (builder && module)
            builder->destroyModule(module);
    }
};

SlangResult _diagnoseUnsupportedIR(
    CodeGenContext* codeGenContext,
    const UnownedStringSlice& construct)
{
    codeGenContext->getSink()->diagnose(
        Diagnostics::NvvmUnsupportedIr{.construct = String(construct)});
    return SLANG_E_NOT_IMPLEMENTED;
}

SlangResult _requireBuilderOperation(
    CodeGenContext* codeGenContext,
    const char* operation,
    SlangResult result)
{
    if (SLANG_SUCCEEDED(result))
        return result;

    codeGenContext->getSink()->diagnose(Diagnostics::NvvmIrBuilderOperationFailed{
        .operation = String(operation),
        .resultCode = result,
    });
    return result;
}

// Returns whether `type` is the canonical signed 32-bit integer type accepted by this slice.
bool _isI32Type(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Int;
}

// Returns an executable signed-i32 literal, excluding layout and other module constants.
IRIntLit* _asExecutableI32Constant(IRInst* value)
{
    auto intLit = as<IRIntLit>(value);
    if (!intLit || !_isI32Type(intLit->getDataType()))
        return nullptr;

    const IRIntegerValue intValue = intLit->getValue();
    return intValue >= kNVVMI32Min && intValue <= kNVVMI32Max ? intLit : nullptr;
}

// Returns whether `type` is the canonical Boolean result type produced by signed comparison.
bool _isBoolType(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Bool;
}

// Returns the accepted nonempty fixed i32 array and its exact provider-representable count.
IRArrayType* _asSupportedI32ArrayType(IRInst* type, uint32_t* outElementCount = nullptr)
{
    if (outElementCount)
        *outElementCount = 0;

    auto arrayType = as<IRArrayType>(type);
    if (!arrayType || arrayType->getOp() != kIROp_ArrayType || arrayType->getOperandCount() != 2 ||
        !_isI32Type(arrayType->getElementType()))
    {
        return nullptr;
    }

    auto elementCount = as<IRIntLit>(arrayType->getElementCount());
    if (!elementCount || elementCount->getValue() <= 0 || elementCount->getValue() > UINT32_MAX)
        return nullptr;

    if (outElementCount)
        *outElementCount = uint32_t(elementCount->getValue());
    return arrayType;
}

// Returns the accepted CUDA device-pointer type, or null for every other pointer spelling.
IRPtrTypeBase* _asSupportedDevicePointerType(IRInst* type)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    if (!ptrType || ptrType->getOp() != kIROp_PtrType || !_isI32Type(ptrType->getValueType()) ||
        ptrType->getAddressSpace() != AddressSpace::UserPointer)
    {
        return nullptr;
    }

    const AccessQualifier access = ptrType->getAccessQualifier();
    return access == AccessQualifier::Read || access == AccessQualifier::ReadWrite ? ptrType
                                                                                   : nullptr;
}

// Returns a device pointer to an accepted fixed i32 array, preserving its canonical array type.
IRPtrTypeBase* _asSupportedDeviceArrayPointerType(
    IRInst* type,
    IRArrayType** outArrayType = nullptr,
    uint32_t* outElementCount = nullptr)
{
    if (outArrayType)
        *outArrayType = nullptr;
    if (outElementCount)
        *outElementCount = 0;

    auto ptrType = as<IRPtrTypeBase>(type);
    IRArrayType* arrayType = nullptr;
    uint32_t elementCount = 0;
    if (!ptrType || ptrType->getOp() != kIROp_PtrType ||
        !(arrayType = _asSupportedI32ArrayType(ptrType->getValueType(), &elementCount)) ||
        ptrType->getAddressSpace() != AddressSpace::UserPointer)
    {
        return nullptr;
    }

    const AccessQualifier access = ptrType->getAccessQualifier();
    if (access != AccessQualifier::Read && access != AccessQualifier::ReadWrite)
        return nullptr;

    if (outArrayType)
        *outArrayType = arrayType;
    if (outElementCount)
        *outElementCount = elementCount;
    return ptrType;
}

// Returns the exact raw CUDA `RWStructuredBuffer<int, DefaultLayout>` launch-value type.
IRHLSLStructuredBufferTypeBase* _asSupportedRawRWStructuredBufferI32Type(IRInst* type)
{
    auto bufferType = as<IRHLSLStructuredBufferTypeBase>(type);
    if (!bufferType || bufferType->getOp() != kIROp_HLSLRWStructuredBufferType ||
        bufferType->getOperandCount() != 3 || !_isI32Type(bufferType->getElementType()))
    {
        return nullptr;
    }

    IRType* dataLayout = bufferType->getDataLayout();
    return dataLayout && dataLayout->getOp() == kIROp_DefaultBufferLayoutType ? bufferType
                                                                              : nullptr;
}

// Returns the canonical generic scalar-layout pointer produced by structured-buffer addressing.
IRPtrTypeBase* _asSupportedRWStructuredBufferI32ElementPointerType(IRInst* type)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    IRType* dataLayout = ptrType ? ptrType->getDataLayout() : nullptr;
    if (!ptrType || ptrType->getOp() != kIROp_PtrType || ptrType->getOperandCount() != 4 ||
        !_isI32Type(ptrType->getValueType()) ||
        ptrType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        ptrType->getAddressSpace() != AddressSpace::Generic || !dataLayout ||
        dataLayout->getOp() != kIROp_ScalarBufferLayoutType)
    {
        return nullptr;
    }
    return ptrType;
}

// Returns whether `type` has a direct CUDA launch-parameter representation.
bool _isSupportedParameterType(IRInst* type)
{
    return _isI32Type(type) || _asSupportedDevicePointerType(type) ||
           _asSupportedDeviceArrayPointerType(type) ||
           _asSupportedRawRWStructuredBufferI32Type(type);
}

// Raises the required builder prefix without weakening an already stronger requirement.
void _requireCapability(NVVMIRCapability& capability, NVVMIRCapability requiredCapability)
{
    if (int(requiredCapability) > int(capability))
        capability = requiredCapability;
}

// Checks that an executable operand has an accepted definition that dominates its use.
SlangResult _validateAvailableValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (value && consumer && dominatorTree && availableValues.contains(value) &&
        dominatorTree->dominates(value, consumer))
    {
        return SLANG_OK;
    }

    return _diagnoseUnsupportedIR(
        codeGenContext,
        value ? UnownedStringSlice(getIROpInfo(value->getOp()).name) : toSlice("missing operand"));
}

// Checks that an executable operand is an available signed 32-bit value.
SlangResult _validateI32Value(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree,
    NVVMIRCapability& capability)
{
    if (!value || !_isI32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 value"));

    if (_asExecutableI32Constant(value))
    {
        _requireCapability(capability, NVVMIRCapability::ScalarSSA);
        return SLANG_OK;
    }

    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks an available device pointer and enforces the source access qualifier for stores.
SlangResult _validatePointerValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree,
    bool requireWriteAccess)
{
    auto ptrType = value ? _asSupportedDevicePointerType(value->getDataType()) : nullptr;
    auto resourceElementPtrType =
        value ? _asSupportedRWStructuredBufferI32ElementPointerType(value->getDataType()) : nullptr;
    if (!ptrType &&
        (!resourceElementPtrType || value->getOp() != kIROp_RWStructuredBufferGetElementPtr))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("device i32 pointer"));
    if (resourceElementPtrType && consumer->getOp() != kIROp_Store)
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            toSlice("raw RWStructuredBuffer signed i32 store consumer"));
    }
    if (requireWriteAccess && ptrType &&
        ptrType->getAccessQualifier() != AccessQualifier::ReadWrite)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("read-only pointer store"));
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks that a branch destination is a block declared by the selected function.
SlangResult _validateBlockTarget(
    CodeGenContext* codeGenContext,
    IRBlock* block,
    const HashSet<IRBlock*>& functionBlocks)
{
    if (block && functionBlocks.contains(block))
        return SLANG_OK;
    return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch target"));
}

// Orders reachable bodies by CFG dominance, then preserves physical order for unreachable bodies.
List<IRBlock*> _getNVVMBodyOrder(IRFunc* function, IRDominatorTree* dominatorTree)
{
    List<IRBlock*> result;
    HashSet<IRBlock*> addedBlocks;
    for (auto block : getReversePostorder(function))
    {
        if (!dominatorTree->isUnreachable(block) && addedBlocks.add(block))
            result.add(block);
    }
    for (auto block : function->getBlocks())
    {
        if (addedBlocks.add(block))
            result.add(block);
    }
    return result;
}

// Counts the positional SSA values a branch to `block` must provide.
UInt _getBlockParamCount(IRBlock* block)
{
    UInt count = 0;
    for (auto param : block->getParams())
    {
        SLANG_UNUSED(param);
        ++count;
    }
    return count;
}

// Validates the positional SSA values carried by an actual branch edge.
SlangResult _validateBranchArguments(
    CodeGenContext* codeGenContext,
    IRUnconditionalBranch* branch,
    IRBlock* entryBlock,
    const HashSet<IRBlock*>& functionBlocks,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree,
    NVVMIRCapability& capability)
{
    IRBlock* targetBlock = branch->getTargetBlock();
    SLANG_RETURN_ON_FAIL(_validateBlockTarget(codeGenContext, targetBlock, functionBlocks));
    if (targetBlock == entryBlock)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-block branch target"));

    const UInt argumentCount = branch->getArgCount();
    if (argumentCount != _getBlockParamCount(targetBlock))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch argument count"));

    IRParam* targetParam = targetBlock->getFirstParam();
    for (UInt argumentIndex = 0; argumentIndex < argumentCount;
         ++argumentIndex, targetParam = targetParam->getNextParam())
    {
        IRInst* argument = branch->getArg(argumentIndex);
        SLANG_ASSERT(targetParam);
        if (!argument || !isTypeEqual(argument->getDataType(), targetParam->getDataType()))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch argument type"));
        SLANG_RETURN_ON_FAIL(_validateI32Value(
            codeGenContext,
            argument,
            branch,
            availableValues,
            dominatorTree,
            capability));
    }

    if (argumentCount)
        _requireCapability(capability, NVVMIRCapability::ScalarSSA);
    return SLANG_OK;
}

// Returns the LLVM symbol chosen from the canonical linked IR for an accepted function.
UnownedStringSlice _getNVVMFunctionName(IRFunc* function, IRFunc* entryPoint)
{
    if (function == entryPoint)
    {
        auto entryPointDecoration = function->findDecoration<IREntryPointDecoration>();
        SLANG_RELEASE_ASSERT(entryPointDecoration);
        return entryPointDecoration->getName()->getStringSlice();
    }
    return getMangledName(function);
}

// Checks the exact helper ABI before adding a direct callee to the accepted closure.
SlangResult _validateNVVMHelperTarget(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    IRFunc* entryPoint,
    IRFunc* helper)
{
    if (!helper)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));
    if (helper == entryPoint || helper->findDecoration<IREntryPointDecoration>() ||
        helper->findDecoration<IRCudaKernelDecoration>())
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));
    }
    if (helper->getParent() != linkedIR.module->getModuleInst() || !helper->isDefinition())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));
    if (!_isI32Type(helper->getResultType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("helper function result type"));
    for (UInt parameterIndex = 0; parameterIndex < helper->getParamCount(); ++parameterIndex)
    {
        if (!_isI32Type(helper->getParamType(parameterIndex)))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("helper function parameter"));
    }
    return SLANG_OK;
}

// Visits the exact direct-call graph and records each reachable function once in preorder.
SlangResult _visitNVVMFunction(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    IRFunc* entryPoint,
    IRFunc* function,
    List<IRFunc*>& functions,
    HashSet<IRFunc*>& functionSet,
    HashSet<IRFunc*>& activeFunctions,
    HashSet<IRFunc*>& completedFunctions)
{
    if (completedFunctions.contains(function))
        return SLANG_OK;
    if (!activeFunctions.add(function))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("recursive function call"));
    if (functionSet.add(function))
        functions.add(function);

    for (auto block : function->getBlocks())
    {
        for (auto inst : block->getOrdinaryInsts())
        {
            auto call = as<IRCall>(inst);
            if (!call)
                continue;
            if (!call->getOperandCount())
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));

            auto helper = as<IRFunc>(call->getOperand(0));
            SLANG_RETURN_ON_FAIL(
                _validateNVVMHelperTarget(codeGenContext, linkedIR, entryPoint, helper));
            if (activeFunctions.contains(helper))
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("recursive function call"));
            SLANG_RETURN_ON_FAIL(_visitNVVMFunction(
                codeGenContext,
                linkedIR,
                entryPoint,
                helper,
                functions,
                functionSet,
                activeFunctions,
                completedFunctions));
        }
    }

    activeFunctions.remove(function);
    completedFunctions.add(function);
    return SLANG_OK;
}

// Collects the finite direct-call closure rooted at the sole selected entry point.
SlangResult _collectNVVMFunctions(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    IRFunc* entryPoint,
    List<IRFunc*>& functions,
    HashSet<IRFunc*>& functionSet)
{
    HashSet<IRFunc*> activeFunctions;
    HashSet<IRFunc*> completedFunctions;
    return _visitNVVMFunction(
        codeGenContext,
        linkedIR,
        entryPoint,
        entryPoint,
        functions,
        functionSet,
        activeFunctions,
        completedFunctions);
}

// Checks that function values remain direct callees rather than becoming first-class data.
SlangResult _validateNVVMFunctionUses(
    CodeGenContext* codeGenContext,
    const List<IRFunc*>& functions)
{
    for (auto function : functions)
    {
        for (auto use = function->firstUse; use; use = use->nextUse)
        {
            auto call = as<IRCall>(use->getUser());
            if (!call || use != call->getCalleeUse())
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("function value use"));
        }
    }
    return SLANG_OK;
}

// Checks that every accepted function has a distinct canonical symbol before provider discovery.
SlangResult _validateNVVMFunctionNames(
    CodeGenContext* codeGenContext,
    IRFunc* entryPoint,
    const List<IRFunc*>& functions)
{
    HashSet<String> names;
    for (auto function : functions)
    {
        UnownedStringSlice name = _getNVVMFunctionName(function, entryPoint);
        if (!name.getLength() || !names.add(String(name)))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("function name"));
    }
    return SLANG_OK;
}

// Checks one function body using the same block and SSA order that emission will use.
SlangResult _validateNVVMFunction(
    CodeGenContext* codeGenContext,
    IRFunc* entryPoint,
    IRFunc* function,
    const HashSet<IRFunc*>& functionSet,
    NVVMIRCapability& capability)
{
    const bool isEntryPoint = function == entryPoint;
    IRBlock* entryBlock = function->getFirstBlock();
    if (!entryBlock)
        return _diagnoseUnsupportedIR(
            codeGenContext,
            isEntryPoint ? toSlice("entry block") : toSlice("helper entry block"));

    HashSet<IRBlock*> functionBlocks;
    for (auto block : function->getBlocks())
        functionBlocks.add(block);
    if (functionBlocks.getCount() > 1)
        _requireCapability(capability, NVVMIRCapability::ScalarControlFlow);

    RefPtr<IRDominatorTree> dominatorTree = computeDominatorTree(function);
    List<IRBlock*> bodyOrder = _getNVVMBodyOrder(function, dominatorTree);
    for (auto block : bodyOrder)
    {
        if (!functionBlocks.contains(block))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch target"));
    }

    HashSet<IRInst*> availableValues;
    UInt actualParamCount = 0;
    for (auto param : function->getParams())
    {
        auto arrayPointerType =
            isEntryPoint ? _asSupportedDeviceArrayPointerType(param->getDataType()) : nullptr;
        auto rawRWStructuredBufferType =
            isEntryPoint ? _asSupportedRawRWStructuredBufferI32Type(param->getDataType()) : nullptr;
        const bool isSupportedType = isEntryPoint ? _isSupportedParameterType(param->getDataType())
                                                  : _isI32Type(param->getDataType());
        if (actualParamCount >= function->getParamCount() || !isSupportedType ||
            !isTypeEqual(param->getDataType(), function->getParamType(actualParamCount)))
        {
            return _diagnoseUnsupportedIR(
                codeGenContext,
                isEntryPoint ? toSlice("entry-point parameter")
                             : toSlice("helper function parameter"));
        }
        if (arrayPointerType)
            _requireCapability(capability, NVVMIRCapability::ScalarArrayAddressing);
        if (rawRWStructuredBufferType)
            _requireCapability(capability, NVVMIRCapability::RawRWStructuredBufferI32);
        availableValues.add(param);
        ++actualParamCount;
    }
    if (actualParamCount != function->getParamCount())
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            isEntryPoint ? toSlice("entry-point parameter count")
                         : toSlice("helper parameter count"));
    }
    if (isEntryPoint && actualParamCount)
        _requireCapability(capability, NVVMIRCapability::ScalarMemory);

    // Register every accepted block parameter before checking uses because emission creates all
    // phi placeholders before any body. Ordinary values join this set in the second pass, in the
    // same order in which their LLVM instructions will be emitted.
    for (auto block : function->getBlocks())
    {
        if (block != entryBlock)
        {
            for (auto param : block->getParams())
            {
                if (!_isI32Type(param->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block parameter"));
                availableValues.add(param);
                _requireCapability(capability, NVVMIRCapability::ScalarSSA);
            }
        }

        IRTerminatorInst* terminator = block->getTerminator();
        if (!terminator)
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("missing terminator"));

        for (auto inst : block->getOrdinaryInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_Load:
                if (!_isI32Type(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("load result type"));
                _requireCapability(capability, NVVMIRCapability::ScalarMemory);
                break;

            case kIROp_Store:
                _requireCapability(capability, NVVMIRCapability::ScalarMemory);
                break;

            case kIROp_Add:
            case kIROp_Sub:
                if (inst->getOperandCount() != 2 || !_isI32Type(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 arithmetic"));
                _requireCapability(capability, NVVMIRCapability::ScalarControlFlow);
                break;

            case kIROp_Mul:
                if (inst->getOperandCount() != 2 || !_isI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 multiplication"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerMultiply);
                break;

            case kIROp_BitAnd:
                if (inst->getOperandCount() != 2 || !_isI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 bitwise AND"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerBitAnd);
                break;

            case kIROp_BitOr:
                if (inst->getOperandCount() != 2 || !_isI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 bitwise OR"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerBitOr);
                break;

            case kIROp_BitXor:
                if (inst->getOperandCount() != 2 || !_isI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 bitwise XOR"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerBitXor);
                break;

            case kIROp_BitNot:
                if (inst->getOperandCount() != 1 || !_isI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 bitwise NOT"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerBitNot);
                break;

            case kIROp_Neg:
                if (inst->getOperandCount() != 1 || !_isI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 arithmetic negation"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerNegate);
                break;

            case kIROp_AtomicAdd:
                {
                    if (inst->getOperandCount() != 3 || !_isI32Type(inst->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("relaxed global signed i32 atomic add"));
                    }
                    auto memoryOrder = _asExecutableI32Constant(inst->getOperand(2));
                    if (!memoryOrder || memoryOrder->getValue() != kIRMemoryOrder_Relaxed)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("relaxed atomic-add memory order"));
                    }
                }
                break;

            case kIROp_Less:
                if (inst->getOperandCount() != 2 || !_isBoolType(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 comparison"));
                _requireCapability(capability, NVVMIRCapability::ScalarControlFlow);
                break;

            case kIROp_Eql:
                if (inst->getOperandCount() != 2 || !_isBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 equality"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerEqual);
                break;

            case kIROp_Neq:
                if (inst->getOperandCount() != 2 || !_isBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 inequality"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerNotEqual);
                break;

            case kIROp_Greater:
                if (inst->getOperandCount() != 2 || !_isBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 greater-than"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerSignedGreaterThan);
                break;

            case kIROp_Leq:
                if (inst->getOperandCount() != 2 || !_isBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 less-than-or-equal"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerSignedLessEqual);
                break;

            case kIROp_Geq:
                if (inst->getOperandCount() != 2 || !_isBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 greater-than-or-equal"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarIntegerSignedGreaterEqual);
                break;

            case kIROp_Call:
                if (!inst->getOperandCount() || !_isI32Type(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 call"));
                _requireCapability(capability, NVVMIRCapability::ScalarFunctions);
                break;

            case kIROp_GetOffsetPtr:
                if (inst->getOperandCount() != 2 ||
                    !_asSupportedDevicePointerType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("device i32 pointer offset"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarPointerArithmetic);
                break;

            case kIROp_GetElementPtr:
                if (inst->getOperandCount() != 2 ||
                    !_asSupportedDevicePointerType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("device i32 array element pointer"));
                }
                _requireCapability(capability, NVVMIRCapability::ScalarArrayAddressing);
                break;

            case kIROp_RWStructuredBufferGetElementPtr:
                if (inst->getOperandCount() != 2 ||
                    !_asSupportedRWStructuredBufferI32ElementPointerType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("raw RWStructuredBuffer signed i32 element pointer"));
                }
                _requireCapability(capability, NVVMIRCapability::RawRWStructuredBufferI32);
                break;

            case kIROp_Return:
                break;

            case kIROp_UnconditionalBranch:
            case kIROp_Loop:
            case kIROp_IfElse:
                _requireCapability(capability, NVVMIRCapability::ScalarControlFlow);
                break;

            default:
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    UnownedStringSlice(getIROpInfo(inst->getOp()).name));
            }
        }
    }

    bool hasHelperReturn = false;
    // Reachable reverse postorder puts every dominating ordinary producer before its consumer
    // without making physical sibling order part of legality. Unreachable blocks retain physical
    // order, and phi definitions are already available in every block.
    for (auto block : bodyOrder)
    {
        IRTerminatorInst* terminator = block->getTerminator();
        SLANG_ASSERT(terminator);

        for (auto inst : block->getOrdinaryInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_Load:
                {
                    auto load = cast<IRLoad>(inst);
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        load->getPtr(),
                        load,
                        availableValues,
                        dominatorTree,
                        false));
                    availableValues.add(load);
                }
                break;

            case kIROp_Store:
                {
                    auto store = cast<IRStore>(inst);
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        store->getPtr(),
                        store,
                        availableValues,
                        dominatorTree,
                        true));
                    SLANG_RETURN_ON_FAIL(_validateI32Value(
                        codeGenContext,
                        store->getVal(),
                        store,
                        availableValues,
                        dominatorTree,
                        capability));
                }
                break;

            case kIROp_Add:
            case kIROp_Sub:
            case kIROp_Mul:
            case kIROp_BitAnd:
            case kIROp_BitOr:
            case kIROp_BitXor:
            case kIROp_Less:
            case kIROp_Eql:
            case kIROp_Neq:
            case kIROp_Greater:
            case kIROp_Leq:
            case kIROp_Geq:
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree,
                    capability));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree,
                    capability));
                availableValues.add(inst);
                break;

            case kIROp_BitNot:
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree,
                    capability));
                availableValues.add(inst);
                break;

            case kIROp_Neg:
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree,
                    capability));
                availableValues.add(inst);
                break;

            case kIROp_AtomicAdd:
                // Operand two is the literal Relaxed policy validated in the shape pass, not an
                // SSA value that the provider should receive.
                SLANG_RETURN_ON_FAIL(_validatePointerValue(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree,
                    true));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree,
                    capability));
                _requireCapability(capability, NVVMIRCapability::RelaxedGlobalI32AtomicAdd);
                availableValues.add(inst);
                break;

            case kIROp_Call:
                {
                    auto call = cast<IRCall>(inst);
                    auto callee = as<IRFunc>(call->getOperand(0));
                    if (!callee || callee == entryPoint || !functionSet.contains(callee) ||
                        !isTypeEqual(call->getDataType(), callee->getResultType()) ||
                        call->getArgCount() != callee->getParamCount())
                    {
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("direct i32 call"));
                    }
                    for (UInt argumentIndex = 0; argumentIndex < call->getArgCount();
                         ++argumentIndex)
                    {
                        IRInst* argument = call->getArg(argumentIndex);
                        if (!argument || !isTypeEqual(
                                             argument->getDataType(),
                                             callee->getParamType(argumentIndex)))
                        {
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("call argument type"));
                        }
                        SLANG_RETURN_ON_FAIL(_validateI32Value(
                            codeGenContext,
                            argument,
                            call,
                            availableValues,
                            dominatorTree,
                            capability));
                    }
                    availableValues.add(call);
                }
                break;

            case kIROp_GetOffsetPtr:
                {
                    IRInst* basePointer = inst->getOperand(0);
                    IRInst* elementOffset = inst->getOperand(1);
                    if (!basePointer ||
                        !isTypeEqual(inst->getDataType(), basePointer->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("pointer offset result type"));
                    }
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        basePointer,
                        inst,
                        availableValues,
                        dominatorTree,
                        false));
                    SLANG_RETURN_ON_FAIL(_validateI32Value(
                        codeGenContext,
                        elementOffset,
                        inst,
                        availableValues,
                        dominatorTree,
                        capability));
                    availableValues.add(inst);
                }
                break;

            case kIROp_GetElementPtr:
                {
                    IRInst* basePointer = inst->getOperand(0);
                    IRInst* elementIndex = inst->getOperand(1);
                    IRArrayType* arrayType = nullptr;
                    auto basePointerType = basePointer ? _asSupportedDeviceArrayPointerType(
                                                             basePointer->getDataType(),
                                                             &arrayType)
                                                       : nullptr;
                    auto resultPointerType = _asSupportedDevicePointerType(inst->getDataType());
                    if (!basePointerType || !resultPointerType || !arrayType ||
                        basePointerType->getAddressSpace() !=
                            resultPointerType->getAddressSpace() ||
                        basePointerType->getAccessQualifier() !=
                            resultPointerType->getAccessQualifier() ||
                        !isTypeEqual(
                            arrayType->getElementType(),
                            resultPointerType->getValueType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("array element pointer relation"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        basePointer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateI32Value(
                        codeGenContext,
                        elementIndex,
                        inst,
                        availableValues,
                        dominatorTree,
                        capability));
                    availableValues.add(inst);
                }
                break;

            case kIROp_RWStructuredBufferGetElementPtr:
                {
                    IRInst* buffer = inst->getOperand(0);
                    IRInst* elementIndex = inst->getOperand(1);
                    if (!buffer || !_asSupportedRawRWStructuredBufferI32Type(buffer->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("raw RWStructuredBuffer signed i32 relation"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateI32Value(
                        codeGenContext,
                        elementIndex,
                        inst,
                        availableValues,
                        dominatorTree,
                        capability));
                    availableValues.add(inst);
                }
                break;

            case kIROp_Return:
                {
                    auto returnInst = cast<IRReturn>(inst);
                    if (returnInst != terminator || !returnInst->getVal())
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("return value"));
                    if (isEntryPoint)
                    {
                        if (returnInst->getVal()->getOp() != kIROp_VoidLit)
                            return _diagnoseUnsupportedIR(codeGenContext, toSlice("return value"));
                    }
                    else
                    {
                        if (!isTypeEqual(
                                returnInst->getVal()->getDataType(),
                                function->getResultType()))
                        {
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("helper return type"));
                        }
                        SLANG_RETURN_ON_FAIL(_validateI32Value(
                            codeGenContext,
                            returnInst->getVal(),
                            returnInst,
                            availableValues,
                            dominatorTree,
                            capability));
                        hasHelperReturn = true;
                    }
                }
                break;

            case kIROp_UnconditionalBranch:
                {
                    auto branch = cast<IRUnconditionalBranch>(inst);
                    if (branch != terminator)
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch position"));
                    SLANG_RETURN_ON_FAIL(_validateBranchArguments(
                        codeGenContext,
                        branch,
                        entryBlock,
                        functionBlocks,
                        availableValues,
                        dominatorTree,
                        capability));
                }
                break;

            case kIROp_Loop:
                {
                    auto loop = cast<IRLoop>(inst);
                    if (loop != terminator)
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("loop position"));
                    SLANG_RETURN_ON_FAIL(_validateBranchArguments(
                        codeGenContext,
                        loop,
                        entryBlock,
                        functionBlocks,
                        availableValues,
                        dominatorTree,
                        capability));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        loop->getBreakBlock(),
                        functionBlocks));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        loop->getContinueBlock(),
                        functionBlocks));
                }
                break;

            case kIROp_IfElse:
                {
                    auto ifElse = cast<IRIfElse>(inst);
                    if (ifElse != terminator)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch position"));
                    }
                    if (!ifElse->getCondition() ||
                        !_isBoolType(ifElse->getCondition()->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch condition"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        ifElse->getCondition(),
                        ifElse,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        ifElse->getTrueBlock(),
                        functionBlocks));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        ifElse->getFalseBlock(),
                        functionBlocks));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        ifElse->getAfterBlock(),
                        functionBlocks));
                    if (ifElse->getTrueBlock()->getFirstParam() ||
                        ifElse->getFalseBlock()->getFirstParam())
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch target parameter"));
                    }
                }
                break;

            default:
                SLANG_UNEXPECTED("NVVM validation reached an unclassified instruction");
            }
        }
    }

    if (!isEntryPoint && !hasHelperReturn)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("helper return"));

    // Every non-entry phi needs at least one actual CFG predecessor. Structural `IRLoop`
    // break/continue and `IRIfElse::afterBlock` operands are deliberately absent from this list.
    for (auto block : function->getBlocks())
    {
        if (block == entryBlock || !block->getFirstParam())
            continue;

        auto predecessors = block->getPredecessors();
        if (predecessors.isEmpty())
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block predecessor"));
        for (auto predecessor : predecessors)
        {
            auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
            if (!branch || branch->getTargetBlock() != block)
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("parameterized predecessor edge"));
            }
        }
    }
    return SLANG_OK;
}

using NVVMValueMap = Dictionary<IRInst*, SlangNVVMValueHandle_1>;

// Gets the one module-owned i32 type on demand so empty Slice 6 kernels keep their minimal graph.
SlangResult _getNVVMI32Type(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1& ioType)
{
    if (ioType)
        return SLANG_OK;
    return _requireBuilderOperation(
        codeGenContext,
        "signed i32 type",
        builder.getIntegerType(module, 32, ioType));
}

// Gets the provider array and AS1 pointer types for one canonical fixed-i32-array type.
SlangResult _getNVVMDeviceArrayPointerType(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle_1 module,
    IRArrayType* irArrayType,
    SlangNVVMTypeHandle_1& ioI32Type,
    Dictionary<IRArrayType*, SlangNVVMTypeHandle_1>& arrayTypeMap,
    Dictionary<IRArrayType*, SlangNVVMTypeHandle_1>& arrayPointerTypeMap,
    SlangNVVMTypeHandle_1& outPointerType)
{
    outPointerType = nullptr;
    uint32_t elementCount = 0;
    SLANG_RELEASE_ASSERT(_asSupportedI32ArrayType(irArrayType, &elementCount));
    SLANG_RETURN_ON_FAIL(_getNVVMI32Type(codeGenContext, builder, module, ioI32Type));

    SlangNVVMTypeHandle_1 arrayType = nullptr;
    if (auto mappedType = arrayTypeMap.tryGetValue(irArrayType))
    {
        arrayType = *mappedType;
    }
    else
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "fixed i32 array type",
            builder.getArrayType(module, ioI32Type, elementCount, arrayType)));
        arrayTypeMap[irArrayType] = arrayType;
    }

    if (auto mappedType = arrayPointerTypeMap.tryGetValue(irArrayType))
    {
        outPointerType = *mappedType;
        return SLANG_OK;
    }
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "device fixed i32 array pointer type",
        builder
            .getPointerType(module, arrayType, SLANG_NVVM_ADDRESS_SPACE_GLOBAL, outPointerType)));
    arrayPointerTypeMap[irArrayType] = outPointerType;
    return SLANG_OK;
}

// Returns an already-lowered SSA value or materializes the exact preflighted i32 literal.
SlangResult _getLoweredNVVMValue(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle_1 module,
    IRInst* irValue,
    NVVMValueMap& valueMap,
    SlangNVVMTypeHandle_1& ioI32Type,
    SlangNVVMValueHandle_1& outValue)
{
    outValue = nullptr;
    if (auto mappedValue = valueMap.tryGetValue(irValue))
    {
        outValue = *mappedValue;
        return SLANG_OK;
    }

    auto intLit = _asExecutableI32Constant(irValue);
    SLANG_RELEASE_ASSERT(intLit);
    SLANG_RETURN_ON_FAIL(_getNVVMI32Type(codeGenContext, builder, module, ioI32Type));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "signed i32 constant",
        builder.getIntegerConstant(module, ioI32Type, int64_t(intLit->getValue()), outValue)));
    valueMap[irValue] = outValue;
    return SLANG_OK;
}

} // namespace

SlangResult validateNVVMSupportedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    NVVMIRCapability& outCapability)
{
    outCapability = NVVMIRCapability::Minimal;
    if (!linkedIR.module || linkedIR.entryPoints.getCount() != 1)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point count"));

    IRFunc* entryPoint = linkedIR.entryPoints[0];
    if (!entryPoint || entryPoint->getParent() != linkedIR.module->getModuleInst() ||
        !entryPoint->isDefinition())
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point definition"));
    }

    auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
    if (!entryPointDecoration)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point decoration"));
    if (entryPointDecoration->getProfile().getStage() != Stage::Compute)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point stage"));
    if (!entryPointDecoration->getName()->getStringSlice().getLength())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point name"));
    if (!as<IRVoidType>(entryPoint->getResultType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point result type"));

    List<IRFunc*> functions;
    HashSet<IRFunc*> functionSet;
    SLANG_RETURN_ON_FAIL(
        _collectNVVMFunctions(codeGenContext, linkedIR, entryPoint, functions, functionSet));
    SLANG_RETURN_ON_FAIL(_validateNVVMFunctionNames(codeGenContext, entryPoint, functions));
    SLANG_RETURN_ON_FAIL(_validateNVVMFunctionUses(codeGenContext, functions));

    if (functions.getCount() > 1)
        _requireCapability(outCapability, NVVMIRCapability::ScalarFunctions);
    for (auto function : functions)
    {
        SLANG_RETURN_ON_FAIL(_validateNVVMFunction(
            codeGenContext,
            entryPoint,
            function,
            functionSet,
            outCapability));
    }

    // Scalar CUDA launch parameters and executable scalar operations are meaningful only for a
    // CUDA kernel. Preserve Slice 6's conventional zero-parameter empty compute entry point, but
    // do not invent a raw CUDA launch ABI for an ordinary shader entry point.
    if (outCapability != NVVMIRCapability::Minimal &&
        !entryPoint->findDecoration<IRCudaKernelDecoration>())
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("CUDA kernel decoration"));
    }

    // Linking can retain module-scope types, layouts, capabilities, and constants needed to spell
    // the reachable functions. IRStructKey is also layout-only identity retained for raw CUDA
    // parameter layouts. Reject every other semantic global so this emitter cannot silently drop a
    // function, parameter, initializer, or storage object.
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        if (auto globalFunction = as<IRFunc>(globalInst))
        {
            if (functionSet.contains(globalFunction))
                continue;
            return _diagnoseUnsupportedIR(
                codeGenContext,
                UnownedStringSlice(getIROpInfo(globalInst->getOp()).name));
        }
        if (as<IRDecoration>(globalInst) || as<IRConstant>(globalInst) ||
            as<IRStructKey>(globalInst) || getIROpInfo(globalInst->getOp()).isHoistable())
        {
            continue;
        }
        return _diagnoseUnsupportedIR(
            codeGenContext,
            UnownedStringSlice(getIROpInfo(globalInst->getOp()).name));
    }

    return SLANG_OK;
}
SlangResult emitNVVMIRFromLinkedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    const NVVMIRBuilder& builder,
    ComPtr<IArtifact>& outArtifact)
{
    outArtifact.setNull();
    SLANG_RELEASE_ASSERT(linkedIR.entryPoints.getCount() == 1);

    IRFunc* entryPoint = linkedIR.entryPoints[0];
    auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
    SLANG_RELEASE_ASSERT(entryPointDecoration);

    // Reuse preflight's exact closure walk so the accepted and emitted function sets cannot drift.
    List<IRFunc*> functions;
    HashSet<IRFunc*> functionSet;
    SLANG_RETURN_ON_FAIL(
        _collectNVVMFunctions(codeGenContext, linkedIR, entryPoint, functions, functionSet));

    ScopedNVVMModule moduleScope;
    moduleScope.builder = &builder;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "module creation",
        builder.createModule(toSlice("slang-direct-nvvm"), moduleScope.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "void type",
        builder.getVoidType(moduleScope.module, voidType)));

    SlangNVVMTypeHandle_1 i32Type = nullptr;
    SlangNVVMTypeHandle_1 deviceI32PointerType = nullptr;
    SlangNVVMTypeHandle_1 rawRWStructuredBufferI32Type = nullptr;
    Dictionary<IRArrayType*, SlangNVVMTypeHandle_1> arrayTypeMap;
    Dictionary<IRArrayType*, SlangNVVMTypeHandle_1> arrayPointerTypeMap;
    Dictionary<IRFunc*, SlangNVVMValueHandle_1> functionMap;
    NVVMValueMap valueMap;
    Dictionary<IRBlock*, SlangNVVMBlockHandle_1> blockMap;

    // Every function is declared before any body is emitted. A call can therefore target a helper
    // that appears later in linked-IR order without turning physical order into a legality rule.
    for (auto function : functions)
    {
        SlangNVVMTypeHandle_1 resultType = voidType;
        if (function != entryPoint)
        {
            SLANG_RETURN_ON_FAIL(
                _getNVVMI32Type(codeGenContext, builder, moduleScope.module, i32Type));
            resultType = i32Type;
        }

        List<SlangNVVMTypeHandle_1> parameterTypes;
        for (auto param : function->getParams())
        {
            if (_isI32Type(param->getDataType()))
            {
                SLANG_RETURN_ON_FAIL(
                    _getNVVMI32Type(codeGenContext, builder, moduleScope.module, i32Type));
                parameterTypes.add(i32Type);
                continue;
            }

            SLANG_RELEASE_ASSERT(function == entryPoint);
            SLANG_RETURN_ON_FAIL(
                _getNVVMI32Type(codeGenContext, builder, moduleScope.module, i32Type));
            if (_asSupportedDevicePointerType(param->getDataType()))
            {
                if (!deviceI32PointerType)
                {
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "device i32 pointer type",
                        builder.getPointerType(
                            moduleScope.module,
                            i32Type,
                            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
                            deviceI32PointerType)));
                }
                parameterTypes.add(deviceI32PointerType);
                continue;
            }

            if (_asSupportedRawRWStructuredBufferI32Type(param->getDataType()))
            {
                if (!rawRWStructuredBufferI32Type)
                {
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "raw RWStructuredBuffer signed i32 type",
                        builder.getRawRWStructuredBufferI32Type(
                            moduleScope.module,
                            rawRWStructuredBufferI32Type)));
                }
                parameterTypes.add(rawRWStructuredBufferI32Type);
                continue;
            }

            IRArrayType* irArrayType = nullptr;
            SLANG_RELEASE_ASSERT(
                _asSupportedDeviceArrayPointerType(param->getDataType(), &irArrayType));
            SlangNVVMTypeHandle_1 arrayPointerType = nullptr;
            SLANG_RETURN_ON_FAIL(_getNVVMDeviceArrayPointerType(
                codeGenContext,
                builder,
                moduleScope.module,
                irArrayType,
                i32Type,
                arrayTypeMap,
                arrayPointerTypeMap,
                arrayPointerType));
            parameterTypes.add(arrayPointerType);
        }

        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "function type",
            builder.getFunctionType(
                moduleScope.module,
                resultType,
                parameterTypes.getCount() ? parameterTypes.getBuffer() : nullptr,
                size_t(parameterTypes.getCount()),
                functionType)));

        SlangNVVMValueHandle_1 loweredFunction = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "function declaration",
            builder.declareFunction(
                moduleScope.module,
                functionType,
                _getNVVMFunctionName(function, entryPoint),
                loweredFunction)));
        functionMap[function] = loweredFunction;
    }

    for (auto function : functions)
    {
        size_t parameterIndex = 0;
        for (auto param : function->getParams())
        {
            SlangNVVMValueHandle_1 parameter = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "function parameter",
                builder.getFunctionParameter(
                    moduleScope.module,
                    functionMap.getValue(function),
                    parameterIndex,
                    parameter)));
            valueMap[param] = parameter;
            ++parameterIndex;
        }
    }

    for (auto function : functions)
    {
        // LLVM branches can refer to blocks declared later, so create this function's complete CFG
        // before emitting any body instruction.
        Index blockIndex = 0;
        for (auto block : function->getBlocks())
        {
            StringBuilder nameBuilder;
            if (blockIndex == 0)
                nameBuilder << "entry";
            else
                nameBuilder << "block" << blockIndex;
            String blockName = nameBuilder.produceString();

            SlangNVVMBlockHandle_1 loweredBlock = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "basic-block creation",
                builder.createBlock(
                    moduleScope.module,
                    functionMap.getValue(function),
                    blockName.getUnownedSlice(),
                    loweredBlock)));
            blockMap[block] = loweredBlock;
            ++blockIndex;
        }

        // Consider the loop header header(i, sum). Its phis must exist before the compare and body
        // use them, while their backedge values are not emitted until later blocks. Create every
        // phi placeholder now; incoming pairs are attached after all bodies and terminators exist.
        IRBlock* entryBlock = function->getFirstBlock();
        for (auto block : function->getBlocks())
        {
            if (block == entryBlock)
                continue;

            for (auto param : block->getParams())
            {
                SLANG_RETURN_ON_FAIL(
                    _getNVVMI32Type(codeGenContext, builder, moduleScope.module, i32Type));
                SlangNVVMValueHandle_1 loweredPhi = nullptr;
                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                    codeGenContext,
                    "signed i32 phi",
                    builder.emitIntegerPhi(
                        moduleScope.module,
                        blockMap.getValue(block),
                        i32Type,
                        loweredPhi)));
                valueMap[param] = loweredPhi;
            }
        }

        RefPtr<IRDominatorTree> dominatorTree = computeDominatorTree(function);
        List<IRBlock*> bodyOrder = _getNVVMBodyOrder(function, dominatorTree);
        for (auto block : bodyOrder)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "insertion-block selection",
                builder.setInsertBlock(moduleScope.module, blockMap.getValue(block))));

            for (auto inst : block->getOrdinaryInsts())
            {
                switch (inst->getOp())
                {
                case kIROp_Load:
                    {
                        auto load = cast<IRLoad>(inst);
                        SlangNVVMValueHandle_1 loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            load->getPtr(),
                            valueMap,
                            i32Type,
                            loweredPointer));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 load",
                            builder.emitLoad(
                                moduleScope.module,
                                loweredPointer,
                                kNVVMI32Alignment,
                                loweredValue)));
                        valueMap[load] = loweredValue;
                    }
                    break;

                case kIROp_Store:
                    {
                        auto store = cast<IRStore>(inst);
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store->getVal(),
                            valueMap,
                            i32Type,
                            loweredValue));
                        SlangNVVMValueHandle_1 loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store->getPtr(),
                            valueMap,
                            i32Type,
                            loweredPointer));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 store",
                            builder.emitStore(
                                moduleScope.module,
                                loweredValue,
                                loweredPointer,
                                kNVVMI32Alignment)));
                    }
                    break;

                case kIROp_Add:
                case kIROp_Sub:
                    {
                        const SlangNVVMIntegerBinaryOp_2 operation =
                            inst->getOp() == kIROp_Add ? SLANG_NVVM_INTEGER_BINARY_OP_ADD
                                                       : SLANG_NVVM_INTEGER_BINARY_OP_SUB;
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            inst->getOp() == kIROp_Add ? "signed i32 addition"
                                                       : "signed i32 subtraction",
                            builder.emitIntegerBinary(
                                moduleScope.module,
                                operation,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Mul:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 multiplication",
                            builder.emitIntegerMultiply(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_BitAnd:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 bitwise AND",
                            builder.emitIntegerBitAnd(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_BitOr:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 bitwise OR",
                            builder.emitIntegerBitOr(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_BitXor:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 bitwise XOR",
                            builder.emitIntegerBitXor(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_BitNot:
                    {
                        SlangNVVMValueHandle_1 loweredOperand = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredOperand));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 bitwise NOT",
                            builder.emitIntegerBitNot(
                                moduleScope.module,
                                loweredOperand,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Neg:
                    {
                        SlangNVVMValueHandle_1 loweredOperand = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredOperand));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 arithmetic negation",
                            builder.emitIntegerNegate(
                                moduleScope.module,
                                loweredOperand,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_AtomicAdd:
                    {
                        SlangNVVMValueHandle_1 loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredPointer));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredValue));
                        SlangNVVMValueHandle_1 loweredOriginalValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "relaxed global signed i32 atomic add",
                            builder.emitRelaxedGlobalI32AtomicAdd(
                                moduleScope.module,
                                loweredPointer,
                                loweredValue,
                                loweredOriginalValue)));
                        valueMap[inst] = loweredOriginalValue;
                    }
                    break;

                case kIROp_Less:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 less-than comparison",
                            builder.emitIntegerSignedLessThan(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Eql:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 equality comparison",
                            builder.emitIntegerEqual(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Neq:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 inequality comparison",
                            builder.emitIntegerNotEqual(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Greater:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 greater-than comparison",
                            builder.emitIntegerSignedGreaterThan(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Leq:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 less-than-or-equal comparison",
                            builder.emitIntegerSignedLessEqual(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Geq:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 greater-than-or-equal comparison",
                            builder.emitIntegerSignedGreaterEqual(
                                moduleScope.module,
                                loweredLeft,
                                loweredRight,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Call:
                    {
                        auto call = cast<IRCall>(inst);
                        auto callee = cast<IRFunc>(call->getOperand(0));
                        List<SlangNVVMValueHandle_1> loweredArguments;
                        for (UInt argumentIndex = 0; argumentIndex < call->getArgCount();
                             ++argumentIndex)
                        {
                            SlangNVVMValueHandle_1 loweredArgument = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                call->getArg(argumentIndex),
                                valueMap,
                                i32Type,
                                loweredArgument));
                            loweredArguments.add(loweredArgument);
                        }

                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 call",
                            builder.emitIntegerCall(
                                moduleScope.module,
                                functionMap.getValue(callee),
                                loweredArguments.getCount() ? loweredArguments.getBuffer()
                                                            : nullptr,
                                size_t(loweredArguments.getCount()),
                                loweredValue)));
                        valueMap[call] = loweredValue;
                    }
                    break;

                case kIROp_GetOffsetPtr:
                    {
                        SlangNVVMValueHandle_1 loweredBasePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredBasePointer));
                        SlangNVVMValueHandle_1 loweredElementOffset = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredElementOffset));
                        SlangNVVMValueHandle_1 loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "device i32 pointer offset",
                            builder.emitPointerOffset(
                                moduleScope.module,
                                loweredBasePointer,
                                loweredElementOffset,
                                loweredPointer)));
                        valueMap[inst] = loweredPointer;
                    }
                    break;

                case kIROp_GetElementPtr:
                    {
                        SlangNVVMValueHandle_1 loweredBasePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredBasePointer));
                        SlangNVVMValueHandle_1 loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredElementIndex));
                        SlangNVVMValueHandle_1 loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "device i32 array element pointer",
                            builder.emitArrayElementPointer(
                                moduleScope.module,
                                loweredBasePointer,
                                loweredElementIndex,
                                loweredPointer)));
                        valueMap[inst] = loweredPointer;
                    }
                    break;

                case kIROp_RWStructuredBufferGetElementPtr:
                    {
                        SlangNVVMValueHandle_1 loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            i32Type,
                            loweredBuffer));
                        SlangNVVMValueHandle_1 loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            i32Type,
                            loweredElementIndex));
                        SlangNVVMValueHandle_1 loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw RWStructuredBuffer signed i32 element pointer",
                            builder.emitRawRWStructuredBufferI32ElementPointer(
                                moduleScope.module,
                                loweredBuffer,
                                loweredElementIndex,
                                loweredPointer)));
                        valueMap[inst] = loweredPointer;
                    }
                    break;

                case kIROp_Return:
                    if (function == entryPoint)
                    {
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "void return",
                            builder.emitReturnVoid(moduleScope.module)));
                    }
                    else
                    {
                        auto returnInst = cast<IRReturn>(inst);
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            returnInst->getVal(),
                            valueMap,
                            i32Type,
                            loweredValue));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 return",
                            builder.emitIntegerReturn(moduleScope.module, loweredValue)));
                    }
                    break;

                case kIROp_UnconditionalBranch:
                case kIROp_Loop:
                    {
                        auto branch = cast<IRUnconditionalBranch>(inst);
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            inst->getOp() == kIROp_Loop ? "loop entry branch"
                                                        : "unconditional branch",
                            builder.emitBranch(
                                moduleScope.module,
                                blockMap.getValue(branch->getTargetBlock()))));
                    }
                    break;

                case kIROp_IfElse:
                    {
                        auto ifElse = cast<IRIfElse>(inst);
                        SlangNVVMValueHandle_1 loweredCondition = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            ifElse->getCondition(),
                            valueMap,
                            i32Type,
                            loweredCondition));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "conditional branch",
                            builder.emitConditionalBranch(
                                moduleScope.module,
                                loweredCondition,
                                blockMap.getValue(ifElse->getTrueBlock()),
                                blockMap.getValue(ifElse->getFalseBlock()))));
                    }
                    break;

                default:
                    SLANG_UNEXPECTED("NVVM emission received IR that was not preflighted");
                }
            }
        }

        // Slang block parameters are the phi source of truth: argument N on each actual predecessor
        // edge feeds parameter N. At this point even loop backedge instructions exist, so every
        // pair can be attached without reconstructing a local variable or searching an operand
        // graph.
        for (auto block : function->getBlocks())
        {
            if (block == entryBlock || !block->getFirstParam())
                continue;

            for (auto predecessor : block->getPredecessors())
            {
                auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
                SLANG_RELEASE_ASSERT(branch && branch->getTargetBlock() == block);

                UInt phiParameterIndex = 0;
                for (auto param : block->getParams())
                {
                    SlangNVVMValueHandle_1 loweredArgument = nullptr;
                    SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                        codeGenContext,
                        builder,
                        moduleScope.module,
                        branch->getArg(phiParameterIndex),
                        valueMap,
                        i32Type,
                        loweredArgument));
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "signed i32 phi incoming value",
                        builder.addIntegerPhiIncoming(
                            moduleScope.module,
                            valueMap.getValue(param),
                            loweredArgument,
                            blockMap.getValue(predecessor))));
                    ++phiParameterIndex;
                }
            }
        }
    }

    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "kernel annotation",
        builder.markFunctionAsKernel(moduleScope.module, functionMap.getValue(entryPoint))));

    if (!builder.supportsSerializationDiagnostics())
    {
        return _requireBuilderOperation(
            codeGenContext,
            "verified LLVM IR serialization",
            SLANG_E_NOT_AVAILABLE);
    }

    const bool useNVVMIR20Assembly = builder.supportsNVVMIR20Assembly();
    const SlangNVVMSerializationFormat_1 serializationFormat =
        useNVVMIR20Assembly ? SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY
                            : SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE;
    const char* serializationOperation = useNVVMIR20Assembly
                                             ? "verified NVVM IR 2.0 assembly serialization"
                                             : "verified LLVM bitcode serialization";

    ComPtr<ISlangBlob> serializedIR;
    String verifierDiagnostics;
    SlangResult serializationResult = builder.serializeModule(
        moduleScope.module,
        serializationFormat,
        serializedIR,
        verifierDiagnostics);
    if (SLANG_FAILED(serializationResult))
    {
        _requireBuilderOperation(codeGenContext, serializationOperation, serializationResult);
        if (verifierDiagnostics.getLength())
        {
            codeGenContext->getSink()->diagnoseRaw(
                Severity::Note,
                verifierDiagnostics.getUnownedSlice());
        }
        return serializationResult;
    }
    if (verifierDiagnostics.getLength())
    {
        codeGenContext->getSink()->diagnoseRaw(
            Severity::Note,
            verifierDiagnostics.getUnownedSlice());
    }
    if (!serializedIR || !serializedIR->getBufferSize())
    {
        return _requireBuilderOperation(codeGenContext, serializationOperation, SLANG_FAIL);
    }

    const ArtifactKind artifactKind =
        useNVVMIR20Assembly ? ArtifactKind::Assembly : ArtifactKind::ObjectCode;
    auto artifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(artifactKind, ArtifactPayload::LLVMIR, ArtifactStyle::Kernel));
    artifact->addRepresentationUnknown(serializedIR);
    ArtifactUtil::addAssociated(artifact, linkedIR.metadata);
    outArtifact = artifact;
    return SLANG_OK;
}

} // namespace Slang
