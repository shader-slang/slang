#include "slang-emit-nvvm.h"

#include "compiler-core/slang-artifact-impl.h"
#include "compiler-core/slang-artifact-util.h"
#include "core/slang-dictionary.h"
#include "slang-code-gen.h"
#include "slang-diagnostics.h"
#include "slang-emit-nvvm-type-lowering.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
namespace
{

// CUDA `int` and `float` both use four-byte natural alignment in the accepted direct scalar ABI.
static const uint32_t kNVVMScalar32Alignment = 4;
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

// Returns an executable signed-i32 literal, excluding layout and other module constants.
IRIntLit* _asExecutableI32Constant(IRInst* value)
{
    auto intLit = as<IRIntLit>(value);
    if (!intLit || !isNVVMSignedI32Type(intLit->getDataType()))
        return nullptr;

    const IRIntegerValue intValue = intLit->getValue();
    return intValue >= kNVVMI32Min && intValue <= kNVVMI32Max ? intLit : nullptr;
}

// Records one independent provider semantic required by the accepted linked IR.
void _requireFeature(NVVMIRFeatureSet& features, SlangNVVMBuilderFeature_3 requiredFeature)
{
    features.words[requiredFeature / 64u] |= uint64_t(1) << (requiredFeature % 64u);
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
    NVVMIRFeatureSet& features)
{
    if (!value || !isNVVMSignedI32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 value"));

    if (_asExecutableI32Constant(value))
    {
        _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_SSA);
        return SLANG_OK;
    }

    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks that an executable operand is an available canonical float32 value.
SlangResult _validateFloat32Value(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (!value || !isNVVMFloat32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("float32 value"));
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks an available device pointer and enforces the source access qualifier for stores.
SlangResult _validatePointerValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree,
    bool requireWriteAccess,
    IRType* expectedPointeeType)
{
    auto ptrType = value ? asNVVMSupportedDevicePointerType(value->getDataType()) : nullptr;
    auto float32PtrType =
        value ? asNVVMSupportedDeviceFloat32PointerType(value->getDataType()) : nullptr;
    auto resourceElementPtrType =
        value ? asNVVMSupportedRWStructuredBufferI32ElementPointerType(value->getDataType())
              : nullptr;
    IRPtrTypeBase* devicePtrType = ptrType ? ptrType : float32PtrType;
    if (!devicePtrType &&
        (!resourceElementPtrType || value->getOp() != kIROp_RWStructuredBufferGetElementPtr))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("device scalar pointer"));
    IRType* actualPointeeType =
        devicePtrType ? devicePtrType->getValueType() : resourceElementPtrType->getValueType();
    if (!expectedPointeeType || !isTypeEqual(actualPointeeType, expectedPointeeType))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("device pointer pointee type"));
    if (resourceElementPtrType && consumer->getOp() != kIROp_Store)
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            toSlice("raw RWStructuredBuffer signed i32 store consumer"));
    }
    if (requireWriteAccess && devicePtrType &&
        devicePtrType->getAccessQualifier() != AccessQualifier::ReadWrite)
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
    NVVMIRFeatureSet& features)
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
            features));
    }

    if (argumentCount)
        _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_SSA);
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
    if (!isNVVMSignedI32Type(helper->getResultType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("helper function result type"));
    for (UInt parameterIndex = 0; parameterIndex < helper->getParamCount(); ++parameterIndex)
    {
        if (!isNVVMSignedI32Type(helper->getParamType(parameterIndex)))
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
    NVVMIRFeatureSet& features)
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
        _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW);

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
            isEntryPoint ? asNVVMSupportedDeviceArrayPointerType(param->getDataType()) : nullptr;
        auto rawRWStructuredBufferType =
            isEntryPoint ? asNVVMSupportedRawRWStructuredBufferI32Type(param->getDataType())
                         : nullptr;
        const bool usesFloat32 =
            isEntryPoint && (isNVVMFloat32Type(param->getDataType()) ||
                             asNVVMSupportedDeviceFloat32PointerType(param->getDataType()));
        const bool isSupportedType = isEntryPoint
                                         ? isNVVMSupportedParameterType(param->getDataType())
                                         : isNVVMSignedI32Type(param->getDataType());
        if (actualParamCount >= function->getParamCount() || !isSupportedType ||
            !isTypeEqual(param->getDataType(), function->getParamType(actualParamCount)))
        {
            return _diagnoseUnsupportedIR(
                codeGenContext,
                isEntryPoint ? toSlice("entry-point parameter")
                             : toSlice("helper function parameter"));
        }
        if (arrayPointerType)
            _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_ARRAY_ADDRESSING);
        if (rawRWStructuredBufferType)
            _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_RAW_RW_STRUCTURED_BUFFER_I32);
        if (usesFloat32)
            _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD);
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
        _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY);

    // Register every accepted block parameter before checking uses because emission creates all
    // phi placeholders before any body. Ordinary values join this set in the second pass, in the
    // same order in which their LLVM instructions will be emitted.
    for (auto block : function->getBlocks())
    {
        if (block != entryBlock)
        {
            for (auto param : block->getParams())
            {
                if (!isNVVMSignedI32Type(param->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block parameter"));
                availableValues.add(param);
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_SSA);
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
                if (isNVVMFloat32Type(inst->getDataType()))
                    _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD);
                else if (!isNVVMSignedI32Type(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("load result type"));
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY);
                break;

            case kIROp_Store:
                if (inst->getOperandCount() != 2 || !inst->getOperand(0))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("store"));
                if (isNVVMFloat32Type(inst->getOperand(0)->getDataType()))
                    _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD);
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY);
                break;

            case kIROp_Add:
            case kIROp_Sub:
                if (inst->getOperandCount() == 2 && isNVVMFloat32Type(inst->getDataType()))
                {
                    _requireFeature(
                        features,
                        inst->getOp() == kIROp_Add
                            ? SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD
                            : SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT);
                    break;
                }
                if (inst->getOperandCount() != 2 || !isNVVMSignedI32Type(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 arithmetic"));
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW);
                break;

            case kIROp_Mul:
                if (inst->getOperandCount() != 2 || !isNVVMSignedI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 multiplication"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY);
                break;

            case kIROp_BitAnd:
                if (inst->getOperandCount() != 2 || !isNVVMSignedI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 bitwise AND"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_AND);
                break;

            case kIROp_BitOr:
                if (inst->getOperandCount() != 2 || !isNVVMSignedI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 bitwise OR"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_OR);
                break;

            case kIROp_BitXor:
                if (inst->getOperandCount() != 2 || !isNVVMSignedI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 bitwise XOR"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_XOR);
                break;

            case kIROp_BitNot:
                if (inst->getOperandCount() != 1 || !isNVVMSignedI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 bitwise NOT"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_NOT);
                break;

            case kIROp_Neg:
                if (inst->getOperandCount() != 1 || !isNVVMSignedI32Type(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 arithmetic negation"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NEGATE);
                break;

            case kIROp_AtomicAdd:
                {
                    if (inst->getOperandCount() != 3 || !isNVVMSignedI32Type(inst->getDataType()))
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
                if (inst->getOperandCount() != 2 || !isNVVMBoolType(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 comparison"));
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW);
                break;

            case kIROp_Eql:
                if (inst->getOperandCount() != 2 || !isNVVMBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 equality"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_EQUAL);
                break;

            case kIROp_Neq:
                if (inst->getOperandCount() != 2 || !isNVVMBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 inequality"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NOT_EQUAL);
                break;

            case kIROp_Greater:
                if (inst->getOperandCount() != 2 || !isNVVMBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 greater-than"));
                }
                _requireFeature(
                    features,
                    SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_THAN);
                break;

            case kIROp_Leq:
                if (inst->getOperandCount() != 2 || !isNVVMBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 less-than-or-equal"));
                }
                _requireFeature(
                    features,
                    SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_LESS_EQUAL);
                break;

            case kIROp_Geq:
                if (inst->getOperandCount() != 2 || !isNVVMBoolType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("signed i32 greater-than-or-equal"));
                }
                _requireFeature(
                    features,
                    SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_EQUAL);
                break;

            case kIROp_Call:
                if (!inst->getOperandCount() || !isNVVMSignedI32Type(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 call"));
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FUNCTIONS);
                break;

            case kIROp_GetOffsetPtr:
                if (inst->getOperandCount() != 2 ||
                    !asNVVMSupportedDevicePointerType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("device i32 pointer offset"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_POINTER_ARITHMETIC);
                break;

            case kIROp_GetElementPtr:
                if (inst->getOperandCount() != 2 ||
                    !asNVVMSupportedDevicePointerType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("device i32 array element pointer"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_ARRAY_ADDRESSING);
                break;

            case kIROp_RWStructuredBufferGetElementPtr:
                if (inst->getOperandCount() != 2 ||
                    !asNVVMSupportedRWStructuredBufferI32ElementPointerType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("raw RWStructuredBuffer signed i32 element pointer"));
                }
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_RAW_RW_STRUCTURED_BUFFER_I32);
                break;

            case kIROp_Return:
                break;

            case kIROp_UnconditionalBranch:
            case kIROp_Loop:
            case kIROp_IfElse:
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW);
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
                        false,
                        load->getDataType()));
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
                        true,
                        store->getVal()->getDataType()));
                    if (isNVVMFloat32Type(store->getVal()->getDataType()))
                    {
                        SLANG_RETURN_ON_FAIL(_validateFloat32Value(
                            codeGenContext,
                            store->getVal(),
                            store,
                            availableValues,
                            dominatorTree));
                    }
                    else
                    {
                        SLANG_RETURN_ON_FAIL(_validateI32Value(
                            codeGenContext,
                            store->getVal(),
                            store,
                            availableValues,
                            dominatorTree,
                            features));
                    }
                }
                break;

            case kIROp_Add:
            case kIROp_Sub:
                if (isNVVMFloat32Type(inst->getDataType()))
                {
                    SLANG_RETURN_ON_FAIL(_validateFloat32Value(
                        codeGenContext,
                        inst->getOperand(0),
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateFloat32Value(
                        codeGenContext,
                        inst->getOperand(1),
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                    break;
                }
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
                    features));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree,
                    features));
                availableValues.add(inst);
                break;

            case kIROp_BitNot:
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree,
                    features));
                availableValues.add(inst);
                break;

            case kIROp_Neg:
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree,
                    features));
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
                    true,
                    inst->getDataType()));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree,
                    features));
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_RELAXED_GLOBAL_I32_ATOMIC_ADD);
                _requireFeature(features, SLANG_NVVM_BUILDER_FEATURE_NVVM_IR_2_0_ASSEMBLY);
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
                            features));
                    }
                    availableValues.add(call);
                }
                break;

            case kIROp_GetOffsetPtr:
                {
                    IRInst* basePointer = inst->getOperand(0);
                    IRInst* elementOffset = inst->getOperand(1);
                    auto basePointerType =
                        basePointer ? asNVVMSupportedDevicePointerType(basePointer->getDataType())
                                    : nullptr;
                    if (!basePointerType ||
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
                        false,
                        basePointerType->getValueType()));
                    SLANG_RETURN_ON_FAIL(_validateI32Value(
                        codeGenContext,
                        elementOffset,
                        inst,
                        availableValues,
                        dominatorTree,
                        features));
                    availableValues.add(inst);
                }
                break;

            case kIROp_GetElementPtr:
                {
                    IRInst* basePointer = inst->getOperand(0);
                    IRInst* elementIndex = inst->getOperand(1);
                    IRArrayType* arrayType = nullptr;
                    auto basePointerType = basePointer ? asNVVMSupportedDeviceArrayPointerType(
                                                             basePointer->getDataType(),
                                                             &arrayType)
                                                       : nullptr;
                    auto resultPointerType = asNVVMSupportedDevicePointerType(inst->getDataType());
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
                        features));
                    availableValues.add(inst);
                }
                break;

            case kIROp_RWStructuredBufferGetElementPtr:
                {
                    IRInst* buffer = inst->getOperand(0);
                    IRInst* elementIndex = inst->getOperand(1);
                    if (!buffer ||
                        !asNVVMSupportedRawRWStructuredBufferI32Type(buffer->getDataType()))
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
                        features));
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
                            features));
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
                        features));
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
                        features));
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
                        !isNVVMBoolType(ifElse->getCondition()->getDataType()))
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

