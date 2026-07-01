#include "slang-ir-float-non-uniform-resource-index.h"

#include "slang-ir-util.h"

// NonUniform propagation for SPIR-V
// ==================================
//
// When a shader indexes a resource array with NonUniformResourceIndex(idx),
// Vulkan (VUID-RuntimeSpirv-None-10148) requires the NonUniform decoration
// on the resource operand consumed by the sampling/memory instruction.
//
// Target applicability
// --------------------
// This pass runs for every target (see the two callers below), but the two
// floatModes do very different amounts of work, and the heavy propagation in
// this file matters for exactly one target: the direct SPIR-V backend.
//
//   * SPIRV mode (called from slang-ir-spirv-legalize.cpp): the direct SPIR-V
//     emitter is the only backend with no downstream shader compiler, so Slang
//     itself must attach OpDecorate NonUniform to every consumed resource
//     operand to satisfy VUID-RuntimeSpirv-None-10148. That is why this mode
//     bubbles the wrapper all the way to the leaf and then runs the decoration
//     phase (decorateNonUniformChain + the legalize forward scan).
//
//   * Textual mode (called from slang-emit.cpp for all !isSPIRV targets): no
//     decoration is emitted (the function returns before the decoration phase).
//     The pass only repositions the NonUniformResourceIndex wrapper so it ends
//     up wrapping the index expression; the rest is the source emitter's job.
//     Per each target's own spec, the non-uniform concept lives on the *index*
//     (and the downstream compiler propagates it) or does not exist at all:
//       - HLSL: re-emitted as the textual `NonUniformResourceIndex(index)`
//         hint. The D3D spec defines it as a hint applied to the indexing
//         expression (indices are wave-uniform by default); DXC/FXC then
//         propagates correctness to the consumed resource. So Slang only needs
//         the wrapper on the index. (DirectX-Specs: Resource Binding, "Shader
//         Derivatives and Divergent Indexing"; SM 6.6 Dynamic Resources.)
//       - GLSL: re-emitted as `nonuniformEXT(index)`. GLSL *does* support
//         non-uniform indexing -- the GL_EXT_nonuniform_qualifier spec defines
//         `nonuniformEXT` as a qualifier/constructor applied to the index or
//         expression -- and glslang propagates it downstream. (Khronos
//         GL_EXT_nonuniform_qualifier.)
//       - Metal / WGSL / CUDA / CPU: the wrapper is dropped at emit time (the
//         CLikeSourceEmitter base just emits operand 0), because none of these
//         have a non-uniform-resource-indexing annotation to carry:
//           . Metal allows dynamic/non-uniform indexing of argument-buffer
//             resource arrays with no shader-side qualifier (Metal Shading
//             Language Spec 2.13 Argument Buffers; Tier 2 bindless).
//           . WGSL/WebGPU has no non-uniform annotation: binding arrays are
//             still proposal-stage and, per that proposal, "do not require
//             explicit non-uniform annotations" (gpuweb sized-binding-arrays).
//           . CUDA/CPU have no descriptor-array / bindless concept at all.
//         There is nothing to propagate, so doing more here would have no
//         target syntax to emit it into.
//
// So the decoration machinery below is intentionally SPIR-V-only: every other
// target either delegates propagation to its downstream compiler (HLSL, GLSL)
// or has no way to express the concept (Metal, WGSL, CUDA, CPU).
//
// Two passes cooperate to achieve this for SPIR-V, and both live in this file:
//
// 1. Float pass (processNonUniformResourceIndex / floatNonUniformResourceIndex):
//    Bubbles the NonUniformResourceIndex wrapper outward through the
//    use-def chain (GetElement, Load, MakeCombinedTextureSampler,
//    CombinedTextureSamplerGetTexture, ImageTexelPointer, etc.).
//    When the wrapper reaches an instruction the pass cannot float
//    through (e.g. the spirv_asm boundary), the decoration phase
//    walks back through the chain via decorateNonUniformChain to
//    attach IRSPIRVNonUniformResourceDecoration to the index and
//    all intermediate resource-creating ops.
//
// 2. Forward scan (propagateNonUniformDecorations): A single forward
//    linear scan that runs after the legalizer rewrites getElement to
//    getElementPtr. Syncs NonUniform between access-chain indices
//    and their instructions (bidirectional), and forward-propagates
//    through Load, FieldAddress/FieldExtract, and ImageSubscript to
//    reach the final resource operand.
//
// Resource-creating ops (MakeCombinedTextureSampler,
// CombinedTextureSamplerGetTexture, ImageTexelPointer) are handled
// exclusively by the float pass. The forward scan handles
// access chains, loads, field accesses, and image subscripts.
//
// ImageSubscript is a forward case (not a float-pass case) because it
// is never converted to a kIROp_ImageTexelPointer IR inst -- it lowers
// directly to SpvOpImageTexelPointer at emit time, so the decoration
// must land on the ImageSubscript inst itself. This mirrors how the
// HLSL `RWTexture[coord]` __ref-accessor atomic path differs from the
// GLSL imageAtomic* path that uses kIROp_ImageTexelPointer directly.
//
// Invocation / ownership
// ----------------------
// This file owns the NonUniform logic, but it does not schedule itself; the
// callers decide when each phase runs:
//
//   * Textual mode: floatNonUniformResourceIndex(module, Textual) runs as a
//     standalone SLANG_PASS from slang-emit.cpp (the IR lowering pipeline) for
//     all non-SPIR-V targets. It only repositions the wrapper; it emits no
//     decorations, and the forward scan is NOT run.
//
//   * SPIR-V mode: both phases are scheduled by the SPIR-V legalize pass
//     (SPIRVLegalizationContext::processModule in slang-ir-spirv-legalize.cpp):
//       - the float phase runs per-instruction, dispatched from the legalize
//         worklist as the kIROp_NonUniformResourceIndex handler
//         (processNonUniformResourceIndex(..., SPIRV));
//       - propagateNonUniformDecorations(module) runs once, as the FINAL step
//         of processModule.
//     The forward scan must run last (after getElement -> getElementPtr,
//     buffer-to-pointer lowering, legalizeStructBlocks, and force-inlining)
//     because it decorates the resulting emit-shape access chains / loads /
//     field accesses / image subscripts, which do not exist in final form
//     earlier. That ordering constraint is why its invocation stays in the
//     legalize pipeline even though its code lives here.

