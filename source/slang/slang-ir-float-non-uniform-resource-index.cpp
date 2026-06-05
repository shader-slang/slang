#include "slang-ir-float-non-uniform-resource-index.h"

#include "slang-ir-util.h"

// NonUniform propagation for SPIR-V
// ==================================
//
// When a shader indexes a resource array with NonUniformResourceIndex(idx),
// Vulkan (VUID-RuntimeSpirv-None-10148) requires the NonUniform decoration
// on the resource operand consumed by the sampling/memory instruction.
//
// Two passes cooperate to achieve this:
//
// 1. Float pass (this file, called during SPIR-V legalization):
//    Bubbles the NonUniformResourceIndex wrapper outward through the
//    use-def chain (GetElement, Load, MakeCombinedTextureSampler,
//    CombinedTextureSamplerGetTexture, ImageTexelPointer, etc.).
//    When the wrapper reaches an instruction the pass cannot float
//    through (e.g. the spirv_asm boundary), the decoration phase
//    walks back through the chain via decorateNonUniformChain to
//    attach IRSPIRVNonUniformResourceDecoration to the index and
//    all intermediate resource-creating ops.
//
// 2. Legalize propagation (propagateNonUniformDecorations in
//    slang-ir-spirv-legalize.cpp): A single forward linear scan
//    that runs after processGlobalParam rewrites getElement to
//    getElementPtr. Syncs NonUniform between access-chain indices
//    and their instructions (bidirectional), and forward-propagates
//    through Load and FieldAddress/FieldExtract to reach the final
//    resource operand.
//
// Resource-creating ops (MakeCombinedTextureSampler,
// CombinedTextureSamplerGetTexture, ImageTexelPointer) are handled
// exclusively by the float pass. The legalize propagation handles
// access chains, loads, and field accesses.

