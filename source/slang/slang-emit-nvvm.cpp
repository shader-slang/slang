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

// Slice 7 accepts only CUDA ABI `int`, whose width and natural alignment are both four bytes.
static const uint32_t kNVVMI32Alignment = 4;

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

// Checks that an executable operand has an accepted earlier definition that dominates its use.
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
    IRDominatorTree* dominatorTree)
{
    if (!value || !_isI32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 value"));
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

    for (auto block : entryPoint->getBlocks())
    {
        if (block != entryBlock && block->getFirstParam())
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block parameter"));

        IRTerminatorInst* terminator = block->getTerminator();
        if (!terminator)
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("missing terminator"));

        for (auto inst : block->getOrdinaryInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_Load:
                {
                    auto load = cast<IRLoad>(inst);
                    if (!_isI32Type(load->getDataType()))
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("load result type"));
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        load->getPtr(),
                        load,
                        availableValues,
                        dominatorTree,
                        false));
                    availableValues.add(load);
                    _requireCapability(outCapability, NVVMIRCapability::ScalarMemory);
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
                        dominatorTree));
                    _requireCapability(outCapability, NVVMIRCapability::ScalarMemory);
                }
                break;

            case kIROp_Add:
            case kIROp_Sub:
                if (inst->getOperandCount() != 2 || !_isI32Type(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 arithmetic"));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree));
                availableValues.add(inst);
                _requireCapability(outCapability, NVVMIRCapability::ScalarControlFlow);
                break;

            case kIROp_Less:
                if (inst->getOperandCount() != 2 || !_isBoolType(inst->getDataType()))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 comparison"));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree));
                availableValues.add(inst);
                _requireCapability(outCapability, NVVMIRCapability::ScalarControlFlow);
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
                    if (branch->getArgCount())
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch argument"));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        branch->getTargetBlock(),
                        functionBlocks));
                    _requireCapability(outCapability, NVVMIRCapability::ScalarControlFlow);
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
                    _requireCapability(outCapability, NVVMIRCapability::ScalarControlFlow);
                }
                break;

            default:
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    UnownedStringSlice(getIROpInfo(inst->getOp()).name));
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
        if (!i32Type)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "signed i32 type",
                builder.getIntegerType(moduleScope.module, 32, i32Type)));
        }

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

    Dictionary<IRInst*, SlangNVVMValueHandle_1> valueMap;
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
    // emitting any body instruction. Values remain one-to-one and are added only when emitted.
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

    for (auto block : entryPoint->getBlocks())
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
                    SlangNVVMValueHandle_1 loweredValue = nullptr;
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "signed i32 load",
                        builder.emitLoad(
                            moduleScope.module,
                            valueMap.getValue(load->getPtr()),
                            kNVVMI32Alignment,
                            loweredValue)));
                    valueMap[load] = loweredValue;
                }
                break;

            case kIROp_Store:
                {
                    auto store = cast<IRStore>(inst);
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "signed i32 store",
                        builder.emitStore(
                            moduleScope.module,
                            valueMap.getValue(store->getVal()),
                            valueMap.getValue(store->getPtr()),
                            kNVVMI32Alignment)));
                }
                break;

            case kIROp_Add:
            case kIROp_Sub:
                {
                    const SlangNVVMIntegerBinaryOp_2 operation =
                        inst->getOp() == kIROp_Add ? SLANG_NVVM_INTEGER_BINARY_OP_ADD
                                                   : SLANG_NVVM_INTEGER_BINARY_OP_SUB;
                    SlangNVVMValueHandle_1 loweredValue = nullptr;
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        inst->getOp() == kIROp_Add ? "signed i32 addition"
                                                   : "signed i32 subtraction",
                        builder.emitIntegerBinary(
                            moduleScope.module,
                            operation,
                            valueMap.getValue(inst->getOperand(0)),
                            valueMap.getValue(inst->getOperand(1)),
                            loweredValue)));
                    valueMap[inst] = loweredValue;
                }
                break;

            case kIROp_Less:
                {
                    SlangNVVMValueHandle_1 loweredValue = nullptr;
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "signed i32 less-than comparison",
                        builder.emitIntegerSignedLessThan(
                            moduleScope.module,
                            valueMap.getValue(inst->getOperand(0)),
                            valueMap.getValue(inst->getOperand(1)),
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
                {
                    auto branch = cast<IRUnconditionalBranch>(inst);
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "unconditional branch",
                        builder.emitBranch(
                            moduleScope.module,
                            blockMap.getValue(branch->getTargetBlock()))));
                }
                break;

            case kIROp_IfElse:
                {
                    auto ifElse = cast<IRIfElse>(inst);
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "conditional branch",
                        builder.emitConditionalBranch(
                            moduleScope.module,
                            valueMap.getValue(ifElse->getCondition()),
                            blockMap.getValue(ifElse->getTrueBlock()),
                            blockMap.getValue(ifElse->getFalseBlock()))));
                }
                break;

            default:
                SLANG_UNEXPECTED("NVVM emission received IR that was not preflighted");
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
