#include "slang-ir-resource-selection-diagnostics.h"

#include "slang-code-gen.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-target.h"

#include <spirv/unified1/spirv.h>

namespace Slang
{

// Resource-selection specialization introduces branches around resource operations. Consider:
//
//     Texture2D<float4> first, second;
//     SamplerState sampler;
//     float4 sampleSelectedTexture(bool condition, float2 uv)
//     {
//         Texture2D<float4> texture = condition ? first : second;
//         return texture.Sample(sampler, uv);
//     }
//
// The specialization pass moves `Sample` into resource-dependent branches and merges its ordinary
// result. When `condition` differs within a derivative group, the new control flow can make the
// implicit derivatives undefined. We warn about that tradeoff without preventing specialization.
// Explicit-LOD samples, explicit-gradient samples, and ordinary resource accesses do not require
// that warning. We classify actual IR and SPIR-V instructions and inspect direct callees; an
// unrecognized instruction or unavailable callee remains unknown rather than proving independence.

/// Whether an operation requires peer invocations to execute the same dynamic instruction.
///
/// `Independent` describes only that requirement, not freedom from memory effects or a proof that
/// arbitrary reordering or speculative execution preserves the operation's behavior.
enum class ResourceOperationControlFlowDependence
{
    Independent,
    Derivative,
    Unknown,
};

/// Return the control-flow dependence recognized for a SPIR-V assembly instruction.
static ResourceOperationControlFlowDependence getResourceOperationControlFlowDependence(
    IRSPIRVAsmInst* inst);

/// Return the control-flow dependence recognized for an ordinary IR operation.
static ResourceOperationControlFlowDependence getOrdinaryResourceOperationControlFlowDependence(
    IRInst* inst);

/// Determine whether a resource operation or any directly called function depends on participation.
static ResourceOperationControlFlowDependence calculateResourceOperationControlFlowDependence(
    CodeGenContext* codeGenContext,
    IRInst* originalConsumer);

void diagnoseResourceSelectionDispatch(CodeGenContext* codeGenContext, IRInst* originalConsumer)
{
    // We choose the diagnostic from the original operation, before specialization creates copies.
    // Both warnings describe a possible change: no quad-uniformity proof is available here.
    auto dependence =
        calculateResourceOperationControlFlowDependence(codeGenContext, originalConsumer);
    switch (dependence)
    {
    case ResourceOperationControlFlowDependence::Independent:
        return;
    case ResourceOperationControlFlowDependence::Derivative:
        codeGenContext->getSink()->diagnose(
            Diagnostics::ResourceSelectionIntroducesConditionalDerivative{
                .location = originalConsumer->sourceLoc,
            });
        return;
    case ResourceOperationControlFlowDependence::Unknown:
        codeGenContext->getSink()->diagnose(
            Diagnostics::ResourceSelectionIntroducesConditionalUnknownOperation{
                .location = originalConsumer->sourceLoc,
            });
        return;
    }
}

static ResourceOperationControlFlowDependence calculateResourceOperationControlFlowDependence(
    CodeGenContext* codeGenContext,
    IRInst* originalConsumer)
{
    // We examine the operation and the bodies of its direct callees. Each instruction enters
    // `pendingInstructions` at most once, so shared callees and recursive calls terminate without
    // treating a recursion edge as evidence that its function is independent.
    List<IRInst*> pendingInstructions;
    HashSet<IRInst*> visitedInstructions;
    pendingInstructions.add(originalConsumer);
    visitedInstructions.add(originalConsumer);
    bool hasUnknownOperation = false;

    for (Index index = 0; index < pendingInstructions.getCount(); index++)
    {
        auto inst = pendingInstructions[index];
        if (auto call = as<IRCall>(inst))
        {
            auto callee = getResolvedInstForDecorations(call->getCallee());
            auto function = as<IRFunc>(callee);
            if (!function)
            {
                // Indirect and unresolved calls can invoke code we cannot inspect.
                hasUnknownOperation = true;
                continue;
            }

            // A target intrinsic replaces the ordinary body at emission, so that body cannot
            // establish the behavior of the emitted call.
            UnownedStringSlice intrinsicDefinition;
            IRInst* intrinsicInst;
            if (findTargetIntrinsicDefinition(
                    function,
                    codeGenContext->getTargetReq()->getTargetCaps(),
                    intrinsicDefinition,
                    intrinsicInst))
            {
                hasUnknownOperation = true;
                continue;
            }
            if (!function->getFirstBlock())
            {
                // An external declaration supplies no body from which to establish independence.
                hasUnknownOperation = true;
                continue;
            }

            // A second call to an inspected function cannot reveal another instruction. This
            // also stops a recursive call before scanning its function body again.
            if (!visitedInstructions.add(function))
                continue;

            // Only executable instructions in the callee can impose a participation requirement.
            // We do not follow argument definitions, which execute before the introduced branch.
            for (auto block : function->getBlocks())
            {
                for (auto child : block->getOrdinaryInsts())
                {
                    if (visitedInstructions.add(child))
                        pendingInstructions.add(child);
                }
            }
            continue;
        }

        if (auto assembly = as<IRSPIRVAsm>(inst))
        {
            // A known derivative operation warrants the specific warning even when another
            // instruction in the same assembly block remains unknown.
            for (auto assemblyInst : assembly->getInsts())
            {
                auto dependence = getResourceOperationControlFlowDependence(assemblyInst);
                if (dependence == ResourceOperationControlFlowDependence::Derivative)
                    return dependence;
                if (dependence == ResourceOperationControlFlowDependence::Unknown)
                    hasUnknownOperation = true;
            }
            continue;
        }

        // Ordinary IR receives the same conservative classification as assembly: only recognized
        // per-invocation operations establish independence. New or unexamined operations remain
        // unknown, including wave operations, barriers, and cooperative operations.
        auto dependence = getOrdinaryResourceOperationControlFlowDependence(inst);
        if (dependence == ResourceOperationControlFlowDependence::Derivative)
            return dependence;
        if (dependence == ResourceOperationControlFlowDependence::Unknown)
            hasUnknownOperation = true;
    }

    if (hasUnknownOperation)
        return ResourceOperationControlFlowDependence::Unknown;
    return ResourceOperationControlFlowDependence::Independent;
}

static ResourceOperationControlFlowDependence getOrdinaryResourceOperationControlFlowDependence(
    IRInst* inst)
{
    // Standard-library derivatives such as `ddx` and `ddy` lower to calls or assembly, accompanied
    // by `RequireComputeDerivative`; they have no separate ordinary IR opcodes. We honor that
    // declared requirement as well as the native implicit-sampling operation.
    switch (inst->getOp())
    {
    case kIROp_RequireComputeDerivative:
    case kIROp_Sample:
        return ResourceOperationControlFlowDependence::Derivative;
    default:
        break;
    }

    // Constants, type descriptions, and debug information perform no shader computation. Atomic
    // operations act on memory but do not require peers to execute the same dynamic instruction.
    if (as<IRConstant>(inst))
        return ResourceOperationControlFlowDependence::Independent;
    if (as<IRType>(inst))
        return ResourceOperationControlFlowDependence::Independent;
    if (isDebugInfoInst(inst))
        return ResourceOperationControlFlowDependence::Independent;
    if (as<IRAtomicOperation>(inst))
        return ResourceOperationControlFlowDependence::Independent;

    // The remaining recognized operations construct per-invocation values, access memory, or
    // preserve the callee's existing control-flow graph. The list deliberately excludes calls,
    // generic assembly, and collective operations, whose requirements need separate analysis.
    // Reading resource contents does not establish independence: `Sample` reads resource contents
    // and requires derivatives. We therefore classify the exact resource operations.
    switch (inst->getOp())
    {
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_IRem:
    case kIROp_FRem:
    case kIROp_Lsh:
    case kIROp_Rsh:
    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Greater:
    case kIROp_Less:
    case kIROp_Geq:
    case kIROp_Leq:
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_And:
    case kIROp_Or:
    case kIROp_Neg:
    case kIROp_Not:
    case kIROp_BitNot:
    case kIROp_Select:
    case kIROp_BitfieldExtract:
    case kIROp_BitfieldInsert:
    case kIROp_Var:
    case kIROp_Load:
    case kIROp_Store:
    case kIROp_SwizzledStore:
    case kIROp_MatrixSwizzleStore:
    case kIROp_LoadFromUninitializedMemory:
    case kIROp_DefaultConstruct:
    case kIROp_MakeVector:
    case kIROp_MakeMatrix:
    case kIROp_MakeMatrixFromScalar:
    case kIROp_MakeVectorFromScalar:
    case kIROp_MakeStruct:
    case kIROp_MakeTuple:
    case kIROp_MakeArray:
    case kIROp_MakeArrayFromElement:
    case kIROp_MatrixReshape:
    case kIROp_VectorReshape:
    case kIROp_Swizzle:
    case kIROp_SwizzleSet:
    case kIROp_FieldExtract:
    case kIROp_FieldAddress:
    case kIROp_GetElement:
    case kIROp_GetElementPtr:
    case kIROp_GetOffsetPtr:
    case kIROp_GetTupleElement:
    case kIROp_BuiltinCast:
    case kIROp_BitCast:
    case kIROp_Reinterpret:
    case kIROp_IntCast:
    case kIROp_FloatCast:
    case kIROp_CastIntToFloat:
    case kIROp_CastFloatToInt:
    case kIROp_PtrCast:
    case kIROp_CastPtrToInt:
    case kIROp_CastIntToPtr:
    case kIROp_CastPtrToBool:
    case kIROp_CastToVoid:
    case kIROp_GlobalValueRef:
    case kIROp_MakeCombinedTextureSampler:
    case kIROp_CombinedTextureSamplerGetTexture:
    case kIROp_CombinedTextureSamplerGetSampler:
    case kIROp_SampleGrad:
    case kIROp_ImageGatherOffset:
    case kIROp_ImageLoad:
    case kIROp_ImageStore:
    case kIROp_ImageSubscript:
    case kIROp_ImageTexelPointer:
    case kIROp_SubpassLoad:
    case kIROp_GetStructuredBufferPtr:
    case kIROp_GetUntypedBufferPtr:
    case kIROp_SPIRVLoadTexelPointerFromHeap:
    case kIROp_StructuredBufferLoad:
    case kIROp_StructuredBufferLoadStatus:
    case kIROp_ByteAddressBufferLoad:
    case kIROp_ByteAddressBufferStore:
    case kIROp_RWStructuredBufferLoad:
    case kIROp_RWStructuredBufferLoadStatus:
    case kIROp_RWStructuredBufferGetElementPtr:
    case kIROp_RWStructuredBufferStore:
    case kIROp_StructuredBufferAppend:
    case kIROp_StructuredBufferConsume:
    case kIROp_StructuredBufferGetDimensions:
    case kIROp_NonUniformResourceIndex:
    case kIROp_TreatAsDynamicUniform:
    case kIROp_UnconditionalBranch:
    case kIROp_ConditionalBranch:
    case kIROp_Loop:
    case kIROp_IfElse:
    case kIROp_Switch:
    case kIROp_Return:
    case kIROp_Unreachable:
    case kIROp_MissingReturn:
    case kIROp_Discard:
    case kIROp_RequirePrelude:
    case kIROp_RequireTargetExtension:
    case kIROp_RequireMaximallyReconverges:
    case kIROp_RequireQuadDerivatives:
        return ResourceOperationControlFlowDependence::Independent;
    default:
        return ResourceOperationControlFlowDependence::Unknown;
    }
}

static ResourceOperationControlFlowDependence getResourceOperationControlFlowDependence(
    IRSPIRVAsmInst* inst)
{
    // The `__truncate` pseudo-instruction converts the sampled value to the requested shape. It
    // has no numeric SPIR-V opcode and does not inspect other invocations.
    if (inst->getOpcodeOperand()->getOp() == kIROp_SPIRVAsmOperandTruncate)
        return ResourceOperationControlFlowDependence::Independent;

    // Implicit sampling and explicit derivative instructions require participating invocations
    // from the derivative group. We recognize every core implicit-sampling variant, including
    // sparse results and projected or comparison coordinates.
    switch (inst->getOpcodeOperandWord())
    {
    case SpvOpImageSampleImplicitLod:
    case SpvOpImageSampleDrefImplicitLod:
    case SpvOpImageSampleProjImplicitLod:
    case SpvOpImageSampleProjDrefImplicitLod:
    case SpvOpImageSparseSampleImplicitLod:
    case SpvOpImageSparseSampleDrefImplicitLod:
    case SpvOpImageSparseSampleProjImplicitLod:
    case SpvOpImageSparseSampleProjDrefImplicitLod:
    case SpvOpImageQueryLod:
    case SpvOpDPdx:
    case SpvOpDPdy:
    case SpvOpFwidth:
    case SpvOpDPdxFine:
    case SpvOpDPdyFine:
    case SpvOpFwidthFine:
    case SpvOpDPdxCoarse:
    case SpvOpDPdyCoarse:
    case SpvOpFwidthCoarse:
        return ResourceOperationControlFlowDependence::Derivative;

    // These resource operations, and the declarations/conversions used around their results in
    // the standard library, do not require peers to execute the same dynamic instruction. Writes
    // remain in the selected branch; that property is not permission to speculate them.
    case SpvOpImageSampleExplicitLod:
    case SpvOpImageSampleDrefExplicitLod:
    case SpvOpImageSampleProjExplicitLod:
    case SpvOpImageSampleProjDrefExplicitLod:
    case SpvOpImageSparseSampleExplicitLod:
    case SpvOpImageSparseSampleDrefExplicitLod:
    case SpvOpImageSparseSampleProjExplicitLod:
    case SpvOpImageSparseSampleProjDrefExplicitLod:
    case SpvOpImageFetch:
    case SpvOpImageSparseFetch:
    case SpvOpImageRead:
    case SpvOpImageSparseRead:
    case SpvOpImageWrite:
    case SpvOpImageGather:
    case SpvOpImageDrefGather:
    case SpvOpImageSparseGather:
    case SpvOpImageSparseDrefGather:
    case SpvOpImageQuerySize:
    case SpvOpImageQuerySizeLod:
    case SpvOpImageQueryLevels:
    case SpvOpImageQuerySamples:
    case SpvOpImageSparseTexelsResident:
    case SpvOpImage:
    case SpvOpSampledImage:
    case SpvOpCapability:
    case SpvOpExtension:
    case SpvOpTypeStruct:
    case SpvOpCompositeExtract:
    case SpvOpCompositeConstruct:
    case SpvOpVectorShuffle:
    case SpvOpCopyObject:
    case SpvOpFConvert:
    case SpvOpSConvert:
    case SpvOpUConvert:
    case SpvOpBitcast:
    case SpvOpLoad:
    case SpvOpStore:
        return ResourceOperationControlFlowDependence::Independent;

    default:
        // Arbitrary assembly can contain subgroup operations or extension instructions whose
        // behavior depends on participation. An unknown opcode cannot prove independence.
        return ResourceOperationControlFlowDependence::Unknown;
    }
}

} // namespace Slang