namespace Slang
{
// Walk back through resource-creating ops to find and decorate the
// access-chain index that is the source of non-uniformity.
// Terminates because the IR operand graph is acyclic (SSA dominance order).
static void decorateNonUniformChain(IRInst* operand, const std::function<void(IRInst*)>& decorate)
{
    if (auto gep = as<IRGetElementPtr>(operand))
    {
        decorate(gep->getOperand(1));
    }
    else if (auto getElement = as<IRGetElement>(operand))
    {
        decorate(getElement->getOperand(1));
    }
    else if (auto load = as<IRLoad>(operand))
    {
        auto addr = load->getOperand(0);
        if (auto addrGep = as<IRGetElementPtr>(addr))
            decorate(addrGep->getOperand(1));
        else if (auto addrGetElement = as<IRGetElement>(addr))
            decorate(addrGetElement->getOperand(1));
        else
        {
            decorate(operand);
            decorate(addr);
        }
    }
    else if (auto makeCombined = as<IRMakeCombinedTextureSampler>(operand))
    {
        decorate(operand);
        decorateNonUniformChain(makeCombined->getOperand(0), decorate);
        decorateNonUniformChain(makeCombined->getOperand(1), decorate);
    }
    else if (
        as<IRCombinedTextureSamplerGetTexture>(operand) ||
        as<IRCombinedTextureSamplerGetSampler>(operand) ||
        operand->getOp() == kIROp_ImageTexelPointer ||
        operand->getOp() == kIROp_GetLegalizedSPIRVGlobalParamAddr)
    {
        decorate(operand);
        decorateNonUniformChain(operand->getOperand(0), decorate);
    }
    else
    {
        decorate(operand);
    }
}

void processNonUniformResourceIndex(
    IRInst* nonUniformResourceIndexInst,
    NonUniformResourceIndexFloatMode floatMode)
{
    // float `NonUniformResourceIndex()` to right before the access operation
    // by walking up the use-def chain
    // from nonUniformResource inst of an index to an array of buffer or
    // texture def all the way to the leaf operations. To be precise:
    // - go through GEP and see if it calls an intrinsic function,
    //   then decorate the address itself (GetElementPtr)
    // - go through GEP to identify the pointer access and the Loads that it
    //   accesses (GetElementPtr -> Load), then decorate the load instruction.
    // - go through IntCasts to deal with u32 -> i32 / vice-versa (IntCast)
    List<IRInst*> resWorkList;

    // Handle cases when `nonUniformResourceIndexInst` inst is wrapped around
    // an index in a nested fashion, i.e. nonUniform(nonUniform(index)) by
    // only adding the inner-most inst in the worklist, and work our way out.
    auto insti = nonUniformResourceIndexInst;
    while (insti->getOp() == kIROp_NonUniformResourceIndex)
    {
        if (resWorkList.getCount() != 0)
            resWorkList.removeLast();
        resWorkList.add(insti);
        insti = insti->getOperand(0);
    }

    // For all the users of a `nonUniformResourceIndexInst`, make them directly
    // use the underlying base inst that is wrapped by `nonUniformResourceIndex`
    // and finally wrap them with a `nonUniformResourceIndex`, and add back to the
    // worklist, and keep bubbling them up until it can.
    for (Index i = 0; i < resWorkList.getCount(); i++)
    {
        auto inst = resWorkList[i];
        traverseUses(
            inst,
            [&](IRUse* use)
            {
                auto user = use->getUser();
                IRBuilder builder(user);
                builder.setInsertBefore(user);

                IRInst* newUser = nullptr;
                switch (user->getOp())
                {
                case kIROp_IntCast:
                    // Replace intCast(nonUniformRes(x)), into nonUniformRes(intCast(x))
                    newUser = builder.emitCast(user->getFullType(), inst->getOperand(0));
                    break;
                case kIROp_CastDescriptorHandleToUInt2:
                    {
                        // Replace castBindlessToInt(nonUniformRes(x)), into
                        // nonUniformRes(castBindlessToInt(x))
                        auto operand = inst->getOperand(0);
                        newUser = builder.emitIntrinsicInst(
                            user->getFullType(),
                            kIROp_CastDescriptorHandleToUInt2,
                            1,
                            &operand);
                    }
                    break;
                case kIROp_GetElementPtr:
                    // Ignore when `NonUniformResourceIndex` is not on the index
                    if (floatMode != NonUniformResourceIndexFloatMode::SPIRV)
                        break;
                    if (user->getOperand(1) == inst)
                    {
                        // Replace gep(pArray, nonUniformRes(x)), into
                        // nonUniformRes(gep(pArray, x))
                        newUser = builder.emitElementAddress(
                            user->getFullType(),
                            user->getOperand(0),
                            inst->getOperand(0));
                    }
                    break;
                case kIROp_GetElement:
                    // A getElement can use the `NonUniformResourceIndex` either as
                    // its base (operand 0) or as its index (operand 1). The base
                    // case runs unconditionally (the base wraps a resource value
                    // that all targets need floated). The index case is SPIRV-only
                    // because only the SPIR-V backend requires the NonUniform
                    // decoration on the access index itself -- other targets do not
                    // model per-index NonUniform and would produce spurious wrappers.
                    // Both operands cannot be the same NonUniformResourceIndex
                    // (base is array-typed, index is integer-typed).
                    if (user->getOperand(0) == inst)
                    {
                        // Replace getElement(nonUniformRes(obj), i), into
                        // nonUniformRes(getElement(obj, i))
                        newUser = builder.emitElementExtract(
                            user->getFullType(),
                            inst->getOperand(0),
                            user->getOperand(1));
                    }
                    else if (
                        floatMode == NonUniformResourceIndexFloatMode::SPIRV &&
                        user->getOperand(1) == inst)
                    {
                        // Replace getElement(obj, nonUniformRes(i)), into
                        // nonUniformRes(getElement(obj, i))
                        newUser = builder.emitElementExtract(
                            user->getFullType(),
                            user->getOperand(0),
                            inst->getOperand(0));
                    }
                    break;
                case kIROp_Swizzle:
                    // Ignore when `NonUniformResourceIndex` is not on base
                    if (user->getOperand(0) == inst)
                    {
                        // Replace swizzle(nonUniformRes(obj), indices), into
                        // nonUniformRes(swizzle(obj, indices))
                        ShortList<IRInst*> operands;
                        for (UInt i = 0; i < user->getOperandCount(); i++)
                            operands.add(user->getOperand(i));
                        operands[0] = inst->getOperand(0);
                        newUser = builder.emitIntrinsicInst(
                            user->getFullType(),
                            kIROp_Swizzle,
                            operands.getCount(),
                            operands.getArrayView().getBuffer());
                    }
                    break;
                case kIROp_NonUniformResourceIndex:
                    // Replace nonUniformRes(nonUniformRes(x)), into nonUniformRes(x)
                    newUser = inst->getOperand(0);
                    break;
                case kIROp_Load:
                    if (floatMode != NonUniformResourceIndexFloatMode::SPIRV)
                        break;
                    newUser = builder.emitLoad(user->getFullType(), inst->getOperand(0));
                    break;
                case kIROp_GetLegalizedSPIRVGlobalParamAddr:
                    if (floatMode != NonUniformResourceIndexFloatMode::SPIRV)
                        break;
                    {
                        auto operand = inst->getOperand(0);
                        IRInst* operands[] = {operand};
                        newUser = builder.emitIntrinsicInst(
                            user->getFullType(),
                            kIROp_GetLegalizedSPIRVGlobalParamAddr,
                            1,
                            operands);
                    }
                    break;
                case kIROp_MakeCombinedTextureSampler:
                    {
                        auto tex = user->getOperand(0);
                        auto samp = user->getOperand(1);
                        if (tex == inst)
                            tex = inst->getOperand(0);
                        else if (samp == inst)
                            samp = inst->getOperand(0);
                        else
                            SLANG_UNREACHABLE("NonUniformResourceIndex must be an operand of "
                                              "MakeCombinedTextureSampler");
                        newUser =
                            builder.emitMakeCombinedTextureSampler(user->getFullType(), tex, samp);
                    }
                    break;
                case kIROp_CombinedTextureSamplerGetTexture:
                case kIROp_CombinedTextureSamplerGetSampler:
                case kIROp_ImageTexelPointer:
                    {
                        ShortList<IRInst*> operands;
                        for (UInt i = 0; i < user->getOperandCount(); i++)
                            operands.add(
                                user->getOperand(i) == inst ? inst->getOperand(0)
                                                            : user->getOperand(i));
                        newUser = builder.emitIntrinsicInst(
                            user->getFullType(),
                            user->getOp(),
                            operands.getCount(),
                            operands.getArrayView().getBuffer());
                    }
                    break;
                default:
                    // Ignore for all other unknown insts.
                    break;
                };

                // Early exit when we could not process the `NonUniformResourceIndex` inst.
                if (!newUser)
                    return;

                auto nonuniformUser = builder.emitNonUniformResourceIndexInst(newUser);
                user->replaceUsesWith(nonuniformUser);

                // Update the worklist with the newly added `NonUniformResourceIndex` inst,
                // based on the base inst it was constructed around, in case we need to further
                // bubble up the `NonUniformResourceIndex` inst.
                switch (user->getOp())
                {
                case kIROp_IntCast:
                case kIROp_GetElementPtr:
                case kIROp_Load:
                case kIROp_GetLegalizedSPIRVGlobalParamAddr:
                case kIROp_NonUniformResourceIndex:
                case kIROp_CastDescriptorHandleToUInt2:
                case kIROp_GetElement:
                case kIROp_Swizzle:
                case kIROp_MakeCombinedTextureSampler:
                case kIROp_CombinedTextureSamplerGetTexture:
                case kIROp_CombinedTextureSamplerGetSampler:
                case kIROp_ImageTexelPointer:
                    resWorkList.add(nonuniformUser);
                    break;
                };

                // Clean up the base inst from the IR module, to avoid duplicate decorations.
                user->removeAndDeallocate();
            });
    }

    if (floatMode != NonUniformResourceIndexFloatMode::SPIRV)
        return;
    // Once all the `NonUniformResourceIndex` insts are visited, and the inst type is bubbled up
    // to the parent, a decoration is added to the operands of the insts.
    for (int i = 0; i < resWorkList.getCount(); ++i)
    {
        // It is only required to decorate the base inst, if the `NonUniformResourceIndex` inst
        // around it has any active uses.
        auto inst = resWorkList[i];
        if (!inst->hasUses())
        {
            inst->removeAndDeallocate();
            continue;
        }
        // For each remaining `NonUniformResourceIndex` inst, walk back through
        // the operand chain to find and decorate the access-chain index. For
        // resource-creating ops (MakeCombinedTextureSampler, etc.) also
        // decorate the op itself, then recurse into its source operand.
        auto operand = inst->getOperand(0);
        auto type = operand->getDataType();
        if (isResourceType(type) || isPointerToResourceType(type))
        {
            IRBuilder builder(operand);
            auto decorate = [&](IRInst* value)
            {
                if (!value->findDecoration<IRSPIRVNonUniformResourceDecoration>())
                    builder.addSPIRVNonUniformResourceDecoration(value);
            };
            decorateNonUniformChain(operand, decorate);
        }
        inst->replaceUsesWith(operand);

        // After replacing uses, any ImageSubscript that consumed the
        // NonUniformResourceIndex wrapper now uses the operand directly.
        // Propagate NonUniform forward so the texel pointer emitted for
        // the ImageSubscript carries the decoration for atomic operations
        // (VUID-RuntimeSpirv-None-10148).
        if (isResourceType(type) || isPointerToResourceType(type))
        {
            IRBuilder builder(operand);
            for (auto use = operand->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (user->getOp() == kIROp_ImageSubscript &&
                    !user->findDecoration<IRSPIRVNonUniformResourceDecoration>())
                {
                    builder.addSPIRVNonUniformResourceDecoration(user);
                }
            }
        }
        inst->removeAndDeallocate();
    }
}

void floatNonUniformResourceIndex(IRModule* module, NonUniformResourceIndexFloatMode floatMode)
{
    // Walk through all the instructions in the module, and float the `NonUniformResourceIndex`
    // insts to the right place in the IR module.

    List<IRInst*> workList;
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRGlobalValueWithCode>(getGenericReturnVal(globalInst));
        if (!func)
            continue;
        workList.clear();
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (inst->getOp() == kIROp_NonUniformResourceIndex)
                    workList.add(inst);
            }
        }
        for (auto inst : workList)
        {
            if (inst->getParent() != nullptr)
                processNonUniformResourceIndex(inst, floatMode);
        }
    }
}
} // namespace Slang