// Returns an already-lowered SSA value or materializes the exact preflighted i32 literal.
SlangResult _getLoweredNVVMValue(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle_1 module,
    IRInst* irValue,
    NVVMValueMap& valueMap,
    NVVMTypeLoweringContext& typeContext,
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
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SLANG_RETURN_ON_FAIL(
        typeContext.lowerType(intLit->getDataType(), NVVMTypeUse::Value, integerType));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "signed i32 constant",
        builder.getIntegerConstant(module, integerType, int64_t(intLit->getValue()), outValue)));
    valueMap[irValue] = outValue;
    return SLANG_OK;
}

} // namespace

SlangResult validateNVVMSupportedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    NVVMIRFeatureSet& outFeatures)
{
    outFeatures = {};
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
        _requireFeature(outFeatures, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FUNCTIONS);
    for (auto function : functions)
    {
        SLANG_RETURN_ON_FAIL(
            _validateNVVMFunction(codeGenContext, entryPoint, function, functionSet, outFeatures));
    }

    // Scalar CUDA launch parameters and executable scalar operations are meaningful only for a
    // CUDA kernel. Preserve Slice 6's conventional zero-parameter empty compute entry point, but
    // do not invent a raw CUDA launch ABI for an ordinary shader entry point.
    bool hasRequiredFeatures = false;
    for (uint32_t i = 0; i < SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT_3; ++i)
        hasRequiredFeatures = hasRequiredFeatures || outFeatures.words[i] != 0;
    if (hasRequiredFeatures && !entryPoint->findDecoration<IRCudaKernelDecoration>())
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

    NVVMTypeLoweringContext typeContext(codeGenContext, builder, moduleScope.module);
    Dictionary<IRFunc*, SlangNVVMValueHandle_1> functionMap;
    NVVMValueMap valueMap;
    Dictionary<IRBlock*, SlangNVVMBlockHandle_1> blockMap;

    // Every function is declared before any body is emitted. A call can therefore target a helper
    // that appears later in linked-IR order without turning physical order into a legality rule.
    for (auto function : functions)
    {
        const bool isEntryPoint = function == entryPoint;
        SlangNVVMTypeHandle_1 resultType = nullptr;
        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
            function->getResultType(),
            isEntryPoint ? NVVMTypeUse::EntryPointResult : NVVMTypeUse::HelperResult,
            resultType));

        List<SlangNVVMTypeHandle_1> parameterTypes;
        for (auto param : function->getParams())
        {
            SlangNVVMTypeHandle_1 parameterType = nullptr;
            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                param->getDataType(),
                isEntryPoint ? NVVMTypeUse::EntryPointParameter : NVVMTypeUse::HelperParameter,
                parameterType));
            parameterTypes.add(parameterType);
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
                SlangNVVMTypeHandle_1 parameterType = nullptr;
                SLANG_RETURN_ON_FAIL(
                    typeContext.lowerType(param->getDataType(), NVVMTypeUse::Value, parameterType));
                SlangNVVMValueHandle_1 loweredPhi = nullptr;
                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                    codeGenContext,
                    "signed i32 phi",
                    builder.emitIntegerPhi(
                        moduleScope.module,
                        blockMap.getValue(block),
                        parameterType,
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
                            typeContext,
                            loweredPointer));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            isNVVMFloat32Type(load->getDataType()) ? "float32 load"
                                                                   : "signed i32 load",
                            builder.emitLoad(
                                moduleScope.module,
                                loweredPointer,
                                kNVVMScalar32Alignment,
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
                            typeContext,
                            loweredValue));
                        SlangNVVMValueHandle_1 loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store->getPtr(),
                            valueMap,
                            typeContext,
                            loweredPointer));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            isNVVMFloat32Type(store->getVal()->getDataType()) ? "float32 store"
                                                                              : "signed i32 store",
                            builder.emitStore(
                                moduleScope.module,
                                loweredValue,
                                loweredPointer,
                                kNVVMScalar32Alignment)));
                    }
                    break;

                case kIROp_Add:
                case kIROp_Sub:
                    {
                        SlangNVVMValueHandle_1 loweredLeft = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        if (isNVVMFloat32Type(inst->getDataType()))
                        {
                            const bool isAdd = inst->getOp() == kIROp_Add;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                isAdd ? "float32 addition" : "float32 subtraction",
                                builder.emitFloatingBinary(
                                    moduleScope.module,
                                    isAdd ? SLANG_NVVM_FLOATING_BINARY_OP_ADD
                                          : SLANG_NVVM_FLOATING_BINARY_OP_SUBTRACT,
                                    loweredLeft,
                                    loweredRight,
                                    loweredValue)));
                        }
                        else
                        {
                            const SlangNVVMIntegerBinaryOp_3 operation =
                                inst->getOp() == kIROp_Add ? SLANG_NVVM_INTEGER_BINARY_OP_3_ADD
                                                           : SLANG_NVVM_INTEGER_BINARY_OP_3_SUB;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                inst->getOp() == kIROp_Add ? "signed i32 addition"
                                                           : "signed i32 subtraction",
                                builder.emitIntegerBinaryOperation(
                                    moduleScope.module,
                                    operation,
                                    loweredLeft,
                                    loweredRight,
                                    loweredValue)));
                        }
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 multiplication",
                            builder.emitIntegerBinaryOperation(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY,
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 bitwise AND",
                            builder.emitIntegerBinaryOperation(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_AND,
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 bitwise OR",
                            builder.emitIntegerBinaryOperation(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_OR,
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 bitwise XOR",
                            builder.emitIntegerBinaryOperation(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_XOR,
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
                            typeContext,
                            loweredOperand));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 bitwise NOT",
                            builder.emitIntegerUnary(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT,
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
                            typeContext,
                            loweredOperand));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 arithmetic negation",
                            builder.emitIntegerUnary(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_UNARY_OP_NEGATE,
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
                            typeContext,
                            loweredPointer));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 less-than comparison",
                            builder.emitIntegerCompare(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN,
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 equality comparison",
                            builder.emitIntegerCompare(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL,
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 inequality comparison",
                            builder.emitIntegerCompare(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL,
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 greater-than comparison",
                            builder.emitIntegerCompare(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN,
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 less-than-or-equal comparison",
                            builder.emitIntegerCompare(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL,
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
                            typeContext,
                            loweredLeft));
                        SlangNVVMValueHandle_1 loweredRight = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredRight));
                        SlangNVVMValueHandle_1 loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "signed i32 greater-than-or-equal comparison",
                            builder.emitIntegerCompare(
                                moduleScope.module,
                                SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL,
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
                                typeContext,
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
                            typeContext,
                            loweredBasePointer));
                        SlangNVVMValueHandle_1 loweredElementOffset = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
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
                            typeContext,
                            loweredBasePointer));
                        SlangNVVMValueHandle_1 loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
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
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle_1 loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
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
                            typeContext,
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
                            typeContext,
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
                        typeContext,
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
