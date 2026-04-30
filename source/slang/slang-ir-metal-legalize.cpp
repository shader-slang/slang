#include "slang-ir-metal-legalize.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-legalize-binary-operator.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-target-program.h"
#include "slang-target.h"

namespace Slang
{

// metal textures only support writing 4-component values, even if the texture is only 1, 2, or
// 3-component in this case the other channels get ignored, but the signature still doesnt match so
// now we have to replace the value being written with a 4-component vector where the new components
// get ignored, nice
void legalizeImageStoreValue(IRBuilder& builder, IRImageStore* imageStore)
{
    builder.setInsertBefore(imageStore);
    auto originalValue = imageStore->getValue();
    auto valueBaseType = originalValue->getDataType();
    IRType* elementType = nullptr;
    List<IRInst*> components;
    if (auto valueVectorType = as<IRVectorType>(valueBaseType))
    {
        if (auto originalElementCount = as<IRIntLit>(valueVectorType->getElementCount()))
        {
            if (originalElementCount->getValue() == 4)
            {
                return;
            }
        }
        elementType = valueVectorType->getElementType();

        // Extract components using IRElementExtract to handle any vector instruction type
        if (auto originalElementCount = as<IRIntLit>(valueVectorType->getElementCount()))
        {
            for (UInt i = 0; i < (UInt)originalElementCount->getValue(); i++)
            {
                auto elementExtract = builder.emitElementExtract(
                    elementType,
                    originalValue,
                    builder.getIntValue(builder.getIntType(), i));
                components.add(elementExtract);
            }
        }
    }
    else
    {
        elementType = valueBaseType;
        components.add(originalValue);
    }
    for (UInt i = components.getCount(); i < 4; i++)
    {
        components.add(builder.getIntValue(builder.getIntType(), 0));
    }
    auto fourComponentVectorType = builder.getVectorType(elementType, 4);
    imageStore->setOperand(2, builder.emitMakeVector(fourComponentVectorType, components));
}

void legalizeFuncBody(IRFunc* func)
{
    IRBuilder builder(func);
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getModifiableChildren())
        {
            if (auto call = as<IRCall>(inst))
            {
                ShortList<IRUse*> argsToFixup;
                // Metal doesn't support taking the address of a vector element.
                // If such an address is used as an argument to a call, we need to replace it with a
                // temporary. for example, if we see:
                // ```
                //     void foo(inout float x) { x = 1; }
                //     float4 v;
                //     foo(v.x);
                // ```
                // We need to transform it into:
                // ```
                //     float4 v;
                //     float temp = v.x;
                //     foo(temp);
                //     v.x = temp;
                // ```
                //
                for (UInt i = 0; i < call->getArgCount(); i++)
                {
                    if (auto addr = as<IRGetElementPtr>(call->getArg(i)))
                    {
                        auto ptrType = addr->getBase()->getDataType();
                        auto valueType = tryGetPointedToType(&builder, ptrType);
                        if (!valueType)
                            continue;
                        if (as<IRVectorType>(valueType))
                            argsToFixup.add(call->getArgs() + i);
                    }
                }
                if (argsToFixup.getCount() == 0)
                    continue;

                // Define temp vars for all args that need fixing up.
                for (auto arg : argsToFixup)
                {
                    auto addr = as<IRGetElementPtr>(arg->get());
                    auto ptrType = addr->getDataType();
                    auto valueType = tryGetPointedToType(&builder, ptrType);
                    builder.setInsertBefore(call);
                    auto temp = builder.emitVar(valueType);
                    auto initialValue = builder.emitLoad(valueType, addr);
                    builder.emitStore(temp, initialValue);
                    builder.setInsertAfter(call);
                    builder.emitStore(addr, builder.emitLoad(valueType, temp));
                    arg->set(temp);
                }
            }
            if (auto write = as<IRImageStore>(inst))
            {
                legalizeImageStoreValue(builder, write);
            }
        }
    }
}

