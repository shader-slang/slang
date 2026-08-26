#include "slang-emit-nvvm.h"

#include "compiler-core/slang-artifact-impl.h"
#include "compiler-core/slang-artifact-util.h"
#include "core/slang-dictionary.h"
#include "slang-code-gen.h"
#include "slang-diagnostics.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"

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

bool _isSelectedEntryPoint(const LinkedIR& linkedIR, IRInst* globalInst)
{
    for (auto entryPoint : linkedIR.entryPoints)
    {
        if (entryPoint == globalInst)
            return true;
    }
    return false;
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

// Returns whether `type` has a direct scalar CUDA launch-parameter representation.
bool _isSupportedParameterType(IRInst* type)
{
    return _isI32Type(type) || _asSupportedDevicePointerType(type);
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
    if (!ptrType)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("device i32 pointer"));
    if (requireWriteAccess && ptrType->getAccessQualifier() != AccessQualifier::ReadWrite)
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
List<IRBlock*> _getNVVMBodyOrder(IRFunc* entryPoint, IRDominatorTree* dominatorTree)
{
    List<IRBlock*> result;
    HashSet<IRBlock*> addedBlocks;
    for (auto block : getReversePostorder(entryPoint))
    {
        if (!dominatorTree->isUnreachable(block) && addedBlocks.add(block))
            result.add(block);
    }
    for (auto block : entryPoint->getBlocks())
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
    if (!entryPoint || !entryPoint->isDefinition())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point definition"));

    auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
    if (!entryPointDecoration)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point decoration"));
    if (entryPointDecoration->getProfile().getStage() != Stage::Compute)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point stage"));
    if (!entryPointDecoration->getName()->getStringSlice().getLength())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point name"));
    if (!as<IRVoidType>(entryPoint->getResultType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point result type"));

    IRBlock* entryBlock = entryPoint->getFirstBlock();
    if (!entryBlock)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry block"));

    HashSet<IRBlock*> functionBlocks;
    for (auto block : entryPoint->getBlocks())
    {
        functionBlocks.add(block);
    }
    if (functionBlocks.getCount() > 1)
        _requireCapability(outCapability, NVVMIRCapability::ScalarControlFlow);

    RefPtr<IRDominatorTree> dominatorTree = computeDominatorTree(entryPoint);
    List<IRBlock*> bodyOrder = _getNVVMBodyOrder(entryPoint, dominatorTree);
    for (auto block : bodyOrder)
    {
        if (!functionBlocks.contains(block))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch target"));
    }

    HashSet<IRInst*> availableValues;
    UInt actualParamCount = 0;
    for (auto param : entryPoint->getParams())
    {
        if (actualParamCount >= entryPoint->getParamCount() ||
            !_isSupportedParameterType(param->getDataType()) ||
            !isTypeEqual(param->getDataType(), entryPoint->getParamType(actualParamCount)))
        {
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point parameter"));
        }
        availableValues.add(param);
        ++actualParamCount;
    }
    if (actualParamCount != entryPoint->getParamCount())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point parameter count"));
    if (actualParamCount)
        _requireCapability(outCapability, NVVMIRCapability::ScalarMemory);

    // Register every accepted block parameter before checking uses because emission creates all
    // phi placeholders before any body. Ordinary values join this set in the second pass, in the
    // same order in which their LLVM instructions will be emitted.
    for (auto block : entryPoint->getBlocks())
    {
        if (block != entryBlock)
        {
            for (auto param : block->getParams())
            {
                if (!_isI32Type(param->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block parameter"));
                availableValues.add(param);
                _requireCapability(outCapability, NVVMIRCapability::ScalarSSA);
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
                _requireCapability(outCapability, NVVMIRCapability::ScalarMemory);
                break;

            case kIROp_Store:
                _requireCapability(outCapability, NVVMIRCapability::ScalarMemory);
                break;

            case kIROp_Add:
            case kIROp_Sub:
                if (inst->getOperandCount() != 2 || !_isI32Type(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 arithmetic"));
                _requireCapability(outCapability, NVVMIRCapability::ScalarControlFlow);
                break;

            case kIROp_Less:
                if (inst->getOperandCount() != 2 || !_isBoolType(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 comparison"));
                _requireCapability(outCapability, NVVMIRCapability::ScalarControlFlow);
                break;

            case kIROp_Return:
                break;

            case kIROp_UnconditionalBranch:
            case kIROp_Loop:
            case kIROp_IfElse:
                _requireCapability(outCapability, NVVMIRCapability::ScalarControlFlow);
                break;

            default:
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    UnownedStringSlice(getIROpInfo(inst->getOp()).name));
            }
        }
    }

    // Reachable reverse postorder puts every dominating ordinary producer before its consumer
    // without making physical sibling order part of legality. The helper retains Slice 7's physical
    // ordering for unreachable blocks. Phi definitions are already available in every block.
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
                        outCapability));
                }
                break;

            case kIROp_Add:
            case kIROp_Sub:
            case kIROp_Less:
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree,
                    outCapability));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree,
                    outCapability));
                availableValues.add(inst);
                break;

            case kIROp_Return:
                {
                    auto returnInst = cast<IRReturn>(inst);
                    if (returnInst != terminator || !returnInst->getVal() ||
                        returnInst->getVal()->getOp() != kIROp_VoidLit)
                    {
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("return value"));
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
                        outCapability));
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
                        outCapability));
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
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch position"));
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

    // Every non-entry phi needs at least one actual CFG predecessor. Structural `IRLoop`
    // break/continue and `IRIfElse::afterBlock` operands are deliberately absent from this list.
    for (auto block : entryPoint->getBlocks())
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

    // Scalar CUDA launch parameters and executable scalar operations are meaningful only for a
    // CUDA kernel. Preserve Slice 6's conventional zero-parameter empty compute entry point, but
    // do not invent a raw CUDA launch ABI for an ordinary shader entry point.
    if (outCapability != NVVMIRCapability::Minimal &&
        !entryPoint->findDecoration<IRCudaKernelDecoration>())
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("CUDA kernel decoration"));
    }

    // Linking can retain module-scope types, layouts, capabilities, and constants needed to spell
    // the selected function. `IRStructKey` is also layout-only identity retained for raw CUDA
    // parameter layouts even though its op is not classified as hoistable. None of these nodes
    // denotes executable code or storage. Reject every other semantic global so this emitter
    // cannot silently drop a parameter, helper, initializer, or exported function.
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        if (as<IRDecoration>(globalInst) || as<IRConstant>(globalInst) ||
            as<IRStructKey>(globalInst) || _isSelectedEntryPoint(linkedIR, globalInst) ||
            getIROpInfo(globalInst->getOp()).isHoistable())
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
    List<SlangNVVMTypeHandle_1> parameterTypes;
    for (auto param : entryPoint->getParams())
    {
        SLANG_RETURN_ON_FAIL(_getNVVMI32Type(codeGenContext, builder, moduleScope.module, i32Type));

        if (_isI32Type(param->getDataType()))
        {
            parameterTypes.add(i32Type);
        }
        else
        {
            SLANG_RELEASE_ASSERT(_asSupportedDevicePointerType(param->getDataType()));
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
        }
    }

    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "function type",
        builder.getFunctionType(
            moduleScope.module,
            voidType,
            parameterTypes.getCount() ? parameterTypes.getBuffer() : nullptr,
            size_t(parameterTypes.getCount()),
            functionType)));

    SlangNVVMValueHandle_1 function = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "function declaration",
        builder.declareFunction(
            moduleScope.module,
            functionType,
            entryPointDecoration->getName()->getStringSlice(),
            function)));

    NVVMValueMap valueMap;
    size_t parameterIndex = 0;
    for (auto param : entryPoint->getParams())
    {
        SlangNVVMValueHandle_1 parameter = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "function parameter",
            builder.getFunctionParameter(moduleScope.module, function, parameterIndex, parameter)));
        valueMap[param] = parameter;
        ++parameterIndex;
    }

    // LLVM branches can refer to blocks declared later, so create the complete function CFG before
    // emitting any body instruction.
    Dictionary<IRBlock*, SlangNVVMBlockHandle_1> blockMap;
    Index blockIndex = 0;
    for (auto block : entryPoint->getBlocks())
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
                function,
                blockName.getUnownedSlice(),
                loweredBlock)));
        blockMap[block] = loweredBlock;
        ++blockIndex;
    }

    // Consider the loop header `header(i, sum)`. Its phis must exist before the compare and body
    // use them, while their backedge values are not emitted until later blocks. Create every phi
    // placeholder now; incoming pairs are attached after all bodies and terminators exist.
    IRBlock* entryBlock = entryPoint->getFirstBlock();
    for (auto block : entryPoint->getBlocks())
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

    RefPtr<IRDominatorTree> dominatorTree = computeDominatorTree(entryPoint);
    List<IRBlock*> bodyOrder = _getNVVMBodyOrder(entryPoint, dominatorTree);
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

            case kIROp_Return:
                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                    codeGenContext,
                    "void return",
                    builder.emitReturnVoid(moduleScope.module)));
                break;

            case kIROp_UnconditionalBranch:
            case kIROp_Loop:
                {
                    auto branch = cast<IRUnconditionalBranch>(inst);
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        inst->getOp() == kIROp_Loop ? "loop entry branch" : "unconditional branch",
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
    // edge feeds parameter N. At this point even loop backedge instructions exist, so every pair
    // can be attached without reconstructing a local variable or searching an operand graph.
    for (auto block : entryPoint->getBlocks())
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

    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "kernel annotation",
        builder.markFunctionAsKernel(moduleScope.module, function)));

    if (!builder.supportsSerializationDiagnostics())
    {
        return _requireBuilderOperation(
            codeGenContext,
            "verified bitcode serialization",
            SLANG_E_NOT_AVAILABLE);
    }

    ComPtr<ISlangBlob> bitcode;
    String verifierDiagnostics;
    SlangResult serializationResult = builder.serializeModule(
        moduleScope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        bitcode,
        verifierDiagnostics);
    if (SLANG_FAILED(serializationResult))
    {
        _requireBuilderOperation(
            codeGenContext,
            "verified bitcode serialization",
            serializationResult);
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
    if (!bitcode || !bitcode->getBufferSize())
    {
        return _requireBuilderOperation(
            codeGenContext,
            "verified bitcode serialization",
            SLANG_FAIL);
    }

    auto artifact = ArtifactUtil::createArtifact(ArtifactDesc::make(
        ArtifactKind::ObjectCode,
        ArtifactPayload::LLVMIR,
        ArtifactStyle::Kernel));
    artifact->addRepresentationUnknown(bitcode);
    ArtifactUtil::addAssociated(artifact, linkedIR.metadata);
    outArtifact = artifact;
    return SLANG_OK;
}

} // namespace Slang