namespace Slang
{
// Walk back through resource-creating ops to find and decorate the
// access-chain index that is the source of non-uniformity.
// Terminates because the IR operand graph is acyclic (SSA dominance order).
//
// Op kinds get deliberately different treatment:
//  - Access-chain ops (GetElementPtr/GetElement, and the address inside a
//    Load): decorate only the *index* operand -- the index is the source of
//    non-uniformity, and the access chain / load itself is decorated later by
//    the forward scan in propagateNonUniformDecorations.
//  - Single-operand resource-creating ops (CombinedTextureSamplerGetTexture/
//    GetSampler, ImageTexelPointer, GetLegalizedSPIRVGlobalParamAddr): decorate
//    the produced resource value itself and recurse into its sole resource
//    operand to reach the underlying index, since the forward scan does not
//    handle these ops.
//  - MakeCombinedTextureSampler: decorate only the produced (combined) value,
//    and do NOT recurse into the texture/sampler operands -- one of them may be
//    uniform. The non-uniform operand is decorated separately at the point it
//    is unwrapped in processNonUniformResourceIndex, which is the only place
//    that knows which of the two operands the non-uniformity flowed through.
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
    else if (as<IRMakeCombinedTextureSampler>(operand))
    {
        // Decorate only the combined (OpSampledImage) result. We intentionally
        // do NOT recurse into the texture/sampler operands here: a combined
        // sampler can be built from one non-uniform and one uniform resource
        // (e.g. tex[NonUniformResourceIndex(i)].Sample(uniformSampler, ...)),
        // and recursing into both would decorate the uniform sibling. Instead,
        // the operand that the non-uniformity actually flowed through is
        // decorated at the point it is unwrapped in processNonUniformResourceIndex
        // (which is the only place that knows which operand it was).
        decorate(operand);
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

// Walk back from `operand` and attach IRSPIRVNonUniformResourceDecoration along
// the chain. Idempotent: a value that is already decorated is left untouched.
static void decorateNonUniformResourceChain(IRInst* operand)
{
    IRBuilder builder(operand);
    auto decorate = [&](IRInst* value)
    {
        if (!value->findDecoration<IRSPIRVNonUniformResourceDecoration>())
            builder.addSPIRVNonUniformResourceDecoration(value);
    };
    decorateNonUniformChain(operand, decorate);
}

void processNonUniformResourceIndex(
    IRInst* nonUniformResourceIndexInst,
    NonUniformResourceIndexFloatMode floatMode)
{
    // Float `NonUniformResourceIndex()` outward along the use-def chain, from
    // the wrapped index toward the leaf operation that consumes the resource,
    // then decorate the resulting chain. The processing switch below enumerates
    // the full set of op kinds this floats through (GetElementPtr, GetElement,
    // Load, IntCast, MakeCombinedTextureSampler, CombinedTextureSamplerGet*,
    // ImageTexelPointer, GetLegalizedSPIRVGlobalParamAddr, ...); see the
    // architecture overview at the top of this file for how the float pass and
    // the legalize forward scan divide the work.
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
                        // Track the operand the non-uniformity flowed through so
                        // we can decorate only that side below; the other
                        // operand (texture or sampler) may be uniform.
                        IRInst* nonUniformOperand = nullptr;
                        if (tex == inst)
                            nonUniformOperand = tex = inst->getOperand(0);
                        else if (samp == inst)
                            nonUniformOperand = samp = inst->getOperand(0);
                        else
                            SLANG_UNREACHABLE("NonUniformResourceIndex must be an operand of "
                                              "MakeCombinedTextureSampler");
                        newUser =
                            builder.emitMakeCombinedTextureSampler(user->getFullType(), tex, samp);

                        // Decorate the non-uniform operand's chain now, while we
                        // still know which operand it was. decorateNonUniformChain
                        // deliberately does not recurse into combined-sampler
                        // operands (that would over-decorate a uniform sibling),
                        // so this is the only place the operand chain gets
                        // decorated. Decoration is SPIR-V only.
                        if (floatMode == NonUniformResourceIndexFloatMode::SPIRV)
                            decorateNonUniformResourceChain(nonUniformOperand);
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
        // An ImageTexelPointer (from the GLSL imageAtomic* path) has a texel
        // pointer result type (e.g. Ptr<uint> in the Image address space), not
        // a resource type, so it fails the isResourceType/isPointerToResourceType
        // checks. Decorate its chain explicitly so the texel pointer, the
        // legalized global-param address, and the access-chain index all carry
        // NonUniform as required for image atomics (VUID-RuntimeSpirv-None-10148).
        if (isResourceType(type) || isPointerToResourceType(type) ||
            operand->getOp() == kIROp_ImageTexelPointer)
        {
            decorateNonUniformResourceChain(operand);
        }
        inst->replaceUsesWith(operand);
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

// Forward-scan decoration phase, phase 2 of the SPIR-V path (see the
// "Invocation / ownership" note in the file header). Complements the float
// pass: it runs once, as the FINAL step of SPIR-V legalization
// (SPIRVLegalizationContext::processModule in slang-ir-spirv-legalize.cpp),
// and decorates the post-legalization, emit-shape instructions.
//
// It must run last, after the legalizer has rewritten getElement ->
// getElementPtr, lowered structured/constant buffers to pointer access chains,
// run legalizeStructBlocks, and force-inlined wrapper functions -- those
// rewrites produce the access chains / loads / field accesses / image
// subscripts this scan decorates, which do not exist in final form earlier.
// That ordering constraint is why the *invocation* lives in the legalize
// pipeline even though the logic lives here.
//
// This handles post-rewrite propagation: bidirectional sync between
// access-chain indices and instructions, and forward propagation through
// loads, field accesses, and image subscripts. Resource-creating ops
// (MakeCombinedTextureSampler, etc.) are handled by the float pass, not here.
//
// Single forward pass suffices because IR instructions are in dominance
// order (producers before consumers).
void propagateNonUniformDecorations(IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;

        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                switch (inst->getOp())
                {
                case kIROp_GetElement:
                case kIROp_GetElementPtr:
                case kIROp_RWStructuredBufferGetElementPtr:
                    {
                        // Contract (VUID-RuntimeSpirv-None-10148): the
                        // resource operand consumed by the access (the final
                        // OpAccessChain result) must be NonUniform. So when a
                        // base, index, or inst on this chain is decorated, we
                        // decorate this inst to guarantee the consumed result
                        // carries NonUniform, and decorate the index so a
                        // non-uniform index operand is marked too. The base
                        // check propagates into inner access chains (e.g.
                        // structured-buffer element pointers) derived from a
                        // decorated outer access chain.
                        //
                        // A constant index (e.g. a fixed [0] offset) is
                        // dynamically uniform and is skipped automatically by
                        // addSPIRVNonUniformResourceDecoration -- so only the
                        // result is decorated for the constant-index case.
                        auto baseOperand = inst->getOperand(0);
                        auto indexOperand = inst->getOperand(1);
                        auto baseDecorated =
                            baseOperand->findDecoration<IRSPIRVNonUniformResourceDecoration>() !=
                            nullptr;
                        auto indexDecorated =
                            indexOperand->findDecoration<IRSPIRVNonUniformResourceDecoration>() !=
                            nullptr;
                        auto instDecorated =
                            inst->findDecoration<IRSPIRVNonUniformResourceDecoration>() != nullptr;
                        if (!baseDecorated && !indexDecorated && !instDecorated)
                            break;
                        IRBuilder builder(inst);
                        builder.setInsertBefore(inst);
                        if (!indexDecorated)
                            builder.addSPIRVNonUniformResourceDecoration(indexOperand);
                        if (!instDecorated)
                            builder.addSPIRVNonUniformResourceDecoration(inst);
                    }
                    break;
                case kIROp_FieldAddress:
                case kIROp_FieldExtract:
                    if (inst->getOperand(0)
                            ->findDecoration<IRSPIRVNonUniformResourceDecoration>() &&
                        !inst->findDecoration<IRSPIRVNonUniformResourceDecoration>())
                    {
                        IRBuilder builder(inst);
                        builder.addSPIRVNonUniformResourceDecoration(inst);
                    }
                    break;
                case kIROp_ImageSubscript:
                    // ImageSubscript lowers directly to SpvOpImageTexelPointer
                    // at emit time (it is never turned into a
                    // kIROp_ImageTexelPointer IR inst), so the NonUniform
                    // decoration required for image atomics
                    // (VUID-RuntimeSpirv-None-10148) must land on this inst.
                    // Forward-propagate from the image operand. For the array
                    // access paths this targets, processImageSubscript's load
                    // HACK has rewired operand 0 to the decorated access-chain
                    // pointer, so the operand carries NonUniform here. Note
                    // that HACK only fires when the image operand is a Load;
                    // if it is not (the HACK's stated failure mode), operand 0
                    // is left unchanged and may be undecorated, in which case
                    // this propagates nothing -- the same limitation the HACK
                    // already carries, not a new one introduced here.
                    if (inst->getOperand(0)
                            ->findDecoration<IRSPIRVNonUniformResourceDecoration>() &&
                        !inst->findDecoration<IRSPIRVNonUniformResourceDecoration>())
                    {
                        IRBuilder builder(inst);
                        builder.addSPIRVNonUniformResourceDecoration(inst);
                    }
                    break;
                case kIROp_Load:
                    if (inst->getOperand(0)
                            ->findDecoration<IRSPIRVNonUniformResourceDecoration>() &&
                        !inst->findDecoration<IRSPIRVNonUniformResourceDecoration>())
                    {
                        IRBuilder builder(inst);
                        builder.addSPIRVNonUniformResourceDecoration(inst);
                    }
                    break;
                default:
                    // MakeCombinedTextureSampler, CombinedTextureSamplerGetTexture,
                    // ImageTexelPointer, and GetLegalizedSPIRVGlobalParamAddr are
                    // intentionally absent -- handled by the float pass.
                    // SPIRVLoadTexelPointerFromHeap is also absent and is not
                    // handled anywhere: the descriptor-heap image-atomic path
                    // that would need it is rejected by E41403 (see the note
                    // in processImageSubscript), so it is currently unreachable.
                    break;
                }
            }
        }
    }
}
} // namespace Slang