struct MetalAddressSpaceAssigner : InitialAddressSpaceAssigner
{
    virtual bool tryAssignAddressSpace(IRInst* inst, AddressSpace& outAddressSpace) override
    {
        switch (inst->getOp())
        {
        case kIROp_Var:
            outAddressSpace = AddressSpace::ThreadLocal;
            return true;
        case kIROp_RWStructuredBufferGetElementPtr:
            outAddressSpace = AddressSpace::Global;
            return true;
        case kIROp_Load:
            {
                auto addrSpace = getAddressSpaceFromVarType(inst->getDataType());
                if (addrSpace != AddressSpace::Generic)
                {
                    outAddressSpace = addrSpace;
                    return true;
                }
            }
            return false;
        default:
            return false;
        }
    }

    virtual AddressSpace getAddressSpaceFromVarType(IRInst* type) override
    {
        if (as<IRUniformParameterGroupType>(type))
        {
            return AddressSpace::Uniform;
        }
        if (as<IRByteAddressBufferTypeBase>(type))
        {
            return AddressSpace::Global;
        }
        if (as<IRHLSLStructuredBufferTypeBase>(type))
        {
            return AddressSpace::Global;
        }
        if (as<IRGLSLShaderStorageBufferType>(type))
        {
            return AddressSpace::Global;
        }
        if (auto ptrType = as<IRPtrTypeBase>(type))
        {
            if (ptrType->hasAddressSpace())
                return ptrType->getAddressSpace();
            return AddressSpace::Generic;
        }
        return AddressSpace::Generic;
    }

    virtual AddressSpace getLeafInstAddressSpace(IRInst* inst) override
    {
        if (as<IRGroupSharedRate>(inst->getRate()))
            return AddressSpace::GroupShared;
        switch (inst->getOp())
        {
        case kIROp_RWStructuredBufferGetElementPtr:
            return AddressSpace::Global;
        case kIROp_Var:
            if (as<IRBlock>(inst->getParent()))
                return AddressSpace::ThreadLocal;
            break;
        default:
            break;
        }
        auto type = unwrapAttributedType(inst->getDataType());
        if (!type)
            return AddressSpace::Generic;
        return getAddressSpaceFromVarType(type);
    }
};

static void processInst(IRInst* inst, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    switch (inst->getOp())
    {
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_FRem:
    case kIROp_IRem:
    case kIROp_And:
    case kIROp_Or:
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_Lsh:
    case kIROp_Rsh:
    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Greater:
    case kIROp_Less:
    case kIROp_Geq:
    case kIROp_Leq:
        legalizeBinaryOp(inst, sink, targetProgram);
        break;
    case kIROp_MeshOutputRef:
        sink->diagnose(Diagnostics::AssignToRefNotSupported{.location = getDiagnosticPos(inst)});
        break;
    case kIROp_MetalCastToDepthTexture:
        {
            // If the operand is already a depth texture, don't do anything.
            auto textureType = as<IRTextureTypeBase>(inst->getOperand(0)->getDataType());
            if (textureType && getIntVal(textureType->getIsShadowInst()) == 1)
            {
                inst->replaceUsesWith(inst->getOperand(0));
                inst->removeAndDeallocate();
            }
            break;
        }
    default:
        for (auto child : inst->getModifiableChildren())
        {
            processInst(child, targetProgram, sink);
        }
    }
}

static void legalizeSubpassInputsForMetal(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink,
    List<EntryPointInfo>& entryPoints)
{
    List<IRGlobalParam*> subpassGlobals;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto globalParam = as<IRGlobalParam>(inst))
        {
            if (as<IRSubpassInputType>(globalParam->getDataType()))
                subpassGlobals.add(globalParam);
        }
    }

    IRFunc* entryPointToFix = nullptr;
    for (auto globalParam : subpassGlobals)
    {
        auto subpassType = as<IRSubpassInputType>(globalParam->getDataType());
        auto elementType = subpassType->getElementType();

        if (subpassType->isMultisample())
        {
            sink->diagnose(
                Diagnostics::MultisampledSubpassInputNotSupportedOnMetal{
                    .location = getDiagnosticPos(globalParam)});
        }

        auto entryPointParamDecor =
            globalParam->findDecoration<IREntryPointParamDecoration>();
        IRFunc* entryPointFunc = nullptr;
        if (entryPointParamDecor)
            entryPointFunc = as<IRFunc>(entryPointParamDecor->getEntryPoint());

        if (!entryPointFunc)
        {
            for (auto use = globalParam->firstUse; use; use = use->nextUse)
            {
                auto parentFunc = getParentFunc(use->getUser());
                if (!parentFunc)
                    continue;
                for (auto& ep : entryPoints)
                {
                    if (ep.entryPointFunc == parentFunc &&
                        ep.entryPointDecor->getProfile().getStage() == Stage::Fragment)
                    {
                        entryPointFunc = ep.entryPointFunc;
                        break;
                    }
                }
                if (entryPointFunc)
                    break;
            }
        }
        if (!entryPointFunc)
        {
            sink->diagnose(
                Diagnostics::SubpassInputUsedOutsideEntryPoint{
                    .location = getDiagnosticPos(globalParam)});
            continue;
        }

        IRIntegerValue attachmentIndex = 0;
        if (auto layoutDecor = globalParam->findDecoration<IRLayoutDecoration>())
        {
            if (auto varLayout = as<IRVarLayout>(layoutDecor->getLayout()))
            {
                if (auto offsetAttr =
                        varLayout->findOffsetAttr(LayoutResourceKind::InputAttachmentIndex))
                {
                    attachmentIndex = offsetAttr->getOffset();
                }
            }
        }

        IRBuilder builder(module);
        auto firstBlock = entryPointFunc->getFirstBlock();
        if (!firstBlock)
            continue;

        auto newParam = builder.createParam(elementType);

        auto firstOrdinary = firstBlock->getFirstOrdinaryInst();
        if (firstOrdinary)
            newParam->insertBefore(firstOrdinary);
        else
            newParam->insertAtEnd(firstBlock);

        StringBuilder colorStr;
        colorStr << "color(" << Int(attachmentIndex) << ")";
        String colorString = colorStr.produceString();
        builder.addTargetSystemValueDecoration(newParam, colorString.getUnownedSlice());

        if (auto nameHint = globalParam->findDecoration<IRNameHintDecoration>())
            builder.addNameHintDecoration(newParam, nameHint->getName());

        IRUse* nextUse = nullptr;
        for (IRUse* use = globalParam->firstUse; use; use = nextUse)
        {
            nextUse = use->nextUse;
            auto user = use->getUser();

            if (getParentFunc(user) != entryPointFunc)
            {
                sink->diagnose(
                    Diagnostics::SubpassInputUsedOutsideEntryPoint{
                        .location = getDiagnosticPos(user)});
                IRBuilder localBuilder(user);
                localBuilder.setInsertBefore(user);
                use->set(localBuilder.emitPoison(elementType));
                continue;
            }

            if (user->getOp() == kIROp_SubpassLoad)
            {
                // For SubpassInputMS, the sample operand is silently dropped
                // since Metal framebuffer fetch doesn't support per-sample reads.
                // The unused sample operand will be cleaned up by DCE.
                user->replaceUsesWith(newParam);
                user->removeAndDeallocate();
                continue;
            }
            use->set(newParam);
        }

        globalParam->removeAndDeallocate();
        entryPointToFix = entryPointFunc;
    }

    if (entryPointToFix)
        fixUpFuncType(entryPointToFix);
}

void legalizeIRForMetal(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    List<EntryPointInfo> entryPoints;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            if (auto entryPointDecor = func->findDecoration<IREntryPointDecoration>())
            {
                EntryPointInfo info;
                info.entryPointDecor = entryPointDecor;
                info.entryPointFunc = func;
                entryPoints.add(info);
            }
            legalizeFuncBody(func);
        }
    }

    legalizeSubpassInputsForMetal(module, targetProgram, sink, entryPoints);

    legalizeEntryPointVaryingParamsForMetal(module, sink, entryPoints);

    processInst(module->getModuleInst(), targetProgram, sink);
}

void specializeAddressSpaceForMetal(IRModule* module)
{
    MetalAddressSpaceAssigner metalAddressSpaceAssigner;
    specializeAddressSpace(module, &metalAddressSpaceAssigner);
}

} // namespace Slang
