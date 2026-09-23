#include "slang-ir-use-uninitialized-values.h"

#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-reachability.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-lookup-spirv.h"
#include "slang-rich-diagnostics.h"

// This file diagnoses a read when the value reaching that read may still be uninitialized. For
// each value under examination, we perform the following steps:
//
// 1. We follow the value through SSA projections and block parameters. Separately, we follow the
//    operations through which a use can read or write the storage that holds the value. Keeping
//    those two flows separate lets us distinguish a pointer value from the storage that contains
//    that pointer.
// 2. We record three facts for each use: whether it reads the incoming value, whether it may write
//    the tracked value, and whether it counts as a definite initialization for this analysis.
// 3. We use control-flow reachability to answer two questions. If no possible write can reach a
//    read, we report an uninitialized use. Otherwise, if some path can reach the read without a
//    definite write, we report a possible uninitialized use.
//
// The module-wide API applies this analysis to `out` parameters, locals, constructor
// results, and globals. `checkForUsingUninitializedVariable` applies the same analysis to one
// variable created by a later IR pass.

namespace Slang
{

static bool isUninitializedValue(IRInst* inst)
{
    // An `IRVar` allocates storage but does not initialize its value. We therefore analyze both
    // explicit undefined values and local variables whose source declaration may lack an
    // initializer.
    return (as<IRUndefined>(inst) || (inst->m_op == kIROp_Var));
}

/// Return whether `func` implements the intrinsic that suppresses modification checks.
static bool isUnmodifying(IRFunc* func)
{
    auto intr = func->findDecoration<IRIntrinsicOpDecoration>();
    return (intr && intr->getIntrinsicOp() == kIROp_Unmodified);
}

/// Return whether `param` has the `out` contract checked by this analysis.
static bool shouldCheckParameterAsOut(IRParam* param)
{
    // An `out` parameter normally begins without a value and must be assigned before it is read or
    // returned. The mesh-shader output collection types below are instead written through their
    // elements. Their current `out` representation does not express that contract, so we exclude
    // them.
    // TODO: Represent these parameters with dedicated output types, like `OutputPatch`, so their
    // initialization contract does not require this exception.
    auto out = as<IROutParamType>(param->getFullType());
    if (!out)
        return false;

    switch (out->getValueType()->getOp())
    {
    case kIROp_VerticesType:
    case kIROp_IndicesType:
    case kIROp_PrimitivesType:
        return false;
    default:
        return true;
    }
}

/// Return whether `op` projects an SSA value from its first operand.
static bool isTrackedValueProjectionOp(IROp op)
{
    // These instructions carry a selected part of an SSA value into their result. Address
    // projections instead participate in the storage-address flow handled below.
    switch (op)
    {
    case kIROp_FieldExtract:
    case kIROp_GetElement:
        return true;
    default:
        break;
    }

    return false;
}

/// Return whether `use` supplies the SSA value projected by its user.
static bool isUseSourceOfTrackedValueProjection(IRUse* use)
{
    // Each value projection takes its source from operand zero. Its remaining operands select a
    // field or element; they do not carry the value being projected.
    auto user = use->getUser();
    return user->getOperandCount() != 0 && user->getOperandUse(0) == use &&
           isTrackedValueProjectionOp(user->getOp());
}

/// Return whether `use` selects part of a value or address projected by its user.
static bool isProjectionSelectorUse(IRUse* use)
{
    // Value projections and address derivations take their source from operand zero. Any other
    // runtime operand selects the field, element, or pointer offset to project.
    auto user = use->getUser();
    return user->getOperandCount() != 0 && user->getOperandUse(0) != use &&
           (isTrackedValueProjectionOp(user->getOp()) || isAddressInst(user));
}

/// Return whether `use` supplies a pointer from which its user may transfer storage access.
static bool mayPointerUseTransferStorageAccess(IRUse* use)
{
    // The shared predicate includes address or pointer operations and l-value implicit casts that
    // transfer at least one access direction through a temporary. We conservatively follow every
    // recognized kind. Requiring operand zero to have pointer type prevents an address formed from
    // a resource value from appearing to transfer access to the storage that held that value. The
    // caller must still distinguish a pointer-valued SSA value from an address of the tracked
    // storage.
    return as<IRPtrTypeBase>(use->get()->getDataType()) && mayUseTransferStorageAccess(use);
}

/// Return the value type whose initialization state is represented by `inst`.
static IRType* getTrackedValueType(IRInst* inst)
{
    // A local or global variable is an address whose pointee is the value under analysis. Every
    // other tracked instruction is already an SSA value, so its full type is the value type.
    if (as<IRVar>(inst) || as<IRGlobalVar>(inst))
    {
        auto addressType = as<IRPtrTypeBase>(unwrapAttributedType(inst->getFullType()));
        SLANG_RELEASE_ASSERT(addressType);
        return addressType->getValueType();
    }
    return inst->getFullType();
}

/// An `InterfaceValueDiagnosticOwner` identifies which analysis diagnoses an uninitialized
/// interface value.
enum class InterfaceValueDiagnosticOwner
{
    UninitializedValueAnalysis,
    TypeFlow,
};

/// Return whether the selected uninitialized-value check may omit values of `type`.
static bool canSkipInitializationCheck(
    IRType* type,
    InterfaceValueDiagnosticOwner interfaceValueDiagnosticOwner)
{
    // We receive a value type rather than the address type of its storage. This distinction matters
    // for pointer values: the pointer itself has runtime state even when its pointee is empty.

    // Specialization can leave a non-type value where this analysis expects a type.
    if (!type)
        return true;

    if (as<IRVoidType>(type))
        return true;

    // A struct needs this check when any field does.
    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (!canSkipInitializationCheck(field->getFieldType(), interfaceValueDiagnosticOwner))
                return false;
        }
        return true;
    }

    // Type-flow specialization already emits the more specific E50101 diagnostic when local
    // interface values reach dynamic dispatch. The module-wide function check defers interface
    // values to that analysis so one use does not receive both diagnostics. Global checking cannot
    // defer: an interface-valued global still needs E41017 when nothing supplies its initial value.
    if (interfaceValueDiagnosticOwner == InterfaceValueDiagnosticOwner::TypeFlow &&
        as<IRInterfaceType>(type))
    {
        return true;
    }

    // A type parameter has no concrete runtime representation at this point.
    if (as<IRParam>(type))
        return true;

    // A pointer value always carries an address, regardless of whether its pointee has runtime
    // state. We must not recurse into the pointee; doing so would also fail to terminate for
    // mutually recursive structures connected by pointers.
    if (as<IRPtrTypeBase>(type))
        return false;

    // A specialized type has the same initialization requirements as its resolved result.
    if (auto spec = as<IRSpecialize>(type))
    {
        IRInst* inner = getResolvedInstForDecorations(spec);
        IRType* innerType = (IRType*)(inner);
        return canSkipInitializationCheck(innerType, interfaceValueDiagnosticOwner);
    }

    return false;
}

/// Return the block parameter that receives the branch argument `argUse`, or null when `argUse` is
/// not a branch argument.
static IRParam* getBranchArgPhiParam(IRUse* argUse)
{
    // An unconditional branch or loop stores its arguments after its block operands. `getArgs()`
    // excludes those block operands, so the argument's offset in that range is also the index of
    // the receiving block parameter.
    auto branch = as<IRUnconditionalBranch>(argUse->getUser());
    if (!branch)
        return nullptr;

    auto args = branch->getArgs();
    UInt argCount = branch->getArgCount();
    UInt argIndex = UInt(argUse - args);
    if (argIndex >= argCount)
        return nullptr;

    // Well-formed IR gives the target one parameter for each argument.
    return getParamAt(branch->getTargetBlock(), argIndex);
}

/// A `TrackedValueFlow` describes how one instruction carries the tracked value.
enum class TrackedValueFlow
{
    /// The instruction is an SSA value, even when that value has pointer type.
    Value,

    /// Uses through the instruction can read or write the tracked storage.
    ///
    /// Most instructions in this flow are addresses derived from the storage root. An l-value
    /// implicit cast can instead point to a temporary, but its copy-in or copy-out still transfers
    /// the relevant effect to the original storage.
    Storage,
};

/// The instructions through which one tracked value or its storage effects can flow.
struct TrackedValueFlowSet
{
    /// Each instruction whose uses must be classified.
    List<IRInst*> instructions;

    /// The same instructions as a set, for dependency queries.
    HashSet<IRInst*> instructionSet;

    /// Instructions reached through SSA value flow.
    HashSet<IRInst*> valueInstructions;

    /// Instructions whose uses can affect the tracked storage.
    HashSet<IRInst*> storageInstructions;

    /// Storage-flow instructions whose effects may instead apply to unrelated storage.
    HashSet<IRInst*> storageFlowWithUncertainDestination;

    /// Return whether uses through `inst` can read or write the tracked storage.
    bool hasStorageFlow(IRInst* inst) const
    {
        // Each root begins in exactly one flow. Branch arguments and operations that propagate
        // pointer-based storage access preserve that flow. A value projection is the only
        // transition from storage to value flow, and it requires an aggregate source while
        // storage-flow instructions have pointer type. A well-typed instruction therefore cannot
        // occur in both sets.
        bool hasStorage = storageInstructions.contains(inst);
        bool hasValue = valueInstructions.contains(inst);
        SLANG_RELEASE_ASSERT(!(hasStorage && hasValue));
        return hasStorage;
    }

    /// Return whether every write through `inst` reaches the tracked storage.
    bool hasDefiniteStorageFlow(IRInst* inst) const
    {
        return hasStorageFlow(inst) && !storageFlowWithUncertainDestination.contains(inst);
    }
};

/// Return whether a root instruction names storage whose contents are the tracked value.
static bool isTrackedStorageRoot(IRInst* inst)
{
    // Local and global variables and address instructions denote storage directly. A parameter with
    // a directional type also names storage passed by reference. A parameter without such a wrapper
    // remains an SSA value even when that value has pointer type.
    if (as<IRVar>(inst) || as<IRGlobalVar>(inst) || isAddressInst(inst))
        return true;

    auto param = as<IRParam>(inst);
    if (!param)
        return false;

    auto type = param->getDataType();
    return as<IROutParamTypeBase>(type) || as<IRRefParamType>(type) ||
           as<IRBorrowInParamType>(type);
}

static bool maySPIRVAssemblyResultReferToAddress(IRSPIRVAsm* assembly, IRInst* address);
static bool doesSPIRVAssemblyResultDefinitelyDenoteCompleteAddress(
    IRSPIRVAsm* assembly,
    IRInst* address);

/// Collect the instructions whose uses can read or write the value represented by `inst`.
///
/// We distinguish storage flow from an SSA value that happens to have pointer type. Casting or
/// storing a storage-flow address uses the address, while doing the same to an SSA pointer reads
/// that pointer value. Value projections preserve SSA value flow. Address derivations and l-value
/// implicit casts preserve storage effects only when their source already has storage flow. Branch
/// arguments preserve whichever form reaches their block parameter.
static void collectTrackedValueFlowRec(
    IRInst* inst,
    TrackedValueFlow flow,
    HashSet<IRInst*>& visitedAsStorage,
    HashSet<IRInst*>& visitedAsValue,
    TrackedValueFlowSet& result)
{
    auto visited = flow == TrackedValueFlow::Value ? &visitedAsValue : &visitedAsStorage;
    if (!visited->add(inst))
        return;

    if (result.instructionSet.add(inst))
        result.instructions.add(inst);
    if (flow == TrackedValueFlow::Value)
        result.valueInstructions.add(inst);
    else
        result.storageInstructions.add(inst);

    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        IRInst* user = use->getUser();

        // We ignore type queries because they do not observe the runtime value carried by this
        // operand.
        if (doesInstOnlyDependOnOperandTypes(user))
            continue;

        // A value projection carries part of the tracked SSA value into its result.
        if (isUseSourceOfTrackedValueProjection(use))
        {
            collectTrackedValueFlowRec(
                user,
                TrackedValueFlow::Value,
                visitedAsStorage,
                visitedAsValue,
                result);
            continue;
        }

        // An operation can transfer storage effects only when its source already has storage flow.
        // Applying the same operation to a pointer-valued SSA value instead reads that value, so
        // classification stops at the transfer instruction.
        if (flow != TrackedValueFlow::Value && mayPointerUseTransferStorageAccess(use))
        {
            collectTrackedValueFlowRec(user, flow, visitedAsStorage, visitedAsValue, result);
            continue;
        }

        // An inline SPIR-V expression can return an address that it derives from this storage. We
        // follow that result so that an ordinary load or store through the returned pointer remains
        // visible to the analysis.
        if (flow == TrackedValueFlow::Storage)
        {
            if (auto operand = as<IRSPIRVAsmOperandInst>(user))
            {
                auto assembly = operand->getAsmBlock();
                if (maySPIRVAssemblyResultReferToAddress(assembly, inst))
                {
                    collectTrackedValueFlowRec(
                        assembly,
                        flow,
                        visitedAsStorage,
                        visitedAsValue,
                        result);
                    continue;
                }
            }
        }

        // A branch argument becomes the corresponding block parameter along that edge.
        if (auto phiParam = getBranchArgPhiParam(use))
        {
            collectTrackedValueFlowRec(phiParam, flow, visitedAsStorage, visitedAsValue, result);
        }
    }
}

/// Return whether every input to `inst` has definite storage flow from the tracked root.
static bool hasOnlyDefiniteStorageFlowInputs(
    IRInst* inst,
    IRInst* root,
    const HashSet<IRInst*>& definiteStorageFlow)
{
    // The root establishes the storage identity from which the proof begins.
    if (inst == root)
        return true;

    // A storage-flow block parameter is definite only when every predecessor supplies definite
    // storage flow at the corresponding argument index. Starting the surrounding fixed point
    // optimistically lets a loop-header parameter remain definite when a back edge passes that
    // same parameter. An unrelated address on any edge still removes the parameter from the set.
    if (auto param = as<IRParam>(inst))
    {
        auto block = as<IRBlock>(param->getParent());
        if (!block)
            return false;

        Index paramIndex = getParamIndexInBlock(param);
        if (paramIndex < 0)
            return false;

        bool hasPredecessor = false;
        for (auto predecessor : block->getPredecessors())
        {
            hasPredecessor = true;
            auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
            if (!branch || UInt(paramIndex) >= branch->getArgCount() ||
                !definiteStorageFlow.contains(branch->getArg(UInt(paramIndex))))
            {
                return false;
            }
        }
        return hasPredecessor;
    }

    // An inline SPIR-V expression may return an address derived from any of its operands. It
    // definitely denotes the complete tracked value when one definite input alone accounts for
    // every possible address carried by the result.
    if (auto assembly = as<IRSPIRVAsm>(inst))
    {
        for (auto candidate : definiteStorageFlow)
        {
            if (doesSPIRVAssemblyResultDefinitelyDenoteCompleteAddress(assembly, candidate))
                return true;
        }
        return false;
    }

    // Every other storage-flow instruction either derives an address from operand zero or can
    // transfer a write from its result back to operand zero. A nonzero or unknown pointer offset
    // may instead reach a neighboring object, so only a literal zero preserves definite flow. For
    // every other recognized operation, a complete write through the result definitely reaches
    // the tracked value while the source has definite storage flow. This proof remains
    // projection-insensitive for fields and elements, as documented by `InstructionUseEffect`.
    if (inst->getOperandCount() == 0)
        return false;
    if (inst->getOp() == kIROp_GetOffsetPtr)
    {
        auto offset = as<IRIntLit>(inst->getOperand(1));
        if (!offset || offset->getValue() != 0)
            return false;
    }
    auto sourceUse = inst->getOperandUse(0);
    return mayPointerUseTransferStorageAccess(sourceUse) &&
           definiteStorageFlow.contains(sourceUse->get());
}

/// Separate definite storage flow from instructions whose effects may reach unrelated storage.
static void classifyStorageFlowCertainty(IRInst* root, TrackedValueFlowSet& flow)
{
    // We compute the greatest fixed point of the definite-storage-flow rules. Every storage-flow
    // candidate begins in the set. We then remove a block parameter when any incoming edge is
    // outside the set, and remove a storage-access transfer when its source has been removed.
    // Repeating that process carries one uncertain merge through later block parameters and
    // operations. A loop whose entry and back edges all carry storage flow from the root remains in
    // the set.
    HashSet<IRInst*> definiteStorageFlow;
    for (auto inst : flow.instructions)
    {
        if (flow.storageInstructions.contains(inst) && !flow.valueInstructions.contains(inst))
        {
            definiteStorageFlow.add(inst);
        }
    }

    for (;;)
    {
        List<IRInst*> instructionsToRemove;
        for (auto inst : flow.instructions)
        {
            if (!definiteStorageFlow.contains(inst))
                continue;
            if (!hasOnlyDefiniteStorageFlowInputs(inst, root, definiteStorageFlow))
                instructionsToRemove.add(inst);
        }

        if (instructionsToRemove.getCount() == 0)
            break;
        for (auto inst : instructionsToRemove)
            definiteStorageFlow.remove(inst);
    }

    flow.storageFlowWithUncertainDestination.clear();
    for (auto inst : flow.instructions)
    {
        if (flow.storageInstructions.contains(inst) && !definiteStorageFlow.contains(inst))
            flow.storageFlowWithUncertainDestination.add(inst);
    }
}

/// Collect value and storage flow that can be followed without analyzing memory contents.
static TrackedValueFlowSet getTrackedValueFlow(IRInst* inst)
{
    // We first classify the root from its IR role, then follow SSA projections, pointer-based
    // storage-access transfers and branch arguments while preserving the distinction between
    // value and storage flow. When the root is storage, a fixed-point proof then distinguishes
    // block parameters whose every input carries effects from that root from parameters that can
    // also receive unrelated storage.
    //
    // We cannot follow an address after code stores it in memory and later loads it. A sound
    // extension would need reaching-definition and points-to information: an address recovered
    // from memory might refer to the tracked storage, but it does not necessarily refer to it.
    // Treating that possible connection as definite could incorrectly let a store initialize the
    // tracked variable.
    // TODO: Preserve possible and definite storage flow when following addresses through memory.
    auto rootFlow =
        isTrackedStorageRoot(inst) ? TrackedValueFlow::Storage : TrackedValueFlow::Value;
    HashSet<IRInst*> visitedAsStorage;
    HashSet<IRInst*> visitedAsValue;
    TrackedValueFlowSet result;
    collectTrackedValueFlowRec(inst, rootFlow, visitedAsStorage, visitedAsValue, result);
    if (rootFlow == TrackedValueFlow::Storage)
        classifyStorageFlowCertainty(inst, result);
    return result;
}

/// Return whether `inst` depends transitively on a value in `flowSet`.
static bool dependsOnTrackedFlow(
    IRInst* inst,
    const HashSet<IRInst*>& flowSet,
    HashSet<IRInst*>& seen)
{
    // We use this test to distinguish a new value entering a block parameter from an expression
    // that merely carries the tracked value around a loop. For example, `add(total, item)` depends
    // on the loop's tracked value for `total`, while a value computed without using `total` does
    // not. The `seen` set terminates cycles between block parameters.
    if (flowSet.contains(inst))
        return true;
    if (!seen.add(inst))
        return false;

    // We inspect block-parameter arguments through predecessor branches. A parameter absent from
    // `flowSet` therefore represents an independent value for this operand walk.
    if (as<IRParam>(inst))
        return false;
    for (UInt i = 0, n = inst->getOperandCount(); i < n; i++)
    {
        auto operand = inst->getOperand(i);
        if (operand && dependsOnTrackedFlow(operand, flowSet, seen))
            return true;
    }
    return false;
}

/// Record CFG edges that introduce an initialized SSA value at a block parameter.
//
// Consider a value that is assigned on one path and then carried through a loop:
//
//     MyAttrs attrs;                       // uninitialized
//     for (;;) { ...; if (cond) attrs = computed; ... }
//     use(attrs);                          // reads the loop-header phi
//
// The loop-header parameter merges the undefined pre-loop value with `computed`. No `IRStore`
// represents the assignment after SSA construction, so we record the incoming CFG edge as a write.
// A loop-carried expression such as `add(total1, a[i])` still depends on the tracked value and is
// not an independent initialized value.
//
// This rule is exact only for an unprojected SSA value. A storage-address block parameter merges
// addresses rather than new values, so the separate storage-flow proof handles its incoming edges.
// The current value-flow model also does not retain the path from a projection back to the complete
// tracked value, so an independent incoming value may initialize only one subobject.
// TODO: Track projection paths before treating a phi edge as a definite write to the complete
// value.
static void collectPhiMergeWrites(const TrackedValueFlowSet& flow, List<IRInst*>& writes)
{
    for (auto inst : flow.instructions)
    {
        auto param = as<IRParam>(inst);
        if (!param || !flow.valueInstructions.contains(param) ||
            flow.storageInstructions.contains(param))
            continue;
        auto block = as<IRBlock>(param->getParent());
        if (!block)
            continue;
        Index paramIndex = getParamIndexInBlock(param);
        if (paramIndex < 0)
            continue;

        for (auto pred : block->getPredecessors())
        {
            auto branch = as<IRUnconditionalBranch>(pred->getTerminator());
            if (!branch || UInt(paramIndex) >= branch->getArgCount())
                continue;
            auto arg = branch->getArg(UInt(paramIndex));
            // The model treats an argument independent of the tracked value flow as a definition
            // on this edge. This conclusion is exact for an unprojected SSA value but remains
            // approximate for the value projections described above.
            HashSet<IRInst*> seen;
            if (dependsOnTrackedFlow(arg, flow.instructionSet, seen))
                continue;

            // For an SSA value, the assigned value reaches the block parameter only when this edge
            // is taken. We therefore record the write at the branch rather than at the earlier
            // point where the argument was computed.
            writes.add(branch);
        }
    }
}

/// An `InstructionUseEffect` records how one operand use affects the tracked value.
struct InstructionUseEffect
{
    /// Whether the instruction reads the tracked value.
    bool readsValue = false;

    /// Whether the instruction may update the value on at least one execution.
    bool mayWriteValue = false;

    /// Whether this checker should treat the instruction as a definite initialization.
    ///
    /// The default inference is projection-insensitive: a store through definite storage flow
    /// counts as a complete assignment. A caller that knows whether the use applies to the complete
    /// tracked value or only a subobject can override that approximation for one exact use.
    bool definitelyWritesValue = false;

    /// The nested instruction that performs the effect, or null when the operand's user does.
    ///
    /// Inline SPIR-V assembly represents each SPIR-V operation as a child of one executable
    /// `IRSPIRVAsm`. Retaining that child lets the analysis preserve the order of a load and store
    /// inside the assembly expression. Diagnostics still use the enclosing executable instruction.
    IRInst* effectInstruction = nullptr;
};

/// Return whether `inst` is a compiler-generated load with no source location whose results are
/// used only by debug metadata.
static bool isSyntheticDebugValueLoad(IRInst* inst)
{
    // `insertDebugValueStore` inserts such a load after an operation that may write through an
    // address. The load lets a debugger display the new value, but it is not a read in the source
    // program. We ignore the load only when it has no source location and every result use is debug
    // metadata. Requiring both conditions prevents us from suppressing a source expression merely
    // because its result survives only in debug metadata.
    if (!as<IRLoad>(inst) || inst->sourceLoc.isValid() || !inst->firstUse)
        return false;
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        if (!isDebugInfoInst(use->getUser()))
            return false;
    }
    return true;
}

/// Return how a call uses the argument identified by `argumentUse`.
static InstructionUseEffect inferCallUseEffect(
    IRCall* call,
    IRUse* argumentUse,
    bool argumentHasStorageFlow)
{
    // We use the corresponding parameter's direction. An `out` parameter assigns the complete
    // value stored at the supplied address when the call returns. An `inout` or `ref` parameter
    // reads that value and may assign it. A borrowed input or value parameter only reads its
    // argument. A raw pointer may read or write its pointee. This analysis no longer knows whether
    // a projected address denotes the complete tracked variable, so a caller that knows the
    // address denotes only a subobject must override the inferred effect. When the callee type
    // provides no corresponding parameter, we classify a storage argument as both a read and a
    // possible write. Passing an SSA value only reads that value; a callee can modify its pointee
    // but cannot replace the pointer value itself.

    auto parameterType = findCallArgumentParameterType(call, argumentUse);
    if (!parameterType)
    {
        return argumentHasStorageFlow
                   ? InstructionUseEffect{.readsValue = true, .mayWriteValue = true}
                   : InstructionUseEffect{.readsValue = true};
    }

    if (as<IROutParamType>(parameterType))
    {
        return argumentHasStorageFlow
                   ? InstructionUseEffect{.mayWriteValue = true, .definitelyWritesValue = true}
                   : InstructionUseEffect{.readsValue = true};
    }
    if (as<IRBorrowInOutParamType>(parameterType) || as<IRRefParamType>(parameterType))
    {
        return argumentHasStorageFlow
                   ? InstructionUseEffect{.readsValue = true, .mayWriteValue = true}
                   : InstructionUseEffect{.readsValue = true};
    }
    if (as<IRBorrowInParamType>(parameterType))
    {
        // A borrowed input is read-only. The callee can read the supplied storage, but it cannot
        // use the borrow to initialize that storage.
        return {.readsValue = true};
    }
    if (as<IRPtrTypeBase>(parameterType))
    {
        // A raw pointer parameter has no direction contract. The callee may read or write its
        // pointee, but the call alone does not prove that every execution writes it. Passing a
        // tracked pointer value reads that value; a pointee write does not replace the pointer.
        return argumentHasStorageFlow
                   ? InstructionUseEffect{.readsValue = true, .mayWriteValue = true}
                   : InstructionUseEffect{.readsValue = true};
    }
    return {.readsValue = true};
}

/// Return whether `opcode` ends the current SPIR-V basic block.
static bool isSPIRVBlockTerminator(SpvWord opcode)
{
    // The main compiler does not link against the private SPIRV-Tools opcode helper, so we keep the
    // corresponding terminator list next to the only analysis that needs it.
    switch (opcode)
    {
    case SpvOpBranch:
    case SpvOpBranchConditional:
    case SpvOpSwitch:
    case SpvOpKill:
    case SpvOpReturn:
    case SpvOpReturnValue:
    case SpvOpUnreachable:
    case SpvOpTerminateInvocation:
    case SpvOpTerminateRayKHR:
    case SpvOpIgnoreIntersectionKHR:
    case SpvOpEmitMeshTasksEXT:
    case SpvOpAbortKHR:
    case SpvOpTerminateRayNV:
    case SpvOpIgnoreIntersectionNV:
        return true;
    default:
        return false;
    }
}

/// Return whether `asmInst` has a numeric SPIR-V opcode, and write that opcode to `result`.
static bool tryGetSPIRVOpcodeWord(IRSPIRVAsmInst* asmInst, SpvWord& result)
{
    // `__truncate` is a pseudo-instruction rather than a SPIR-V opcode, and its opcode operand has
    // no numeric value. Every other accepted opcode operand has a value that
    // `getOpcodeOperandWord` can read.
    if (asmInst->getOpcodeOperand()->getOp() == kIROp_SPIRVAsmOperandTruncate)
        return false;

    result = asmInst->getOpcodeOperandWord();
    return true;
}

/// Return whether `asmInst` is in the entry block of its inline SPIR-V assembly expression.
static bool isInEntryBlockOfSPIRVAssembly(IRSPIRVAsmInst* asmInst)
{
    // `IRSPIRVAsm` is one executable Slang IR instruction, but its children can introduce several
    // SPIR-V basic blocks. Children before the first block terminator or `OpLabel` execute whenever
    // the enclosing instruction executes. A child after that boundary may be reached only on some
    // internal control-flow paths. We stop at `asmInst`, so a terminator that follows it does not
    // weaken its effect.
    auto asmBlock = cast<IRSPIRVAsm>(asmInst->getParent());
    for (auto currentInst : asmBlock->getInsts())
    {
        if (currentInst == asmInst)
            return true;

        SpvWord opcode;
        if (!tryGetSPIRVOpcodeWord(currentInst, opcode))
            continue;
        if (opcode == SpvOpLabel || isSPIRVBlockTerminator(opcode))
            return false;
    }

    SLANG_UNREACHABLE("SPIR-V assembly instruction is not a child of its reported parent");
}

/// A `SPIRVAtomicMemoryEffect` describes how one SPIR-V atomic operation accesses the memory named
/// by its pointer operand.
enum class SPIRVAtomicMemoryEffect
{
    Read,
    Write,
    ReadAndWrite,
    ReadAndConditionalWrite,
};

/// Write `opcode`'s memory effect to `effect` and return true, or return false when `opcode` is not
/// a SPIR-V atomic opcode.
static bool tryGetSPIRVAtomicMemoryEffect(SpvWord opcode, SPIRVAtomicMemoryEffect& effect)
{
    // Every SPIR-V atomic opcode has one pointer operand. Result-producing atomics place it after
    // the result type and result ID, while the two atomics with no result place it first. We
    // classify all atomic opcodes in the unified SPIR-V header so that an inline assembly address
    // receives the same read/write treatment as a native Slang atomic operation.
    switch (opcode)
    {
    case SpvOpAtomicLoad:
        effect = SPIRVAtomicMemoryEffect::Read;
        return true;

    case SpvOpAtomicStore:
    case SpvOpAtomicFlagClear:
        effect = SPIRVAtomicMemoryEffect::Write;
        return true;

    case SpvOpAtomicCompareExchange:
    case SpvOpAtomicCompareExchangeWeak:
        effect = SPIRVAtomicMemoryEffect::ReadAndConditionalWrite;
        return true;

    case SpvOpAtomicExchange:
    case SpvOpAtomicIIncrement:
    case SpvOpAtomicIDecrement:
    case SpvOpAtomicIAdd:
    case SpvOpAtomicISub:
    case SpvOpAtomicSMin:
    case SpvOpAtomicUMin:
    case SpvOpAtomicSMax:
    case SpvOpAtomicUMax:
    case SpvOpAtomicAnd:
    case SpvOpAtomicOr:
    case SpvOpAtomicXor:
    case SpvOpAtomicFlagTestAndSet:
    case SpvOpAtomicFMinEXT:
    case SpvOpAtomicFMaxEXT:
    case SpvOpAtomicFAddEXT:
        effect = SPIRVAtomicMemoryEffect::ReadAndWrite;
        return true;

    default:
        return false;
    }
}

/// The effect that one inline-SPIR-V operand has on the storage supplied through that operand.
struct SPIRVAddressOperandEffect
{
    bool readsValue = false;
    bool mayWriteValue = false;
    bool definitelyWritesCompleteValue = false;
};

/// The storage-access role assigned to one address operand of a SPIR-V instruction.
enum class SPIRVAddressOperandRole
{
    Read,
    PossibleWrite,
    ReadAndPossibleWrite,
    DefiniteWrite,
    AddressOnly,
};

/// Set `effect` when `operandIndex` identifies `expectedOperandIndex` with the given `role`.
static bool trySetSPIRVAddressOperandEffect(
    UInt operandIndex,
    UInt expectedOperandIndex,
    SPIRVAddressOperandRole role,
    SPIRVAddressOperandEffect& effect)
{
    // The opcode cases below use this helper to keep each numeric operand position next to its
    // semantic role. An address-only role is still recognized, but contributes no read or write.
    if (operandIndex != expectedOperandIndex)
        return false;

    effect.readsValue = role == SPIRVAddressOperandRole::Read ||
                        role == SPIRVAddressOperandRole::ReadAndPossibleWrite;
    effect.mayWriteValue = role == SPIRVAddressOperandRole::PossibleWrite ||
                           role == SPIRVAddressOperandRole::ReadAndPossibleWrite ||
                           role == SPIRVAddressOperandRole::DefiniteWrite;
    effect.definitelyWritesCompleteValue = role == SPIRVAddressOperandRole::DefiniteWrite;
    return true;
}

/// Return whether `operandIndex` uses a pointer value without dereferencing it.
static bool tryGetSPIRVPointerValueOperandEffect(
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // We classify operations that inspect, compare, or retain a pointer value without accessing
    // its pointee. Recording these operands as address-only prevents the conservative fallback
    // from treating a passed or stored pointer value as a possible write through the pointer.
    switch (opcode)
    {
    case SpvOpExtInstWithForwardRefsKHR:
        // SPIR-V permits this opcode only with a non-semantic instruction set. Its trailing
        // operands therefore cannot access the pointee of a pointer value.
        if (operandIndex >= 5)
        {
            effect = {};
            return true;
        }
        return false;

    case SpvOpGenericPtrMemSemantics:
    case SpvOpConvertPtrToU:
    case SpvOpSizeOf:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpPtrEqual:
    case SpvOpPtrNotEqual:
    case SpvOpPtrDiff:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                3,
                SPIRVAddressOperandRole::AddressOnly,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            4,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpLifetimeStart:
    case SpvOpLifetimeStop:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpArrayLength:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpUntypedArrayLengthKHR:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            4,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpStore:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            2,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpReturnValue:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpVariable:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            4,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpUntypedVariableKHR:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            5,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    default:
        return false;
    }
}

/// Return whether `operandIndex` has a known address-operand role in a cataloged SPIR-V
/// memory-extension operation.
static bool tryGetSPIRVMemoryExtensionOperandEffect(
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // We list the address operands of cooperative matrix and vector operations, subgroup block
    // operations, predicated and masked I/O, asynchronous copies, prefetches, and memory restores.
    // These operations may access only part of the tracked value, or may perform a write only when
    // a predicate is true. We therefore classify every write as possible rather than definite.
    switch (opcode)
    {
    case SpvOpCooperativeMatrixPerElementOpEXT:
        // The callback receives every operand from index five onward. If one of those operands is
        // a pointer capture, the callback may both read and write its pointee because the SPIR-V
        // grammar gives the captured operand no direction contract.
        if (operandIndex >= 5)
        {
            effect = {.readsValue = true, .mayWriteValue = true};
            return true;
        }
        return false;

    case SpvOpSubgroupBlockReadINTEL:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpSubgroupBlockWriteINTEL:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpPredicatedLoadINTEL:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpPredicatedStoreINTEL:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpCooperativeMatrixLoadKHR:
    case SpvOpCooperativeVectorLoadNV:
    case SpvOpCooperativeMatrixLoadNV:
    case SpvOpCooperativeMatrixLoadTensorNV:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpCooperativeMatrixStoreKHR:
    case SpvOpCooperativeVectorStoreNV:
    case SpvOpCooperativeMatrixStoreNV:
    case SpvOpCooperativeMatrixStoreTensorNV:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpCooperativeVectorMatrixMulNV:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            5,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpCooperativeVectorMatrixMulAddNV:
        if (trySetSPIRVAddressOperandEffect(operandIndex, 5, SPIRVAddressOperandRole::Read, effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            8,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpCooperativeVectorOuterProductAccumulateNV:
    case SpvOpCooperativeVectorReduceSumAccumulateNV:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpSubgroup2DBlockLoadINTEL:
    case SpvOpSubgroup2DBlockLoadTransformINTEL:
    case SpvOpSubgroup2DBlockLoadTransposeINTEL:
        if (trySetSPIRVAddressOperandEffect(operandIndex, 5, SPIRVAddressOperandRole::Read, effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            10,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpSubgroup2DBlockStoreINTEL:
        if (trySetSPIRVAddressOperandEffect(operandIndex, 5, SPIRVAddressOperandRole::Read, effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            6,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpSubgroup2DBlockPrefetchINTEL:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            5,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpMaskedGatherINTEL:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpMaskedScatterINTEL:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            2,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpGroupAsyncCopy:
    case SpvOpUntypedGroupAsyncCopyKHR:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                4,
                SPIRVAddressOperandRole::PossibleWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            5,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpUntypedPrefetchKHR:
    case SpvOpSubgroupBlockPrefetchINTEL:
    case SpvOpRestoreMemoryINTEL:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    default:
        return false;
    }
}

/// Return whether `operandIndex` accesses storage through a task or kernel operation.
static bool tryGetSPIRVTaskOrKernelOperandEffect(
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // We classify the storage operands used by node-payload, pipe, event, enqueue, and device-side
    // task operations. `OpAsmCallINTEL` and `OpTaskSequenceAsyncALTERA` pass operands to separately
    // described code without a direction contract, so we classify each such operand as both read
    // and possibly written.
    switch (opcode)
    {
    case SpvOpEnqueueNodePayloadsAMDX:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpFinishWritingNodePayloadAMDX:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpNodePayloadArrayLengthAMDX:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::AddressOnly,
            effect);

    case SpvOpAsmCallINTEL:
        if (operandIndex >= 4)
        {
            effect = {.readsValue = true, .mayWriteValue = true};
            return true;
        }
        return false;

    case SpvOpTaskSequenceAsyncALTERA:
        if (operandIndex >= 2)
        {
            effect = {.readsValue = true, .mayWriteValue = true};
            return true;
        }
        return false;

    case SpvOpReadPipe:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            4,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpWritePipe:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            4,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpReservedReadPipe:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            6,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpReservedWritePipe:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            6,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpReadPipeBlockingALTERA:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            2,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpWritePipeBlockingALTERA:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            2,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpGroupWaitEvents:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpEnqueueMarker:
        if (trySetSPIRVAddressOperandEffect(operandIndex, 5, SPIRVAddressOperandRole::Read, effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            6,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    case SpvOpEnqueueKernel:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                7,
                SPIRVAddressOperandRole::Read,
                effect) ||
            trySetSPIRVAddressOperandEffect(
                operandIndex,
                8,
                SPIRVAddressOperandRole::PossibleWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            10,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpGetKernelNDrangeSubGroupCount:
    case SpvOpGetKernelNDrangeMaxSubGroupSize:
    case SpvOpGetKernelLocalSizeForSubgroupCount:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            5,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpGetKernelWorkGroupSize:
    case SpvOpGetKernelPreferredWorkGroupSizeMultiple:
    case SpvOpGetKernelMaxNumSubgroups:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            4,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpCaptureEventProfilingInfo:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::PossibleWrite,
            effect);

    default:
        return false;
    }
}

/// Return whether `operandIndex` reads or updates a ray-query object.
static bool tryGetSPIRVRayQueryOperandEffect(
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // Slang passes a ray-query object to inline SPIR-V by address even though the SPIR-V grammar
    // describes an opaque object. We therefore classify every ray-query operand that observes or
    // changes that object, just as we classify explicit pointer operands in other instructions.
    switch (opcode)
    {

    case SpvOpRayQueryInitializeKHR:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::DefiniteWrite,
            effect);

    case SpvOpRayQueryTerminateKHR:
    case SpvOpRayQueryGenerateIntersectionKHR:
    case SpvOpRayQueryConfirmIntersectionKHR:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpRayQueryProceedKHR:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpRayQueryGetIntersectionTypeKHR:
    case SpvOpRayQueryGetIntersectionTriangleVertexPositionsKHR:
    case SpvOpRayQueryGetIntersectionClusterIdNV:
    case SpvOpRayQueryGetIntersectionSpherePositionNV:
    case SpvOpRayQueryGetIntersectionSphereRadiusNV:
    case SpvOpRayQueryGetIntersectionLSSPositionsNV:
    case SpvOpRayQueryGetIntersectionLSSRadiiNV:
    case SpvOpRayQueryGetIntersectionLSSHitValueNV:
    case SpvOpRayQueryIsSphereHitNV:
    case SpvOpRayQueryIsLSSHitNV:
    case SpvOpRayQueryGetRayTMinKHR:
    case SpvOpRayQueryGetRayFlagsKHR:
    case SpvOpRayQueryGetIntersectionTKHR:
    case SpvOpRayQueryGetIntersectionInstanceCustomIndexKHR:
    case SpvOpRayQueryGetIntersectionInstanceIdKHR:
    case SpvOpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR:
    case SpvOpRayQueryGetIntersectionGeometryIndexKHR:
    case SpvOpRayQueryGetIntersectionPrimitiveIndexKHR:
    case SpvOpRayQueryGetIntersectionBarycentricsKHR:
    case SpvOpRayQueryGetIntersectionFrontFaceKHR:
    case SpvOpRayQueryGetIntersectionCandidateAABBOpaqueKHR:
    case SpvOpRayQueryGetIntersectionObjectRayDirectionKHR:
    case SpvOpRayQueryGetIntersectionObjectRayOriginKHR:
    case SpvOpRayQueryGetWorldRayDirectionKHR:
    case SpvOpRayQueryGetWorldRayOriginKHR:
    case SpvOpRayQueryGetIntersectionObjectToWorldKHR:
    case SpvOpRayQueryGetIntersectionWorldToObjectKHR:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::Read,
            effect);

    default:
        return false;
    }
}

/// Return whether `operandIndex` is read or written while constructing a hit-object value.
static bool tryGetSPIRVHitObjectConstructionOperandEffect(
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // Slang passes hit objects to inline SPIR-V by address even though SPIR-V describes them as
    // opaque objects. We mark the destination hit object as a complete write and separately mark
    // any ray-query, hit-attribute, or payload storage that the construction operation reads or
    // updates.
    switch (opcode)
    {

    case SpvOpHitObjectRecordEmptyNV:
    case SpvOpHitObjectRecordMissNV:
    case SpvOpHitObjectRecordMissMotionNV:
    case SpvOpHitObjectRecordMissEXT:
    case SpvOpHitObjectRecordMissMotionEXT:
    case SpvOpHitObjectRecordEmptyEXT:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::DefiniteWrite,
            effect);

    case SpvOpHitObjectRecordHitMotionNV:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                1,
                SPIRVAddressOperandRole::DefiniteWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            14,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpHitObjectRecordHitWithIndexMotionNV:
    case SpvOpHitObjectRecordHitNV:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                1,
                SPIRVAddressOperandRole::DefiniteWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            13,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpHitObjectRecordHitWithIndexNV:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                1,
                SPIRVAddressOperandRole::DefiniteWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            12,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpHitObjectTraceRayMotionNV:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                1,
                SPIRVAddressOperandRole::DefiniteWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            13,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpHitObjectTraceRayNV:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                1,
                SPIRVAddressOperandRole::DefiniteWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            12,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpHitObjectRecordFromQueryEXT:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                1,
                SPIRVAddressOperandRole::DefiniteWrite,
                effect))
        {
            return true;
        }
        if (trySetSPIRVAddressOperandEffect(operandIndex, 2, SPIRVAddressOperandRole::Read, effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            4,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpHitObjectTraceReorderExecuteEXT:
    case SpvOpHitObjectTraceRayEXT:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                1,
                SPIRVAddressOperandRole::DefiniteWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            12,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpHitObjectTraceMotionReorderExecuteEXT:
    case SpvOpHitObjectTraceRayMotionEXT:
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                1,
                SPIRVAddressOperandRole::DefiniteWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            13,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    default:
        return false;
    }
}

/// Return whether `operandIndex` accesses storage while querying a hit-object value.
static bool tryGetSPIRVHitObjectObservationOperandEffect(
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // We classify the hit object itself as an input to every query. The attribute and triangle
    // queries also write a complete result through a separate output operand, so we record both
    // roles for those instructions.
    switch (opcode)
    {

    case SpvOpHitObjectGetWorldToObjectNV:
    case SpvOpHitObjectGetObjectToWorldNV:
    case SpvOpHitObjectGetObjectRayDirectionNV:
    case SpvOpHitObjectGetObjectRayOriginNV:
    case SpvOpHitObjectGetShaderRecordBufferHandleNV:
    case SpvOpHitObjectGetShaderBindingTableRecordIndexNV:
    case SpvOpHitObjectGetCurrentTimeNV:
    case SpvOpHitObjectGetHitKindNV:
    case SpvOpHitObjectGetPrimitiveIndexNV:
    case SpvOpHitObjectGetGeometryIndexNV:
    case SpvOpHitObjectGetInstanceIdNV:
    case SpvOpHitObjectGetInstanceCustomIndexNV:
    case SpvOpHitObjectGetWorldRayDirectionNV:
    case SpvOpHitObjectGetWorldRayOriginNV:
    case SpvOpHitObjectGetRayTMaxNV:
    case SpvOpHitObjectGetRayTMinNV:
    case SpvOpHitObjectIsEmptyNV:
    case SpvOpHitObjectIsHitNV:
    case SpvOpHitObjectIsMissNV:
    case SpvOpHitObjectGetClusterIdNV:
    case SpvOpHitObjectGetSpherePositionNV:
    case SpvOpHitObjectGetSphereRadiusNV:
    case SpvOpHitObjectGetLSSPositionsNV:
    case SpvOpHitObjectGetLSSRadiiNV:
    case SpvOpHitObjectIsSphereHitNV:
    case SpvOpHitObjectIsLSSHitNV:
    case SpvOpHitObjectGetRayFlagsEXT:
    case SpvOpHitObjectGetCurrentTimeEXT:
    case SpvOpHitObjectGetHitKindEXT:
    case SpvOpHitObjectGetPrimitiveIndexEXT:
    case SpvOpHitObjectGetGeometryIndexEXT:
    case SpvOpHitObjectGetInstanceIdEXT:
    case SpvOpHitObjectGetInstanceCustomIndexEXT:
    case SpvOpHitObjectGetObjectRayOriginEXT:
    case SpvOpHitObjectGetObjectRayDirectionEXT:
    case SpvOpHitObjectGetWorldRayDirectionEXT:
    case SpvOpHitObjectGetWorldRayOriginEXT:
    case SpvOpHitObjectGetObjectToWorldEXT:
    case SpvOpHitObjectGetWorldToObjectEXT:
    case SpvOpHitObjectGetRayTMaxEXT:
    case SpvOpHitObjectGetRayTMinEXT:
    case SpvOpHitObjectGetShaderBindingTableRecordIndexEXT:
    case SpvOpHitObjectGetShaderRecordBufferHandleEXT:
    case SpvOpHitObjectIsEmptyEXT:
    case SpvOpHitObjectIsHitEXT:
    case SpvOpHitObjectIsMissEXT:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpHitObjectGetIntersectionTriangleVertexPositionsEXT:
        // The SPIR-V grammar defines the result-producing form, whose hit object is operand three.
        // Slang's GLSL module also uses a compatibility form with the hit object in operand one and
        // an output array in operand two.
        if (trySetSPIRVAddressOperandEffect(
                operandIndex,
                1,
                SPIRVAddressOperandRole::Read,
                effect) ||
            trySetSPIRVAddressOperandEffect(
                operandIndex,
                2,
                SPIRVAddressOperandRole::DefiniteWrite,
                effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            3,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpHitObjectGetAttributesNV:
    case SpvOpHitObjectGetAttributesEXT:
        if (trySetSPIRVAddressOperandEffect(operandIndex, 1, SPIRVAddressOperandRole::Read, effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            2,
            SPIRVAddressOperandRole::DefiniteWrite,
            effect);

    default:
        return false;
    }
}

/// Return whether `operandIndex` accesses a hit object or payload during a hit-object execution
/// operation.
static bool tryGetSPIRVHitObjectExecutionOperandEffect(
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // We treat the hit object as an input to execution and reordering operations. An invoked shader
    // may also update its payload, while changing the shader-table index mutates the hit object
    // itself.
    switch (opcode)
    {

    case SpvOpHitObjectExecuteShaderNV:
    case SpvOpHitObjectReorderExecuteShaderEXT:
    case SpvOpHitObjectExecuteShaderEXT:
        if (trySetSPIRVAddressOperandEffect(operandIndex, 1, SPIRVAddressOperandRole::Read, effect))
        {
            return true;
        }
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            2,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpReorderThreadWithHitObjectNV:
    case SpvOpReorderThreadWithHitObjectEXT:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::Read,
            effect);

    case SpvOpHitObjectSetShaderBindingTableRecordIndexEXT:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            1,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    default:
        return false;
    }
}

/// Return whether `operandIndex` supplies payload storage to a launched shader.
static bool tryGetSPIRVShaderPayloadOperandEffect(
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // We classify the payload operands of ray tracing, callable-shader, and mesh-shader launch
    // instructions. Ray and callable payloads may be read and updated by the invoked shader. A
    // mesh payload is an input to the launched mesh workgroups and is therefore read-only.
    switch (opcode)
    {

    case SpvOpTraceRayKHR:
    case SpvOpTraceNV:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            11,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpTraceMotionNV:
    case SpvOpTraceRayMotionNV:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            12,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpExecuteCallableKHR:
    case SpvOpExecuteCallableNV:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            2,
            SPIRVAddressOperandRole::ReadAndPossibleWrite,
            effect);

    case SpvOpEmitMeshTasksEXT:
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            4,
            SPIRVAddressOperandRole::Read,
            effect);

    default:
        return false;
    }
}

/// Return whether `operandIndex` has a known role as an address in an extended instruction.
static bool tryGetSPIRVExtendedInstructionOperandEffect(
    IRSPIRVAsmInst* asmInst,
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // We first inspect the instruction-set operand, because an extended opcode has meaning only
    // within its set. The parser represents `GLSL.std.450` and `NonSemantic.DebugPrintf` with
    // distinct IR operand kinds, so we can recognize them directly.
    if (opcode != SpvOpExtInst || asmInst->getOperandCount() <= 4)
        return false;

    // `NonSemantic.DebugPrintf` uses operand five as the format-string ID and every later operand
    // as a value to format. Neither role dereferences a pointer argument.
    auto instructionSet = asmInst->getOperand(3)->getOp();
    if (instructionSet == kIROp_SPIRVAsmOperandDebugPrintfSet)
    {
        if (operandIndex < 5)
            return false;
        effect = {};
        return true;
    }
    if (instructionSet != kIROp_SPIRVAsmOperandGLSL450Set)
        return false;

    auto extendedOpcodeOperand = as<IRSPIRVAsmOperand>(asmInst->getOperand(4));
    auto extendedOpcode =
        extendedOpcodeOperand ? as<IRIntLit>(extendedOpcodeOperand->getValue()) : nullptr;
    if (!extendedOpcode)
        return false;

    switch (extendedOpcode->getValue())
    {
    case GLSLstd450Modf:
    case GLSLstd450Frexp:
        // These operations return one result and write the other through operand six.
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            6,
            SPIRVAddressOperandRole::DefiniteWrite,
            effect);

    case GLSLstd450InterpolateAtCentroid:
    case GLSLstd450InterpolateAtSample:
    case GLSLstd450InterpolateAtOffset:
        // These operations read the interpolant through the pointer in operand five.
        return trySetSPIRVAddressOperandEffect(
            operandIndex,
            5,
            SPIRVAddressOperandRole::Read,
            effect);

    default:
        return false;
    }
}

/// Return whether `operandIndex` has a cataloged role as an address operand.
static bool tryGetCatalogedSPIRVAddressOperandEffect(
    IRSPIRVAsmInst* asmInst,
    SpvWord opcode,
    UInt operandIndex,
    SPIRVAddressOperandEffect& effect)
{
    // We divide the SPIR-V grammar into operand families so that each helper answers one concrete
    // question. The order is not significant because one opcode belongs to at most one family.
    if (tryGetSPIRVPointerValueOperandEffect(opcode, operandIndex, effect))
        return true;
    if (tryGetSPIRVMemoryExtensionOperandEffect(opcode, operandIndex, effect))
        return true;
    if (tryGetSPIRVTaskOrKernelOperandEffect(opcode, operandIndex, effect))
        return true;
    if (tryGetSPIRVRayQueryOperandEffect(opcode, operandIndex, effect))
        return true;
    if (tryGetSPIRVHitObjectConstructionOperandEffect(opcode, operandIndex, effect))
        return true;
    if (tryGetSPIRVHitObjectObservationOperandEffect(opcode, operandIndex, effect))
        return true;
    if (tryGetSPIRVHitObjectExecutionOperandEffect(opcode, operandIndex, effect))
        return true;
    if (tryGetSPIRVShaderPayloadOperandEffect(opcode, operandIndex, effect))
        return true;
    return tryGetSPIRVExtendedInstructionOperandEffect(asmInst, opcode, operandIndex, effect);
}

/// Return how the instruction containing `use` reads or writes the tracked value.
static InstructionUseEffect inferInstructionUseEffect(IRUse* use, bool instHasStorageFlow)
{
    // We first exclude instructions that propagate tracked value or storage flow, or inspect only
    // types. We then handle instructions with known operand roles. For any remaining role, we
    // conservatively record a read. This default can over-report an address-only operation, but it
    // avoids missing a dereference hidden in an instruction that this analysis does not recognize.
    // When an unknown use of a storage address produces another pointer, we also assume that the
    // operation may let the tracked address escape to a writer. We do not follow that unknown
    // result or claim to have found a particular downstream write. An unknown operation never
    // establishes definite initialization.
    //
    // Address-carrying operations such as `select` remain in that conservative default. Merely
    // suppressing their immediate read would miss a later dereference of the result. Following the
    // result correctly would require us to preserve possible and definite storage flow through
    // instructions that can choose among several addresses.
    auto user = use->getUser();
    if (doesInstOnlyDependOnOperandTypes(user) || isDebugInfoInst(user))
        return {};

    if (isSyntheticDebugValueLoad(user))
        return {};

    if (isUseSourceOfTrackedValueProjection(use) ||
        (instHasStorageFlow && mayPointerUseTransferStorageAccess(use)))
        return {};

    // Flow traversal follows operand zero of each projection. Evaluating a field, element, or
    // pointer-offset selector reads that selector's value.
    if (isProjectionSelectorUse(use))
        return {.readsValue = true};

    // When this operation receives an SSA value rather than an address in the tracked storage flow,
    // it must evaluate that value before it can propagate an access. The result can access storage
    // selected by a resource handle or pointer value, but it does not access the storage that held
    // that SSA value. We therefore record a read and stop following storage flow at this use.
    if (mayUseTransferStorageAccess(use))
        return {.readsValue = true};

    switch (user->getOp())
    {
    case kIROp_Loop:
    case kIROp_UnconditionalBranch:
        // Flow traversal follows branch arguments to their receiving block parameters. The
        // branch's remaining operands identify blocks and do not observe the tracked value.
        return {};

    case kIROp_Call:
        return inferCallUseEffect(as<IRCall>(user), use, instHasStorageFlow);

    case kIROp_Load:
    case kIROp_AtomicLoad:
        // A load reads the value stored at its address. When that value is itself a pointer, the
        // pointer result does not mean that the load can also write the storage being read.
        return {.readsValue = true};

    case kIROp_AtomicExchange:
    case kIROp_AtomicAdd:
    case kIROp_AtomicSub:
    case kIROp_AtomicMax:
    case kIROp_AtomicMin:
    case kIROp_AtomicAnd:
    case kIROp_AtomicOr:
    case kIROp_AtomicXor:
    case kIROp_AtomicInc:
    case kIROp_AtomicDec:
        // These operations read the previous value and unconditionally replace it. The pointer in
        // operand zero names the storage; every remaining operand is an input value. When operand
        // zero instead carries a pointer-valued SSA value, the operation reads that pointer but
        // does not replace it.
        if (use == user->getOperandUse(0) && instHasStorageFlow)
        {
            return {
                .readsValue = true,
                .mayWriteValue = true,
                .definitelyWritesValue = true,
            };
        }
        return {.readsValue = true};

    case kIROp_AtomicCompareExchange:
        // Compare-exchange always reads the previous value, but it writes the replacement only when
        // the comparison succeeds. As with the unconditional operations above, only operand zero
        // can name the tracked storage.
        if (use == user->getOperandUse(0) && instHasStorageFlow)
            return {.readsValue = true, .mayWriteValue = true};
        return {.readsValue = true};

    case kIROp_BitCast:
    case kIROp_Reinterpret:
    case kIROp_PtrCast:
    case kIROp_CastPtrToBool:
    case kIROp_CastPtrToInt:
    case kIROp_CastIntToPtr:
        // Storage-flow traversal handles a cast of the tracked storage address before this switch.
        // A cast of an SSA value reads that value, including when the value itself has pointer
        // type.
        return instHasStorageFlow ? InstructionUseEffect{}
                                  : InstructionUseEffect{.readsValue = true};

    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Less:
    case kIROp_Leq:
    case kIROp_Greater:
    case kIROp_Geq:
    case kIROp_Return:
        // Comparing a storage address observes the address, not the value stored there. Comparing
        // an SSA value reads that value, including when the value itself has pointer type.
        // Returning an address likewise exports only the address, while returning an SSA value
        // observes that value.
        return instHasStorageFlow ? InstructionUseEffect{}
                                  : InstructionUseEffect{.readsValue = true};

    case kIROp_Store:
    case kIROp_AtomicStore:
    case kIROp_SwizzledStore:
    case kIROp_MatrixSwizzleStore:
        // A store through a tracked storage address writes that storage. Storing the address as the
        // value operand does not read the stored value. In contrast, using a pointer-valued SSA
        // value in either operand reads that pointer value; a write through it changes its pointee,
        // not the pointer itself. Any later operands, such as swizzle indices or atomic
        // memory-order values, are read inputs rather than destinations.
        //
        // The general checker does not retain field or element paths while following
        // storage flow. It therefore treats a store through definite storage flow as a complete
        // assignment. Transformations that know whether a use writes the complete root value supply
        // an explicit effect instead; resource-global legalization uses that path for its generated
        // locals.
        if (use == user->getOperandUse(0))
        {
            return instHasStorageFlow
                       ? InstructionUseEffect{.mayWriteValue = true, .definitelyWritesValue = true}
                       : InstructionUseEffect{.readsValue = true};
        }
        if (use == user->getOperandUse(1))
        {
            return instHasStorageFlow ? InstructionUseEffect{}
                                      : InstructionUseEffect{.readsValue = true};
        }
        return {.readsValue = true};

    case kIROp_SPIRVAsm:
        // A direct operand of the assembly container has no instruction-level role. We record a
        // possible write so that assembly can satisfy the permissive check for an `out` parameter,
        // but we do not claim that it definitely initializes the value. An SSA value is an input
        // and must be read before assembly can use it.
        return instHasStorageFlow ? InstructionUseEffect{.mayWriteValue = true}
                                  : InstructionUseEffect{.readsValue = true};

    case kIROp_SPIRVAsmOperandInst:
        // `inferSPIRVAssemblyAddressEffects` handles every wrapper reached through storage flow
        // before this switch. A remaining wrapper preserves the IR value or address produced by
        // expression lowering, and using that SSA value as an assembly operand reads it. We retain
        // the wrapper as the effect point so diagnostics map back to the enclosing assembly
        // expression.
        SLANG_RELEASE_ASSERT(!instHasStorageFlow);
        return {.readsValue = true, .effectInstruction = user};

    case kIROp_MakeExistential:
    case kIROp_MakeExistentialWithRTTI:
        return {.readsValue = true};

    case kIROp_ManagedPtrAttach:
        // `ManagedPtrAttach` writes the managed-pointer variable in operand 0 and reads the native
        // pointer value in operand 1.
        if (use == user->getOperandUse(0) && instHasStorageFlow)
            return {.mayWriteValue = true, .definitelyWritesValue = true};
        return {.readsValue = true};

    case kIROp_Unmodified:
        // The `unused(inout T)` and `unmodified(out T)` intrinsics explicitly suppress
        // uninitialized-value diagnostics, so this marker establishes initialization by contract.
        return instHasStorageFlow
                   ? InstructionUseEffect{.mayWriteValue = true, .definitelyWritesValue = true}
                   : InstructionUseEffect{.readsValue = true};

    default:
        {
            InstructionUseEffect effect = {.readsValue = true};
            if (instHasStorageFlow && as<IRPtrTypeBase>(user->getDataType()))
                effect.mayWriteValue = true;
            return effect;
        }
    }
}

/// The way in which an inline SPIR-V instruction derives one pointer ID from other pointer IDs.
enum class SPIRVAddressForwardingKind
{
    AccessChain,
    PointerAccessChain,
    UntypedAccessChain,
    UntypedPointerAccessChain,
    RawAccessChain,
    Copy,
    PointerCast,
    FalliblePointerCast,
    RepresentationCast,
    Phi,
    Select,
    ConditionalCopy,
};

/// Facts about one inline SPIR-V ID that may denote the tracked storage.
struct SPIRVAddressFlow
{
    /// Whether at least one source of this ID denotes the tracked storage.
    bool mayReferToTrackedStorage = false;

    /// Whether every source of this ID denotes the tracked storage.
    bool definitelyRefersToTrackedStorage = false;

    /// Whether every source denotes the complete tracked value rather than one of its subobjects.
    bool definitelyDenotesCompleteValue = false;
};

/// The flow facts and defining instruction for one inline SPIR-V result ID.
struct SPIRVForwardedAddress
{
    IRStringLit* id = nullptr;
    IRSPIRVAsmInst* definition = nullptr;
    SPIRVAddressForwardingKind kind = SPIRVAddressForwardingKind::Copy;
    SPIRVAddressFlow flow;
};

/// Return the name represented by an inline SPIR-V ID operand, or null for another operand kind.
static IRStringLit* getSPIRVAssemblyId(IRInst* inst)
{
    auto operand = as<IRSPIRVAsmOperand>(inst);
    if (!operand || operand->getOp() != kIROp_SPIRVAsmOperandId)
        return nullptr;
    return as<IRStringLit>(operand->getValue());
}

/// Return whether `opcode` forwards an address through one of the supported SPIR-V instructions.
static bool tryGetSPIRVAddressForwardingKind(SpvWord opcode, SPIRVAddressForwardingKind& result)
{
    // Typed, untyped, and raw access chains preserve the identity of the allocation; their indexes
    // determine whether the result still denotes its complete value. Copy, freeze, and the
    // infallible storage-class casts preserve the address and its pointee type. The explicit
    // generic-pointer cast can instead produce a null pointer when its requested storage class
    // does not match. `OpBitcast` preserves the address bits, but a changed pointee type may cover
    // only part of the original value. `OpPhi`, `OpSelect`, and `OpConditionalCopyObjectINTEL` can
    // choose among several pointers.
    switch (opcode)
    {
    case SpvOpAccessChain:
    case SpvOpInBoundsAccessChain:
        result = SPIRVAddressForwardingKind::AccessChain;
        return true;

    case SpvOpPtrAccessChain:
    case SpvOpInBoundsPtrAccessChain:
        result = SPIRVAddressForwardingKind::PointerAccessChain;
        return true;

    case SpvOpUntypedAccessChainKHR:
    case SpvOpUntypedInBoundsAccessChainKHR:
        result = SPIRVAddressForwardingKind::UntypedAccessChain;
        return true;

    case SpvOpUntypedPtrAccessChainKHR:
    case SpvOpUntypedInBoundsPtrAccessChainKHR:
        result = SPIRVAddressForwardingKind::UntypedPointerAccessChain;
        return true;

    case SpvOpRawAccessChainNV:
        result = SPIRVAddressForwardingKind::RawAccessChain;
        return true;

    case SpvOpCopyObject:
    case SpvOpFreezeKHR:
        result = SPIRVAddressForwardingKind::Copy;
        return true;

    case SpvOpPtrCastToGeneric:
    case SpvOpGenericCastToPtr:
    case SpvOpPtrCastToCrossWorkgroupALTERA:
    case SpvOpCrossWorkgroupCastToPtrALTERA:
        result = SPIRVAddressForwardingKind::PointerCast;
        return true;

    case SpvOpGenericCastToPtrExplicit:
        result = SPIRVAddressForwardingKind::FalliblePointerCast;
        return true;

    case SpvOpBitcast:
        result = SPIRVAddressForwardingKind::RepresentationCast;
        return true;

    case SpvOpPhi:
        result = SPIRVAddressForwardingKind::Phi;
        return true;

    case SpvOpSelect:
        result = SPIRVAddressForwardingKind::Select;
        return true;

    case SpvOpConditionalCopyObjectINTEL:
        result = SPIRVAddressForwardingKind::ConditionalCopy;
        return true;

    default:
        return false;
    }
}

/// Find the flow record for `id`, or return null when no supported instruction defines it.
static SPIRVForwardedAddress* findSPIRVForwardedAddress(
    List<SPIRVForwardedAddress>& addresses,
    IRStringLit* id)
{
    // Inline SPIR-V emission identifies an ID by its spelling. We compare the text because linking
    // or cloning may create distinct string constants for two references to the same ID.
    if (!id)
        return nullptr;
    for (auto& address : addresses)
    {
        if (address.id->getStringSlice() == id->getStringSlice())
            return &address;
    }
    return nullptr;
}

/// Return the current address-flow facts for one inline SPIR-V operand.
static SPIRVAddressFlow getSPIRVOperandAddressFlow(
    IRSPIRVAsmOperand* operand,
    IRInst* trackedAddress,
    bool trackedAddressIsDefinite,
    List<SPIRVForwardedAddress>& addresses)
{
    // Every `$value` or `&value` occurrence has its own wrapper. We treat all wrappers around the
    // same tracked IR address as roots, because a later `OpPhi` or `OpSelect` may combine several
    // such occurrences. A named ID receives the facts computed for its defining instruction.
    if (auto instOperand = as<IRSPIRVAsmOperandInst>(operand))
    {
        if (instOperand->getValue() == trackedAddress)
        {
            return {
                .mayReferToTrackedStorage = true,
                .definitelyRefersToTrackedStorage = trackedAddressIsDefinite,
                .definitelyDenotesCompleteValue = trackedAddressIsDefinite,
            };
        }
        return {};
    }

    if (auto address = findSPIRVForwardedAddress(addresses, getSPIRVAssemblyId(operand)))
        return address->flow;
    return {};
}

/// Compute the flow facts implied by one supported pointer-producing SPIR-V instruction.
static SPIRVAddressFlow inferSPIRVForwardedAddressFlow(
    const SPIRVForwardedAddress& address,
    IRInst* trackedAddress,
    bool trackedAddressIsDefinite,
    List<SPIRVForwardedAddress>& addresses)
{
    // Result-producing SPIR-V instructions store the result type in IR operand one and the result
    // ID in operand two. The cases below use the literal operand positions from the SPIR-V grammar
    // after accounting for those two leading operands.
    auto definition = address.definition;
    auto getOperandFlow = [&](UInt index)
    {
        if (index >= definition->getOperandCount())
            return SPIRVAddressFlow{};
        return getSPIRVOperandAddressFlow(
            cast<IRSPIRVAsmOperand>(definition->getOperand(index)),
            trackedAddress,
            trackedAddressIsDefinite,
            addresses);
    };

    switch (address.kind)
    {
    case SPIRVAddressForwardingKind::AccessChain:
        {
            // Operand three is the base pointer, and any later operands select subobjects. The
            // grammar also permits an access chain with no indexes; that identity form preserves
            // the complete tracked value.
            auto result = getOperandFlow(3);
            if (definition->getOperandCount() > 4)
                result.definitelyDenotesCompleteValue = false;
            return result;
        }

    case SPIRVAddressForwardingKind::PointerAccessChain:
        {
            // Operand three is the base pointer and operand four is a mandatory element offset. We
            // conservatively treat the result as a subobject because this analysis does not resolve
            // an internal SPIR-V ID to prove that the offset is zero.
            auto result = getOperandFlow(3);
            result.definitelyDenotesCompleteValue = false;
            return result;
        }

    case SPIRVAddressForwardingKind::UntypedAccessChain:
        {
            // Operand three is the base type and operand four is the base pointer. Any operand
            // after the base pointer selects a subobject.
            auto result = getOperandFlow(4);
            if (definition->getOperandCount() > 5)
                result.definitelyDenotesCompleteValue = false;
            return result;
        }

    case SPIRVAddressForwardingKind::UntypedPointerAccessChain:
        {
            // Operand three is the base type, operand four is the base pointer, and operand five
            // is the mandatory element offset. We cannot prove that the offset is zero.
            auto result = getOperandFlow(4);
            result.definitelyDenotesCompleteValue = false;
            return result;
        }

    case SPIRVAddressForwardingKind::RawAccessChain:
        {
            // Operand three is the base pointer. Byte-stride, element-index, and byte-offset
            // operands follow it, so the result may identify only part of the tracked value.
            auto result = getOperandFlow(3);
            result.definitelyDenotesCompleteValue = false;
            return result;
        }

    case SPIRVAddressForwardingKind::Copy:
        // Operand three is the object copied by `OpCopyObject` or frozen by `OpFreezeKHR`.
        return getOperandFlow(3);

    case SPIRVAddressForwardingKind::PointerCast:
        // Operand three is the pointer converted to or from the Generic storage class. SPIR-V
        // requires both pointer types to have the same pointee type, so the cast preserves the
        // complete-value relation.
        return getOperandFlow(3);

    case SPIRVAddressForwardingKind::FalliblePointerCast:
        {
            // Operand three is the Generic pointer being converted. The result has the same
            // pointee type, but SPIR-V produces a null pointer when the requested storage class
            // does not match. The result may therefore retain the address without definitely
            // denoting the tracked storage.
            auto result = getOperandFlow(3);
            result.definitelyRefersToTrackedStorage = false;
            result.definitelyDenotesCompleteValue = false;
            return result;
        }

    case SPIRVAddressForwardingKind::RepresentationCast:
        {
            // Operand three is the value reinterpreted by `OpBitcast`. A pointer result preserves
            // the same address bits, but its pointee type may cover only part of the original
            // value. Clearing the complete-value fact prevents a later store from proving that it
            // initialized the complete tracked value.
            auto result = getOperandFlow(3);
            result.definitelyDenotesCompleteValue = false;
            return result;
        }

    case SPIRVAddressForwardingKind::Select:
        {
            // Operand three is the condition. Operands four and five are the two pointer choices.
            auto trueFlow = getOperandFlow(4);
            auto falseFlow = getOperandFlow(5);
            return {
                .mayReferToTrackedStorage =
                    trueFlow.mayReferToTrackedStorage || falseFlow.mayReferToTrackedStorage,
                .definitelyRefersToTrackedStorage = trueFlow.definitelyRefersToTrackedStorage &&
                                                    falseFlow.definitelyRefersToTrackedStorage,
                .definitelyDenotesCompleteValue = trueFlow.definitelyDenotesCompleteValue &&
                                                  falseFlow.definitelyDenotesCompleteValue,
            };
        }

    case SPIRVAddressForwardingKind::Phi:
        {
            // Starting at operand three, `OpPhi` stores alternating value and predecessor-label
            // operands. The result may denote the tracked storage when any incoming value does. It
            // definitely denotes that storage, and its complete value, only when every incoming
            // value does.
            SPIRVAddressFlow result;
            bool hasIncomingValue = false;
            result.definitelyRefersToTrackedStorage = true;
            result.definitelyDenotesCompleteValue = true;
            for (UInt i = 3; i < definition->getOperandCount(); i += 2)
            {
                hasIncomingValue = true;
                auto incoming = getOperandFlow(i);
                result.mayReferToTrackedStorage |= incoming.mayReferToTrackedStorage;
                result.definitelyRefersToTrackedStorage &=
                    incoming.definitelyRefersToTrackedStorage;
                result.definitelyDenotesCompleteValue &= incoming.definitelyDenotesCompleteValue;
            }
            if (!hasIncomingValue)
                return {};
            return result;
        }

    case SPIRVAddressForwardingKind::ConditionalCopy:
        {
            // Starting at operand four, `OpConditionalCopyObjectINTEL` stores alternating object
            // and condition operands. The result can select any listed object.
            SPIRVAddressFlow result;
            bool hasCandidate = false;
            result.definitelyRefersToTrackedStorage = true;
            result.definitelyDenotesCompleteValue = true;
            for (UInt i = 4; i < definition->getOperandCount(); i += 2)
            {
                hasCandidate = true;
                auto candidate = getOperandFlow(i);
                result.mayReferToTrackedStorage |= candidate.mayReferToTrackedStorage;
                result.definitelyRefersToTrackedStorage &=
                    candidate.definitelyRefersToTrackedStorage;
                result.definitelyDenotesCompleteValue &= candidate.definitelyDenotesCompleteValue;
            }
            if (!hasCandidate)
                return {};
            return result;
        }
    }

    SLANG_UNREACHABLE("unhandled SPIR-V address forwarding kind");
}

/// Collect the supported inline SPIR-V definitions that can forward a pointer.
static List<SPIRVForwardedAddress> collectSPIRVForwardedAddresses(IRSPIRVAsm* assembly)
{
    // We record a supported instruction only when operand two defines an internal ID.
    // `getSPIRVAssemblyResultAddressFlow` handles a result marker separately because it exports the
    // address as the enclosing assembly expression's result.
    List<SPIRVForwardedAddress> result;
    for (auto asmInst : assembly->getInsts())
    {
        SpvWord opcode;
        SPIRVAddressForwardingKind kind;
        if (!tryGetSPIRVOpcodeWord(asmInst, opcode) ||
            !tryGetSPIRVAddressForwardingKind(opcode, kind) || asmInst->getOperandCount() <= 2)
        {
            continue;
        }

        auto id = getSPIRVAssemblyId(asmInst->getOperand(2));
        if (!id || findSPIRVForwardedAddress(result, id))
            continue;

        result.add(SPIRVForwardedAddress{
            .id = id,
            .definition = asmInst,
            .kind = kind,
        });
    }
    return result;
}

/// Solve address flow from `trackedAddress` through the supported inline SPIR-V instructions.
static List<SPIRVForwardedAddress> solveSPIRVAddressFlow(
    IRSPIRVAsm* assembly,
    IRInst* trackedAddress,
    bool trackedAddressIsDefinite)
{
    // We first compute the least fixed point for possible flow. This finds every pointer ID that a
    // path from the tracked address can reach, including through a cyclic `OpPhi`. We then begin
    // definite and complete flow at the greatest possible solution and remove facts that the
    // defining instruction cannot guarantee. Starting the second phase from the greatest solution
    // preserves a loop-carried `OpPhi` whose entry value and back-edge value both denote the same
    // tracked storage.
    auto addresses = collectSPIRVForwardedAddresses(assembly);
    bool changed;
    do
    {
        changed = false;
        for (auto& address : addresses)
        {
            auto inferred = inferSPIRVForwardedAddressFlow(
                address,
                trackedAddress,
                trackedAddressIsDefinite,
                addresses);
            if (inferred.mayReferToTrackedStorage && !address.flow.mayReferToTrackedStorage)
            {
                address.flow.mayReferToTrackedStorage = true;
                changed = true;
            }
        }
    } while (changed);

    for (auto& address : addresses)
    {
        address.flow.definitelyRefersToTrackedStorage = address.flow.mayReferToTrackedStorage;
        address.flow.definitelyDenotesCompleteValue = address.flow.mayReferToTrackedStorage;
    }

    do
    {
        changed = false;
        for (auto& address : addresses)
        {
            auto inferred = inferSPIRVForwardedAddressFlow(
                address,
                trackedAddress,
                trackedAddressIsDefinite,
                addresses);
            bool isDefinite =
                address.flow.mayReferToTrackedStorage && inferred.definitelyRefersToTrackedStorage;
            bool isComplete = isDefinite && inferred.definitelyDenotesCompleteValue;
            if (address.flow.definitelyRefersToTrackedStorage != isDefinite ||
                address.flow.definitelyDenotesCompleteValue != isComplete)
            {
                address.flow.definitelyRefersToTrackedStorage = isDefinite;
                address.flow.definitelyDenotesCompleteValue = isComplete;
                changed = true;
            }
        }
    } while (changed);

    return addresses;
}

/// Return the address-flow facts for an inline SPIR-V expression's result marker.
static SPIRVAddressFlow getSPIRVAssemblyResultAddressFlow(
    IRSPIRVAsm* assembly,
    IRInst* trackedAddress)
{
    // A result-producing SPIR-V instruction stores its result ID in operand two. The inline
    // assembly result marker occupies that position in the final instruction and makes the result
    // available as the value of the enclosing `IRSPIRVAsm` instruction. We solve named-ID flow
    // first, then apply the same forwarding rule to that final definition.
    auto addresses = solveSPIRVAddressFlow(assembly, trackedAddress, true);
    for (auto asmInst : assembly->getInsts())
    {
        if (asmInst->getOperandCount() <= 2 ||
            asmInst->getOperand(2)->getOp() != kIROp_SPIRVAsmOperandResult)
        {
            continue;
        }

        SpvWord opcode;
        SPIRVAddressForwardingKind kind;
        if (!tryGetSPIRVOpcodeWord(asmInst, opcode) ||
            !tryGetSPIRVAddressForwardingKind(opcode, kind))
        {
            return {};
        }

        SPIRVForwardedAddress resultAddress{
            .definition = asmInst,
            .kind = kind,
        };
        return inferSPIRVForwardedAddressFlow(resultAddress, trackedAddress, true, addresses);
    }
    return {};
}

/// Return whether the inline SPIR-V result may denote `address`.
static bool maySPIRVAssemblyResultReferToAddress(IRSPIRVAsm* assembly, IRInst* address)
{
    // Only a pointer-typed assembly result can carry storage flow into ordinary Slang IR.
    if (!as<IRPtrTypeBase>(assembly->getDataType()))
        return false;
    return getSPIRVAssemblyResultAddressFlow(assembly, address).mayReferToTrackedStorage;
}

/// Return whether the inline SPIR-V result definitely denotes all of `address`'s value.
static bool doesSPIRVAssemblyResultDefinitelyDenoteCompleteAddress(
    IRSPIRVAsm* assembly,
    IRInst* address)
{
    auto flow = getSPIRVAssemblyResultAddressFlow(assembly, address);
    return flow.definitelyRefersToTrackedStorage && flow.definitelyDenotesCompleteValue;
}

/// Return the Slang parameter type for one direct inline-SPIR-V call argument, or null when the
/// assembly does not expose that type.
static IRType* findSPIRVFunctionCallArgumentParameterType(
    IRSPIRVAsmInst* call,
    UInt argumentOperandIndex)
{
    // `OpFunctionCall` stores its function in operand three and its arguments from operand four
    // onward. A `$function` operand retains the Slang function value and therefore its directional
    // parameter types. A named SPIR-V ID or indirect function pointer does not provide that
    // information to this analysis.
    if (call->getOperandCount() <= 3 || argumentOperandIndex < 4)
        return nullptr;

    auto functionOperand = as<IRSPIRVAsmOperandInst>(call->getOperand(3));
    if (!functionOperand)
        return nullptr;

    auto functionType =
        as<IRFuncType>(unwrapAttributedType(functionOperand->getValue()->getDataType()));
    UInt parameterIndex = argumentOperandIndex - 4;
    if (!functionType || parameterIndex >= functionType->getParamCount())
        return nullptr;
    return as<IRType>(unwrapAttributedType(functionType->getParamType(parameterIndex)));
}

/// Return whether `operandIndex` has an address role understood by this analysis.
static bool isRecognizedSPIRVAddressOperand(
    IRSPIRVAsmInst* asmInst,
    SpvWord opcode,
    UInt operandIndex)
{
    // The indices below are IR operand indices, including the opcode at index zero. They come
    // directly from the SPIR-V grammar: result-producing instructions place their result type and
    // result ID before their semantic operands, while stores and memory copies have no result.
    SPIRVAddressOperandEffect catalogedEffect;
    if (tryGetCatalogedSPIRVAddressOperandEffect(asmInst, opcode, operandIndex, catalogedEffect))
    {
        return true;
    }

    switch (opcode)
    {
    case SpvOpAccessChain:
    case SpvOpInBoundsAccessChain:
    case SpvOpPtrAccessChain:
    case SpvOpInBoundsPtrAccessChain:
    case SpvOpRawAccessChainNV:
    case SpvOpCopyObject:
    case SpvOpFreezeKHR:
    case SpvOpPtrCastToGeneric:
    case SpvOpGenericCastToPtr:
    case SpvOpGenericCastToPtrExplicit:
    case SpvOpPtrCastToCrossWorkgroupALTERA:
    case SpvOpCrossWorkgroupCastToPtrALTERA:
    case SpvOpBitcast:
        return operandIndex == 3;
    case SpvOpUntypedAccessChainKHR:
    case SpvOpUntypedInBoundsAccessChainKHR:
    case SpvOpUntypedPtrAccessChainKHR:
    case SpvOpUntypedInBoundsPtrAccessChainKHR:
        return operandIndex == 4;
    case SpvOpPhi:
        return operandIndex >= 3 && ((operandIndex - 3) % 2) == 0;
    case SpvOpSelect:
        return operandIndex == 4 || operandIndex == 5;
    case SpvOpConditionalCopyObjectINTEL:
        return operandIndex >= 4 && (operandIndex % 2) == 0;
    case SpvOpLoad:
        return operandIndex == 3;
    case SpvOpStore:
        return operandIndex == 1;
    case SpvOpCopyMemory:
    case SpvOpCopyMemorySized:
        return operandIndex == 1 || operandIndex == 2;
    case SpvOpFunctionCall:
    case SpvOpFunctionPointerCallINTEL:
        return operandIndex >= 4;
    default:
        break;
    }

    SPIRVAtomicMemoryEffect atomicEffect;
    if (tryGetSPIRVAtomicMemoryEffect(opcode, atomicEffect))
    {
        UInt pointerOperandIndex = atomicEffect == SPIRVAtomicMemoryEffect::Write ? 1 : 3;
        return operandIndex == pointerOperandIndex;
    }
    return false;
}

/// Return whether `operandIndex` is a pointer source forwarded by `kind`.
static bool isSPIRVAddressForwardingSource(SPIRVAddressForwardingKind kind, UInt operandIndex)
{
    // These are the same literal source positions used by
    // `inferSPIRVForwardedAddressFlow`. Keeping this predicate separate lets the effect scan
    // distinguish an internal named result from a result marker that exports the pointer.
    switch (kind)
    {
    case SPIRVAddressForwardingKind::AccessChain:
    case SPIRVAddressForwardingKind::PointerAccessChain:
    case SPIRVAddressForwardingKind::RawAccessChain:
    case SPIRVAddressForwardingKind::Copy:
    case SPIRVAddressForwardingKind::PointerCast:
    case SPIRVAddressForwardingKind::FalliblePointerCast:
    case SPIRVAddressForwardingKind::RepresentationCast:
        return operandIndex == 3;
    case SPIRVAddressForwardingKind::UntypedAccessChain:
    case SPIRVAddressForwardingKind::UntypedPointerAccessChain:
        return operandIndex == 4;
    case SPIRVAddressForwardingKind::Phi:
        return operandIndex >= 3 && ((operandIndex - 3) % 2) == 0;
    case SPIRVAddressForwardingKind::Select:
        return operandIndex == 4 || operandIndex == 5;
    case SPIRVAddressForwardingKind::ConditionalCopy:
        return operandIndex >= 4 && (operandIndex % 2) == 0;
    }
    SLANG_UNREACHABLE("unhandled SPIR-V address forwarding kind");
}

/// Infer all memory effects reached through one tracked address in inline SPIR-V assembly.
static bool inferSPIRVAssemblyAddressEffects(
    IRUse* use,
    bool hasDefiniteStorageFlow,
    List<InstructionUseEffect>& effects)
{
    // The outer analysis sees one use for each `$value` or `&value` wrapper, while internal SPIR-V
    // IDs are connected only by their names. We handle the first wrapper around a tracked address,
    // solve the internal ID graph once, and then report, in source order, every memory operation
    // whose address may derive from the tracked address. Later wrappers around the same address are
    // already covered by that solution.
    auto operand = as<IRSPIRVAsmOperandInst>(use->getUser());
    if (!operand)
        return false;

    auto assembly = operand->getAsmBlock();
    auto trackedAddress = use->get();
    for (auto child = assembly->getFirstChild(); child && child != operand;
         child = child->getNextInst())
    {
        if (auto earlierOperand = as<IRSPIRVAsmOperandInst>(child))
        {
            if (earlierOperand->getValue() == trackedAddress)
                return true;
        }
    }

    auto addresses = solveSPIRVAddressFlow(assembly, trackedAddress, hasDefiniteStorageFlow);
    auto getOperandFlow = [&](IRSPIRVAsmInst* asmInst, UInt index)
    {
        if (index >= asmInst->getOperandCount())
            return SPIRVAddressFlow{};
        return getSPIRVOperandAddressFlow(
            cast<IRSPIRVAsmOperand>(asmInst->getOperand(index)),
            trackedAddress,
            hasDefiniteStorageFlow,
            addresses);
    };
    auto addRead = [&](IRSPIRVAsmInst* asmInst, SPIRVAddressFlow flow)
    {
        if (flow.mayReferToTrackedStorage)
            effects.add({.readsValue = true, .effectInstruction = asmInst});
    };
    auto addWrite =
        [&](IRSPIRVAsmInst* asmInst, SPIRVAddressFlow flow, bool definitelyWritesCompletePointee)
    {
        if (!flow.mayReferToTrackedStorage)
            return;
        effects.add({
            .mayWriteValue = true,
            .definitelyWritesValue =
                definitelyWritesCompletePointee && flow.definitelyRefersToTrackedStorage &&
                flow.definitelyDenotesCompleteValue && isInEntryBlockOfSPIRVAssembly(asmInst),
            .effectInstruction = asmInst,
        });
    };

    // We interpret the address roles below literally. A read through an address that may denote the
    // tracked storage is a possible read of that value. A write initializes the complete value only
    // when the address definitely denotes that complete value and the SPIR-V operation executes in
    // the assembly entry block.
    for (auto asmInst : assembly->getInsts())
    {
        SpvWord opcode;
        if (!tryGetSPIRVOpcodeWord(asmInst, opcode))
            continue;

        if (opcode == SpvOpLoad)
        {
            addRead(asmInst, getOperandFlow(asmInst, 3));
        }
        else if (opcode == SpvOpStore)
        {
            addWrite(asmInst, getOperandFlow(asmInst, 1), true);
        }
        else if (opcode == SpvOpCopyMemory)
        {
            addWrite(asmInst, getOperandFlow(asmInst, 1), true);
            addRead(asmInst, getOperandFlow(asmInst, 2));
        }
        else if (opcode == SpvOpCopyMemorySized)
        {
            // The size operand may cover only part of the pointee. Because this analysis has no
            // target layout with which to prove otherwise, the copy is a possible write but not a
            // definite initialization of the complete tracked value.
            addWrite(asmInst, getOperandFlow(asmInst, 1), false);
            addRead(asmInst, getOperandFlow(asmInst, 2));
        }
        else if (opcode == SpvOpFunctionCall || opcode == SpvOpFunctionPointerCallINTEL)
        {
            // Operand three names the function or function pointer, and operands four onward are
            // its arguments. A direct `$function` operand retains Slang parameter directions. An
            // `out` argument is a complete assignment when the call executes, a borrowed input is
            // read-only, and `inout` or `ref` can read and write. When no Slang signature is
            // available, SPIR-V provides no direction contract, so the callee may do both.
            for (UInt i = 4; i < asmInst->getOperandCount(); ++i)
            {
                auto argumentFlow = getOperandFlow(asmInst, i);
                auto parameterType = opcode == SpvOpFunctionCall
                                         ? findSPIRVFunctionCallArgumentParameterType(asmInst, i)
                                         : nullptr;
                if (as<IROutParamType>(parameterType))
                {
                    addWrite(asmInst, argumentFlow, true);
                }
                else if (as<IRBorrowInParamType>(parameterType))
                {
                    addRead(asmInst, argumentFlow);
                }
                else
                {
                    addRead(asmInst, argumentFlow);
                    addWrite(asmInst, argumentFlow, false);
                }
            }
        }
        else
        {
            SPIRVAtomicMemoryEffect atomicEffect;
            if (tryGetSPIRVAtomicMemoryEffect(opcode, atomicEffect))
            {
                UInt pointerOperandIndex = atomicEffect == SPIRVAtomicMemoryEffect::Write ? 1 : 3;
                auto pointerFlow = getOperandFlow(asmInst, pointerOperandIndex);
                if (atomicEffect == SPIRVAtomicMemoryEffect::Read ||
                    atomicEffect == SPIRVAtomicMemoryEffect::ReadAndWrite ||
                    atomicEffect == SPIRVAtomicMemoryEffect::ReadAndConditionalWrite)
                {
                    addRead(asmInst, pointerFlow);
                }
                if (atomicEffect == SPIRVAtomicMemoryEffect::Write ||
                    atomicEffect == SPIRVAtomicMemoryEffect::ReadAndWrite ||
                    atomicEffect == SPIRVAtomicMemoryEffect::ReadAndConditionalWrite)
                {
                    addWrite(
                        asmInst,
                        pointerFlow,
                        atomicEffect != SPIRVAtomicMemoryEffect::ReadAndConditionalWrite);
                }
            }
        }

        for (UInt i = 1; i < asmInst->getOperandCount(); ++i)
        {
            SPIRVAddressOperandEffect effect;
            if (!tryGetCatalogedSPIRVAddressOperandEffect(asmInst, opcode, i, effect))
                continue;

            auto flow = getOperandFlow(asmInst, i);
            if (effect.readsValue)
                addRead(asmInst, flow);
            if (effect.mayWriteValue)
                addWrite(asmInst, flow, effect.definitelyWritesCompleteValue);
        }

        // The cases above classify every pointee access in the current core SPIR-V grammar, plus
        // the GLSL.std.450 operations for which Slang provides a dedicated instruction-set
        // operand. A tracked address in any remaining role is transported or exposed as a pointer
        // value. That use does not read the pointee, but another operation may retain the pointer
        // and write through it. We therefore record a possible write without claiming either a
        // read or a definite initialization. When Slang adds another dereferencing opcode or
        // extended instruction set, add its exact operand roles above.
        //
        // The recognized-operand test also includes the sources of supported forwarding
        // instructions. We suppress their direct effect only when the fixed-point solver follows
        // the pointer into an internal result ID or the exported assembly result; otherwise the
        // pointer has escaped through an untracked result.
        auto forwardedResult =
            asmInst->getOperandCount() > 2
                ? findSPIRVForwardedAddress(addresses, getSPIRVAssemblyId(asmInst->getOperand(2)))
                : nullptr;
        for (UInt i = 1; i < asmInst->getOperandCount(); ++i)
        {
            auto flow = getOperandFlow(asmInst, i);
            if (!flow.mayReferToTrackedStorage)
                continue;

            // Operand two defines the result ID of every supported forwarding instruction. The ID
            // can acquire tracked-address flow from this definition, but defining it is not a use
            // of the address and therefore has no memory effect.
            auto operandAddress =
                findSPIRVForwardedAddress(addresses, getSPIRVAssemblyId(asmInst->getOperand(i)));
            if (i == 2 && operandAddress && operandAddress->definition == asmInst)
                continue;

            if (isRecognizedSPIRVAddressOperand(asmInst, opcode, i))
            {
                SPIRVAddressForwardingKind forwardingKind;
                bool forwardsAddress = tryGetSPIRVAddressForwardingKind(opcode, forwardingKind) &&
                                       isSPIRVAddressForwardingSource(forwardingKind, i);
                bool exportsAssemblyResult =
                    asmInst->getOperandCount() > 2 &&
                    asmInst->getOperand(2)->getOp() == kIROp_SPIRVAsmOperandResult;
                if (!forwardsAddress || forwardedResult || exportsAssemblyResult)
                {
                    continue;
                }
            }
            effects.add({.mayWriteValue = true, .effectInstruction = asmInst});
            break;
        }
    }
    return true;
}

/// Return the Slang IR instruction whose execution contains `effectInstruction`.
static IRInst* getExecutableInstructionForEffect(IRInst* effectInstruction)
{
    // Most effects occur directly at an instruction in a Slang IR block. An `IRSPIRVAsmInst` is
    // instead one ordered operation inside its parent `IRSPIRVAsm`; the parent is the instruction
    // that participates in the surrounding control-flow graph and carries the source location.
    if (as<IRSPIRVAsmInst>(effectInstruction) || as<IRSPIRVAsmOperand>(effectInstruction))
        return effectInstruction->getParent();
    return effectInstruction;
}

/// Return whether `first` occurs before `second` inside one inline SPIR-V assembly expression.
static bool isEarlierSPIRVAssemblyEffect(IRInst* first, IRInst* second)
{
    // The child order of `IRSPIRVAsm` is the textual order of its SPIR-V operations. We use that
    // order only when both effects belong to the same assembly expression. An operation does not
    // precede itself: an atomic read observes the incoming value before its own write.
    auto firstAsmInst = as<IRSPIRVAsmInst>(first);
    auto secondAsmInst = as<IRSPIRVAsmInst>(second);
    if (!firstAsmInst || !secondAsmInst ||
        firstAsmInst->getParent() != secondAsmInst->getParent() || firstAsmInst == secondAsmInst)
    {
        return false;
    }

    auto assembly = cast<IRSPIRVAsm>(firstAsmInst->getParent());
    bool foundFirst = false;
    for (auto asmInst : assembly->getInsts())
    {
        if (asmInst == firstAsmInst)
            foundFirst = true;
        if (asmInst == secondAsmInst)
            return foundFirst;
    }
    SLANG_UNREACHABLE("SPIR-V assembly effect is not a child of its reported parent");
}

/// Return whether the effect at `first` can reach the later effect at `second`.
static bool isEffectReachable(ReachabilityContext& reachability, IRInst* first, IRInst* second)
{
    // Distinct Slang IR instructions use CFG reachability. Two effects inside the same
    // inline assembly instruction instead use the ordered SPIR-V children retained above.
    auto firstExecutable = getExecutableInstructionForEffect(first);
    auto secondExecutable = getExecutableInstructionForEffect(second);
    if (firstExecutable == secondExecutable)
        return isEarlierSPIRVAssemblyEffect(first, second);
    return reachability.isInstReachable(firstExecutable, secondExecutable);
}

/// Record generic assembly in `block` as a possible write to an unresolved output parameter.
static void collectGenericAssemblyPossibleWrites(List<IRInst*>& possibleWrites, IRBlock* block)
{
    // Generic assembly does not expose operand directions. We record it as a possible but not
    // definite write. It can therefore satisfy the coarse check for an unresolved `out` parameter,
    // but it cannot prove initialization before a later read.
    for (auto inst = block->getFirstInst(); inst; inst = inst->next)
    {
        if (as<IRGenericAsm>(inst))
            possibleWrites.add(inst);
    }
}

/// Add one use of the tracked value to the read/write sets consumed by the two CFG analyses.
static void collectInstructionUse(
    List<IRInst*>& possibleWrites,
    List<IRInst*>* definiteWrites,
    List<IRInst*>& reads,
    IRUse* use,
    bool instHasStorageFlow,
    bool hasDefiniteStorageFlow,
    ConstArrayView<UninitializedVariableUseEffect> useEffects = {},
    HashSet<IRUse*>* consumedUseEffects = nullptr)
{
    // We first look for an exact effect supplied by the caller. The caller may know whether the use
    // applies to the complete tracked value or only a subobject, even when the rewritten opcode and
    // operand role no longer express that fact. Its answer therefore takes precedence. An inline
    // SPIR-V operand can reach several ordered memory operations through internal IDs, so we
    // collect those effects separately. Every other use has one effect inferred from its
    // instruction and operand role.
    auto user = use->getUser();
    InstructionUseEffect effect;
    bool foundExplicitEffect = false;
    for (auto const& explicitEffect : useEffects)
    {
        if (explicitEffect.use != use)
            continue;

        SLANG_RELEASE_ASSERT(consumedUseEffects && consumedUseEffects->add(use));
        foundExplicitEffect = true;
        effect = InstructionUseEffect{
            .readsValue = explicitEffect.readsValue,
            .mayWriteValue = explicitEffect.mayWriteValue || explicitEffect.definitelyWritesValue,
            .definitelyWritesValue = explicitEffect.definitelyWritesValue,
        };
        break;
    }

    if (!foundExplicitEffect)
    {
        if (instHasStorageFlow)
        {
            List<InstructionUseEffect> assemblyEffects;
            if (inferSPIRVAssemblyAddressEffects(use, hasDefiniteStorageFlow, assemblyEffects))
            {
                for (auto assemblyEffect : assemblyEffects)
                {
                    auto effectInstruction =
                        assemblyEffect.effectInstruction ? assemblyEffect.effectInstruction : user;
                    if (assemblyEffect.readsValue)
                        reads.add(effectInstruction);
                    if (assemblyEffect.mayWriteValue || assemblyEffect.definitelyWritesValue)
                        possibleWrites.add(effectInstruction);
                    if (assemblyEffect.definitelyWritesValue && definiteWrites)
                        definiteWrites->add(effectInstruction);
                }
                return;
            }
        }

        effect = inferInstructionUseEffect(use, instHasStorageFlow);

        // A write through uncertain storage flow may update unrelated storage. We keep it as a
        // possible write, but it cannot prove that the tracked value is initialized.
        if (instHasStorageFlow && !hasDefiniteStorageFlow)
            effect.definitelyWritesValue = false;
    }

    auto effectInstruction = effect.effectInstruction ? effect.effectInstruction : user;
    if (effect.readsValue)
        reads.add(effectInstruction);
    if (effect.mayWriteValue || effect.definitelyWritesValue)
        possibleWrites.add(effectInstruction);
    if (effect.definitelyWritesValue && definiteWrites)
        definiteWrites->add(effectInstruction);
}

/// Retain reads that are not reachable from any possible write.
static void retainReadsUnreachableFromPossibleWrites(
    ReachabilityContext& reachability,
    const List<IRInst*>& possibleWrites,
    List<IRInst*>& reads)
{
    // We remove each read reachable from an earlier possible write. When one instruction both reads
    // and writes, its outgoing write cannot initialize the incoming value read by that instruction.
    for (auto write : possibleWrites)
    {
        for (Index i = 0; i < reads.getCount();)
        {
            if (isEffectReachable(reachability, write, reads[i]))
                reads.fastRemoveAt(i);
            else
                i++;
        }
    }
}

/// Return the successor that cannot be taken after control enters `block` from `fromPred`, or null
/// when the predecessor does not determine the branch condition.
///
/// Slang lowers short-circuit expressions through a merge block whose condition is a block
/// parameter. `fromPred` may pass a Boolean constant to that parameter. We return the successor
/// that contradicts that constant. Without this correlation, the CFG contains a spurious
/// write-free path in expressions such as `f(out x) && use(x)`.
static IRBlock* findInfeasibleBranchFromPredecessor(IRBlock* block, IRBlock* fromPred)
{
    auto ifElse = as<IRIfElse>(block->getTerminator());
    if (!ifElse)
        return nullptr;

    auto cond = ifElse->getCondition();
    auto condParam = as<IRParam>(cond);
    if (!condParam || condParam->getParent() != block)
        return nullptr;

    // We find the condition's position among the block parameters.
    UInt paramIndex = 0;
    bool found = false;
    for (auto p : block->getParams())
    {
        if (p == condParam)
        {
            found = true;
            break;
        }
        paramIndex++;
    }
    if (!found)
        return nullptr;

    // We then read the argument supplied at that position by `fromPred`.
    auto branch = as<IRUnconditionalBranch>(fromPred->getTerminator());
    if (!branch || paramIndex >= branch->getArgCount())
        return nullptr;

    auto argVal = as<IRBoolLit>(branch->getArg(paramIndex));
    if (!argVal)
        return nullptr;

    // A constant argument makes the opposite successor infeasible on this edge.
    return argVal->getValue() ? ifElse->getFalseBlock() : ifElse->getTrueBlock();
}

/// A `WaveElectionGuard` identifies the elected-lane region and its reconvergence block for one
/// direct `if (WaveIsFirstLane())` statement.
struct WaveElectionGuard
{
    /// The first block executed by the elected lane.
    IRBlock* trueBlock;

    /// The block where the true and false branches reconverge.
    IRBlock* mergeBlock;
};

/// Collect direct `if (WaveIsFirstLane())` guards in `func`.
static List<WaveElectionGuard> collectWaveElectionGuards(IRGlobalValueWithCode* func)
{
    // We require the condition itself to be the known builtin. An inverted condition executes its
    // true branch on non-elected lanes, so a write in that branch cannot initialize the value later
    // broadcast from the elected lane.
    List<WaveElectionGuard> guards;
    for (auto block : func->getBlocks())
    {
        auto ifElse = as<IRIfElse>(block->getTerminator());
        if (!ifElse)
            continue;
        auto call = as<IRCall>(ifElse->getCondition());
        if (!call)
            continue;
        if (getBuiltinFuncEnum(call->getCallee()) != KnownBuiltinDeclName::WaveIsFirstLane)
            continue;
        guards.add(WaveElectionGuard{ifElse->getTrueBlock(), ifElse->getAfterBlock()});
    }
    return guards;
}

/// A `WaveElectionContext` stores the function-wide analysis needed for elected-lane reads.
struct WaveElectionContext
{
    /// The direct elected-lane guards in the function.
    List<WaveElectionGuard> guards;

    /// The function's dominator tree, or null when `guards` is empty.
    RefPtr<IRDominatorTree> dominatorTree;
};

/// Collect the elected-lane guards in `func` and the dominator tree needed to analyze them.
static WaveElectionContext collectWaveElectionContext(IRGlobalValueWithCode* func)
{
    // The guards and dominator tree do not depend on the variable being checked, so we compute them
    // once per function. Most functions have no elected-lane guard and need no dominator tree.
    WaveElectionContext context;
    context.guards = collectWaveElectionGuards(func);
    if (context.guards.getCount() != 0)
        context.dominatorTree = computeDominatorTree(func);
    return context;
}

/// Return whether `to` is reachable from `from` through CFG successor edges.
static bool isBlockReachableFrom(IRBlock* from, IRBlock* to)
{
    // We use this independent reachability test to distinguish a path blocked by a write from a
    // path that exits through `return`, `break`, or `discard` and never reaches `to` at all.
    HashSet<IRBlock*> visited;
    List<IRBlock*> worklist;
    visited.add(from);
    worklist.add(from);
    while (worklist.getCount())
    {
        auto block = worklist.getLast();
        worklist.removeLast();
        if (block == to)
            return true;
        for (auto succ : block->getSuccessors())
        {
            if (visited.add(succ))
                worklist.add(succ);
        }
    }
    return false;
}

/// Return whether `to` is reachable from `from` and every such path reaches a write block before
/// reaching `to`.
static bool isEveryPathFromBlockedByWrite(
    IRBlock* from,
    IRBlock* to,
    const HashSet<IRBlock*>& blocksWithWrite)
{
    // We first prove that the elected-lane region can reach its merge block at all. We then repeat
    // the search while treating write blocks as barriers. Reaching `to` in the second search proves
    // that one path omits an earlier write; exhausting the search proves that every path reaches a
    // write before `to`.
    //
    // We search from the true branch instead of using whole-function dominance because the false
    // branch also reaches the merge block and does not execute the elected lane's write.
    if (!isBlockReachableFrom(from, to))
        return false;

    HashSet<IRBlock*> visited;
    List<IRBlock*> worklist;
    visited.add(from);
    worklist.add(from);
    while (worklist.getCount())
    {
        auto block = worklist.getLast();
        worklist.removeLast();
        if (block == to)
            return false; // Reached `to` along a path with no preceding write.
        if (blocksWithWrite.contains(block))
            continue; // A write in this block blocks propagation past it.
        for (auto succ : block->getSuccessors())
        {
            if (visited.add(succ))
                worklist.add(succ);
        }
    }
    return true;
}

/// Return whether `readingInst` is, or feeds only, a direct `WaveReadLaneFirst` call.
static bool isWaveReadLaneFirstUse(IRInst* readingInst)
{
    // The IR can attach the source read either to the builtin call or to the instruction that
    // computes its argument. In the latter case, we accept the read only when the builtin call is
    // its sole runtime use. We do not look through user functions because that would suppress a
    // diagnostic without proving the required wave behavior.
    if (auto call = as<IRCall>(readingInst))
    {
        if (getBuiltinFuncEnum(call->getCallee()) == KnownBuiltinDeclName::WaveReadLaneFirst)
            return true;
    }

    IRInst* realUse = nullptr;
    int numRealUses = 0;
    for (auto use = readingInst->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (doesInstOnlyDependOnOperandTypes(user) || isDebugInfoInst(user))
            continue;
        numRealUses++;
        realUse = user;
    }
    if (numRealUses != 1)
        return false;
    auto call = as<IRCall>(realUse);
    if (!call)
        return false;
    return getBuiltinFuncEnum(call->getCallee()) == KnownBuiltinDeclName::WaveReadLaneFirst;
}

/// Retain reads reachable along at least one path without a preceding write classified as definite.
///
/// We walk forward from the function entry and stop propagation after a write classified as
/// definite. A read is safe only if no write-free path reaches it. This path-based test handles
/// separate writes on separate branches, where no single write dominates the read even though every
/// path performs one. It also prunes infeasible short-circuit edges so `f(out x) && use(x)` does
/// not acquire a spurious path from the failed call to `use(x)`.
static void retainReadsNotDefinitelyInitialized(
    IRGlobalValueWithCode* func,
    const List<IRInst*>& definiteWrites,
    List<IRInst*>& reads,
    const WaveElectionContext& waveElection)
{
    if (reads.getCount() == 0)
        return;

    // We index writes classified as definite both by executable instruction and by containing
    // block. The block set stops propagation to successors. The instruction set preserves ordering
    // between different Slang IR instructions in one block. For two effects inside one inline
    // SPIR-V assembly expression, we compare the ordered SPIR-V child instructions separately.
    HashSet<IRBlock*> blocksWithDefiniteWrite;
    HashSet<IRInst*> executableInstructionsWithDefiniteWrite;
    for (auto write : definiteWrites)
    {
        auto executableWrite = getExecutableInstructionForEffect(write);
        if (auto block = as<IRBlock>(executableWrite->getParent()))
        {
            blocksWithDefiniteWrite.add(block);
            executableInstructionsWithDefiniteWrite.add(executableWrite);
        }
    }

    // We make one narrow exception for https://github.com/shader-slang/slang/issues/12545. A read
    // used only by `WaveReadLaneFirst()` is safe when every path through the elected-lane region
    // performs a definite write before reconvergence and the read occurs afterward. If the active
    // lane set remains stable, the builtin broadcasts from the same elected lane that performed the
    // write and cannot observe the earlier uninitialized value.
    //
    // This proof considers only the single-thread CFG. It does not model a change to the wave's
    // active-lane mask between the guard and the read, such as an intervening lane-dependent
    // `discard` or `return`. Slang has no general dynamic-uniformity or wave-reconvergence analysis
    // that could prove such cases.
    for (auto& guard : waveElection.guards)
    {
        if (!isEveryPathFromBlockedByWrite(
                guard.trueBlock,
                guard.mergeBlock,
                blocksWithDefiniteWrite))
            continue;
        for (Index i = 0; i < reads.getCount();)
        {
            auto read = reads[i];
            auto executableRead = getExecutableInstructionForEffect(read);
            auto block = as<IRBlock>(executableRead->getParent());
            if (block && waveElection.dominatorTree->dominates(guard.mergeBlock, block) &&
                isWaveReadLaneFirstUse(executableRead))
            {
                reads.fastRemoveAt(i);
            }
            else
            {
                i++;
            }
        }
    }

    if (reads.getCount() == 0)
        return;

    // A read and a write can share a block, so block reachability alone is insufficient. We record
    // which reads follow a definite write in their own block. When both effects occur inside the
    // same inline assembly expression, the ordered SPIR-V child instructions decide which comes
    // first.
    HashSet<IRInst*> readHasPriorDefiniteWriteInBlock;
    for (auto read : reads)
    {
        auto executableRead = getExecutableInstructionForEffect(read);
        auto block = as<IRBlock>(executableRead->getParent());
        if (!block)
            continue;
        if (!blocksWithDefiniteWrite.contains(block))
            continue;
        for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (inst == executableRead)
            {
                for (auto write : definiteWrites)
                {
                    if (getExecutableInstructionForEffect(write) == inst &&
                        isEarlierSPIRVAssemblyEffect(write, read))
                    {
                        readHasPriorDefiniteWriteInBlock.add(read);
                        break;
                    }
                }
                break;
            }
            if (executableInstructionsWithDefiniteWrite.contains(inst))
            {
                readHasPriorDefiniteWriteInBlock.add(read);
                break;
            }
        }
    }

    // We retain an existing permissive exception intended for constant-trip-count loops that
    // initialize an array or vector element by element. A structural CFG analysis includes a
    // zero-trip path and would report noisy false positives after loops such as
    // `[ForceUnroll] for (i) result[i] = ...;`.
    //
    // When the loop body contains a definite write, we therefore stop uninitialized state at the
    // loop's break block. Uninitialized state still enters the body, so a read before the first
    // write continues to diagnose the bug reported in #10658.
    HashSet<IRBlock*> suppressedBreakBlocks;
    for (auto block : func->getBlocks())
    {
        auto loop = as<IRLoop>(block->getTerminator());
        if (!loop)
            continue;
        auto breakBlock = loop->getBreakBlock();

        // We collect blocks reachable from the loop target without crossing the break block. If
        // any collected block contains a definite write, we treat the loop as initialized when
        // control reaches the break block.
        //
        // This exception is deliberately permissive. It can miss a conditional write in a loop
        // whose trip count or per-iteration control flow this analysis does not prove. We accept
        // that known limitation to avoid warnings for code that initializes aggregate elements in a
        // loop.
        HashSet<IRBlock*> bodyVisited;
        List<IRBlock*> bodyWork;
        bodyVisited.add(breakBlock); // sentinel: never traverse past the break block
        if (auto target = loop->getTargetBlock())
        {
            if (bodyVisited.add(target))
                bodyWork.add(target);
        }
        bool bodyHasDefiniteWrite = false;
        while (bodyWork.getCount())
        {
            auto b = bodyWork.getLast();
            bodyWork.removeLast();
            if (blocksWithDefiniteWrite.contains(b))
            {
                bodyHasDefiniteWrite = true;
                break;
            }
            for (auto succ : b->getSuccessors())
            {
                if (bodyVisited.add(succ))
                    bodyWork.add(succ);
            }
        }
        if (bodyHasDefiniteWrite)
            suppressedBreakBlocks.add(breakBlock);
    }

    // We now walk forward from the entry while the variable may still be uninitialized. A block
    // containing a definite write is a barrier: the block itself remains reachable in that state,
    // but the state does not propagate to its successors.
    //
    // The resulting set contains blocks reachable from entry along a feasible CFG path with no
    // preceding definite write. `suppressedBreakBlocks` applies the loop exception described above.
    // We track CFG edges rather than blocks so that each worklist item retains the predecessor that
    // `findInfeasibleBranchFromPredecessor` needs to remove contradictory short-circuit edges. A
    // block enters the set when any feasible edge reaches it before a definite write.
    //
    // Each worklist item is a `(predecessor, block)` edge. The synthetic entry edge has no
    // predecessor.
    HashSet<IRBlock*> blocksReachableWithoutDefiniteWrite;
    HashSet<KeyValuePair<IRBlock*, IRBlock*>> visitedEdges;
    List<KeyValuePair<IRBlock*, IRBlock*>> worklist;

    auto enqueueEdge = [&](IRBlock* pred, IRBlock* succ)
    {
        // We do not propagate uninitialized state to a loop's break block when the loop exception
        // above established a write in its body.
        if (suppressedBreakBlocks.contains(succ))
            return;
        KeyValuePair<IRBlock*, IRBlock*> edge(pred, succ);
        if (visitedEdges.add(edge))
        {
            blocksReachableWithoutDefiniteWrite.add(succ);
            worklist.add(edge);
        }
    };

    if (auto entry = func->getFirstBlock())
        enqueueEdge(nullptr, entry);

    while (worklist.getCount())
    {
        auto edge = worklist.getLast();
        worklist.removeLast();
        IRBlock* pred = edge.key;
        IRBlock* block = edge.value;

        // A definite write in this block blocks uninitialized state from its successors.
        if (blocksWithDefiniteWrite.contains(block))
            continue;

        // We prune the successor that contradicts the constant passed by this predecessor.
        IRBlock* infeasibleSucc = pred ? findInfeasibleBranchFromPredecessor(block, pred) : nullptr;

        for (auto succ : block->getSuccessors())
        {
            if (succ == infeasibleSucc)
                continue;
            enqueueEdge(block, succ);
        }
    }

    // We retain a read when a path without a definite write reaches its block and no definite write
    // precedes the read inside that block.
    for (Index i = 0; i < reads.getCount();)
    {
        auto executableRead = getExecutableInstructionForEffect(reads[i]);
        auto block = as<IRBlock>(executableRead->getParent());
        bool definitelyInitialized = !block ||
                                     !blocksReachableWithoutDefiniteWrite.contains(block) ||
                                     readHasPriorDefiniteWriteInBlock.contains(reads[i]);
        if (definitelyInitialized)
            reads.fastRemoveAt(i);
        else
            i++;
    }
}

/// Collect reads and writes reached through SSA projections, storage-access transfers, and branch
/// arguments.
///
/// Add every possible write to `possibleWrites`. When `definiteWrites` is non-null, also add each
/// instruction that this analysis treats as a definite initialization.
///
/// Inferred effects do not distinguish one field or element path from another. A caller that knows
/// whether an exact use reads or writes the complete tracked value can use `useEffects` to replace
/// that approximation for that use. This analysis also does not recover an address after code
/// stores it in memory and later loads it; `getTrackedValueFlow` explains why that would require
/// additional memory analysis.
static void collectTrackedReadsAndWrites(
    IRInst* inst,
    List<IRInst*>& possibleWrites,
    List<IRInst*>& reads,
    ConstArrayView<UninitializedVariableUseEffect> useEffects = {},
    List<IRInst*>* definiteWrites = nullptr)
{
    // We reject duplicate effect entries before classifying uses because classification stops at
    // the first entry for a use. We then classify every reachable use exactly once.
    HashSet<IRUse*> declaredUseEffects;
    for (auto const& effect : useEffects)
        SLANG_RELEASE_ASSERT(effect.use && declaredUseEffects.add(effect.use));
    HashSet<IRUse*> consumedUseEffects;

    auto flow = getTrackedValueFlow(inst);

    for (auto flowInst : flow.instructions)
    {
        // TODO: Retain the field and element path of each storage-flow instruction so that inferred
        // effects can distinguish a write that covers a later read from a write to a different
        // subobject.
        for (auto use = flowInst->firstUse; use; use = use->nextUse)
            collectInstructionUse(
                possibleWrites,
                definiteWrites,
                reads,
                use,
                flow.hasStorageFlow(flowInst),
                flow.hasDefiniteStorageFlow(flowInst),
                useEffects,
                &consumedUseEffects);
    }

    for (auto const& effect : useEffects)
        SLANG_RELEASE_ASSERT(consumedUseEffects.contains(effect.use));

    // The phi model treats an independent incoming SSA value as both a possible and a definite
    // write on its edge. As documented by `collectPhiMergeWrites`, that classification remains
    // projection-insensitive.
    collectPhiMergeWrites(flow, possibleWrites);
    if (definiteWrites)
        collectPhiMergeWrites(flow, *definiteWrites);
}

/// Return reads of `inst` that no possible write can reach, including normal returns from `func`.
static List<IRInst*> getUnresolvedParamReads(
    ReachabilityContext& reachability,
    IRFunc* func,
    IRInst* inst)
{
    // We treat returns as reads because an unresolved output parameter must hold a value whenever
    // the function returns normally. Generic assembly remains a possible write because its operand
    // directions are not represented in the IR.
    List<IRInst*> possibleWrites;
    List<IRInst*> reads;

    collectTrackedReadsAndWrites(inst, possibleWrites, reads);

    for (const auto& b : func->getBlocks())
    {
        collectGenericAssemblyPossibleWrites(possibleWrites, b);

        auto t = b->getTerminator();
        if (as<IRReturn>(t))
            reads.add(t);
    }

    retainReadsUnreachableFromPossibleWrites(reachability, possibleWrites, reads);

    return reads;
}

/// An `UninitializedReadSets` value separates the two classes of uninitialized reads reported for
/// one tracked variable.
struct UninitializedReadSets
{
    /// Reads that no possible write can reach (diagnostics 41016 and 41033).
    List<IRInst*> uninitializedReads;

    /// Other reads reachable along a path with no write classified as definite (41035 and 41036).
    List<IRInst*> possiblyUninitializedReads;
};

/// Return all reads of `inst` that can observe an uninitialized value.
static UninitializedReadSets getUninitializedReads(
    ReachabilityContext& reachability,
    IRGlobalValueWithCode* func,
    IRInst* inst,
    const WaveElectionContext& waveElection,
    ConstArrayView<UninitializedVariableUseEffect> useEffects = {})
{
    // We first retain reads that no possible write can reach; these receive diagnostics 41016 or
    // 41033. For the remaining reads, definite-assignment analysis retains those reachable along a
    // path with no write classified as definite; these receive 41035 or 41036. We remove the first
    // set from the second so that a read receives only one diagnostic.
    List<IRInst*> possibleWrites;
    List<IRInst*> definiteWrites;
    List<IRInst*> allReads;
    collectTrackedReadsAndWrites(inst, possibleWrites, allReads, useEffects, &definiteWrites);

    UninitializedReadSets result;

    result.uninitializedReads = allReads;
    retainReadsUnreachableFromPossibleWrites(
        reachability,
        possibleWrites,
        result.uninitializedReads);

    // With no possible write, every read is already in the first set. With no read, both sets are
    // empty.
    if (possibleWrites.getCount() == 0 || allReads.getCount() == 0)
        return result;

    HashSet<IRInst*> uninitializedReadSet;
    for (auto read : result.uninitializedReads)
        uninitializedReadSet.add(read);

    result.possiblyUninitializedReads = allReads;
    retainReadsNotDefinitelyInitialized(
        func,
        definiteWrites,
        result.possiblyUninitializedReads,
        waveElection);

    for (Index i = 0; i < result.possiblyUninitializedReads.getCount();)
    {
        if (uninitializedReadSet.contains(result.possiblyUninitializedReads[i]))
            result.possiblyUninitializedReads.fastRemoveAt(i);
        else
            i++;
    }

    return result;
}

/// Diagnose every read in `reads` as an uninitialized use of `inst`.
//
// We use the named-variable diagnostic when `inst` has a user-visible name. Compiler-generated
// values instead use the diagnostic that describes their type. The caller chooses either the
// uninitialized diagnostics (41016 and 41033) or the possibly-uninitialized diagnostics (41035 and
// 41036).
template<typename TVarDiag, typename TValDiag>
static void diagnoseUninitializedUses(
    DiagnosticSink* sink,
    IRInst* inst,
    IRType* type,
    const List<IRInst*>& reads)
{
    // Several reads inside one inline assembly expression can refer to the same tracked value. We
    // keep their distinct effect points during the analysis because their order matters, but one
    // source expression should receive only one diagnostic of each class.
    HashSet<IRInst*> diagnosedInstructions;
    bool hasName = inst->findDecoration<IRNameHintDecoration>() != nullptr ||
                   inst->findDecoration<IRLinkageDecoration>() != nullptr;

    for (auto effectPoint : reads)
    {
        auto read = getExecutableInstructionForEffect(effectPoint);
        if (!diagnosedInstructions.add(read))
            continue;

        if (hasName)
        {
            StringBuilder varNameSb;
            printDiagnosticArg(varNameSb, inst);
            sink->diagnose(TVarDiag{
                .varName = varNameSb.produceString(),
                .location = read->sourceLoc,
            });
        }
        else
        {
            StringBuilder typeNameSb;
            printDiagnosticArg(typeNameSb, type);
            sink->diagnose(TValDiag{
                .typeName = typeNameSb.produceString(),
                .location = read->sourceLoc,
            });
        }
    }
}

/// Return whether a possible write to `inst` can reach `reference`.
static bool canPossibleWriteReachInstruction(
    ReachabilityContext& reachability,
    IRInst* inst,
    IRInst* reference)
{
    // Constructor checking intentionally asks the permissive question of whether any write can
    // reach the return. It does not use this helper to prove that every path writes the value.
    List<IRInst*> possibleWrites;
    List<IRInst*> reads;

    auto flow = getTrackedValueFlow(inst);
    for (auto flowInst : flow.instructions)
    {
        for (auto use = flowInst->firstUse; use; use = use->nextUse)
        {
            collectInstructionUse(
                possibleWrites,
                nullptr,
                reads,
                use,
                flow.hasStorageFlow(flowInst),
                flow.hasDefiniteStorageFlow(flowInst));
        }
    }

    for (auto write : possibleWrites)
    {
        if (isEffectReachable(reachability, write, reference))
            return true;
    }

    return false;
}

/// Follow loads back to the storage or undefined value from which `inst` originates.
static IRInst* traceInstOrigin(IRInst* inst)
{
    if (auto load = as<IRLoad>(inst))
        return traceInstOrigin(load->getPtr());

    return inst;
}

/// Return whether `inst`, possibly after a chain of loads, supplies a function return.
static bool isReturnedValue(IRInst* inst)
{
    // Constructor checking handles a returned, initially undefined value field by field. We follow
    // loads because aggregate construction can return a value loaded from local storage.
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        IRInst* user = use->getUser();
        if (as<IRReturn>(user))
            return true;

        IRLoad* load = as<IRLoad>(user);
        if (load && isReturnedValue(load))
            return true;
    }
    return false;
}

/// Return whether an immediate use of `inst` may write it.
static bool hasImmediatePossibleWrite(IRInst* inst)
{
    // This predicate supports the constructor check's permissive policy. A possible write is enough
    // to suppress its field-by-field fallback; we do not claim that the write occurs on every path.
    List<IRInst*> possibleWrites;
    List<IRInst*> reads;
    bool hasStorageFlow = isTrackedStorageRoot(inst);
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        collectInstructionUse(possibleWrites, nullptr, reads, use, hasStorageFlow, hasStorageFlow);
        if (possibleWrites.getCount())
            return true;
    }

    return false;
}

/// Return the fields for which no possible initializing write can reach `ret`.
static List<IRStructField*> findFieldsWithoutPossibleWriteAtReturn(
    ReachabilityContext& reachability,
    IRReturn* ret,
    IRStructType* type)
{
    // We first find the initially undefined value returned on this path. A value produced by
    // another constructor delegates responsibility to that constructor, and a direct possible
    // write satisfies this permissive check. Otherwise, we collect the fields with writes that can
    // reach this return and report every remaining field.
    IRInst* origin = traceInstOrigin(ret->getVal());

    // Another constructor is responsible for initializing a value that this constructor returns.
    if (!isUninitializedValue(origin))
        return {};

    // A direct write handles the complete value under the constructor check's existing policy.
    if (hasImmediatePossibleWrite(origin))
        return {};

    // A field counts as initialized when a possible write to its address can reach this return.
    HashSet<IRStructKey*> usedKeys;
    for (auto use = origin->firstUse; use; use = use->nextUse)
    {
        IRInst* user = use->getUser();

        auto fieldAddress = as<IRFieldAddress>(user);
        if (!fieldAddress || !canPossibleWriteReachInstruction(reachability, user, ret))
            continue;

        IRInst* field = fieldAddress->getField();
        usedKeys.add(as<IRStructKey>(field));
    }

    List<IRStructField*> uninitializedFields;

    auto fields = type->getFields();
    for (auto field : fields)
    {
        if (canSkipInitializationCheck(
                field->getFieldType(),
                InterfaceValueDiagnosticOwner::TypeFlow))
            continue;

        if (!usedKeys.contains(field->getKey()))
            uninitializedFields.add(field);
    }

    return uninitializedFields;
}

/// Diagnose constructor fields for which no possible initializing write reaches a normal return.
static void checkConstructor(IRFunc* func, ReachabilityContext& reachability, DiagnosticSink* sink)
{
    // We first exclude functions that are not struct constructors and unused synthesized
    // constructors. We then inspect each return independently because different return sites can
    // have different writes reaching them. A synthesized constructor points at the field
    // declaration, while a user-written constructor points at the return.
    auto constructor = func->findDecoration<IRConstructorDecoration>();
    if (!constructor)
        return;

    IRStructType* stype = as<IRStructType>(func->getResultType());
    if (!stype)
        return;

    // An unused synthesized constructor cannot expose an uninitialized result to user code.
    bool synthesized = constructor->getSynthesizedStatus();
    if (synthesized && !func->firstUse)
        return;

    // We report synthesized-constructor warnings at the field declaration when possible, because
    // no user-written return exists. For a user-written constructor, we report each warning at the
    // return whose incoming paths lack a possible write to the field.
    auto printWarnings = [&](const List<IRStructField*>& fields, IRReturn* ret)
    {
        for (auto field : fields)
        {
            StringBuilder typeNameSb;
            printDiagnosticArg(typeNameSb, stype);
            StringBuilder fieldNameSb;
            printDiagnosticArg(fieldNameSb, field->getKey());
            if (synthesized)
            {
                // A linked struct can have no source location on its field key. We fall back to the
                // struct and then the constructor so that the warning still identifies source code.
                SourceLoc loc = field->getKey()->sourceLoc;
                if (!loc.isValid())
                    loc = stype->sourceLoc;
                if (!loc.isValid())
                    loc = func->sourceLoc;
                sink->diagnose(Diagnostics::FieldNotDefaultInitialized{
                    .typeName = typeNameSb.produceString(),
                    .fieldName = fieldNameSb.produceString(),
                    .location = loc,
                });
            }
            else
            {
                sink->diagnose(Diagnostics::ConstructorUninitializedField{
                    .fieldName = fieldNameSb.produceString(),
                    .location = ret->sourceLoc,
                });
            }
        }
    };

    // Each return describes one constructor exit whose initialized fields may differ.
    for (auto block : func->getBlocks())
    {
        for (auto inst = block->getFirstInst(); inst; inst = inst->next)
        {
            auto ret = as<IRReturn>(inst);
            if (!ret)
                continue;

            auto fields = findFieldsWithoutPossibleWriteAtReturn(reachability, ret, stype);
            printWarnings(fields, ret);
        }
    }
}

/// Diagnose reads or normal returns that no possible write to `param` can reach.
static void checkParameterAsOut(
    ReachabilityContext& reachability,
    IRFunc* func,
    IRParam* param,
    DiagnosticSink* sink)
{
    // We diagnose a read that no possible write can reach as an immediate use of an uninitialized
    // `out` parameter. We also model each normal return as a read, which diagnoses a return when no
    // possible write to the parameter can reach it. This check is intentionally permissive: it does
    // not prove that every path to a return contains a write.
    auto reads = getUnresolvedParamReads(reachability, func, param);
    HashSet<IRInst*> diagnosedInstructions;
    for (auto effectPoint : reads)
    {
        auto read = getExecutableInstructionForEffect(effectPoint);
        if (!diagnosedInstructions.add(read))
            continue;

        StringBuilder paramNameSb;
        printDiagnosticArg(paramNameSb, param);
        if (as<IRTerminatorInst>(read))
        {
            sink->diagnose(Diagnostics::ReturningWithUninitializedOut{
                .paramName = paramNameSb.produceString(),
                .location = read->sourceLoc,
            });
        }
        else
        {
            sink->diagnose(Diagnostics::UsingUninitializedOut{
                .paramName = paramNameSb.produceString(),
                .location = read->sourceLoc,
            });
        }
    }
}

/// Diagnose uninitialized parameter, local, and constructor-result uses in `func`.
static void checkUninitializedValues(IRFunc* func, DiagnosticSink* sink)
{
    // We build control-flow information once, then perform three checks. First, we check parameters
    // whose direction imposes an initialization contract. Second, we analyze every undefined value
    // and local variable in the function. Finally, we apply the field-level constructor check.
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return;

    ReachabilityContext reachability(func);

    // Wave-election structure depends only on the function, so all per-value analyses share it.
    auto waveElection = collectWaveElectionContext(func);

    // We diagnose a constructor's returned value field by field instead of reporting one warning
    // for the complete value.
    auto constructor = func->findDecoration<IRConstructorDecoration>();

    // We skip parameter checks for a function explicitly marked as unmodifying.
    if (!isUnmodifying(func))
    {
        for (auto param : firstBlock->getParams())
        {
            if (shouldCheckParameterAsOut(param))
                checkParameterAsOut(reachability, func, param, sink);
        }
    }

    // We analyze each value that begins without a source-level initializer.
    for (auto block : func->getBlocks())
    {
        for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (!isUninitializedValue(inst))
                continue;

            // The constructor check below examines a returned undefined value field by field.
            if (constructor && isReturnedValue(inst))
                continue;

            IRType* type = getTrackedValueType(inst);
            if (canSkipInitializationCheck(type, InterfaceValueDiagnosticOwner::TypeFlow))
                continue;

            // We collect both diagnostic classes from one set of reads and writes. The first class
            // has no reachable possible write. The second has a possible write, but some path
            // reaches the read without a write that this checker classifies as definite.
            auto reads = getUninitializedReads(reachability, func, inst, waveElection);

            diagnoseUninitializedUses<
                Diagnostics::UsingUninitializedVariable,
                Diagnostics::UsingUninitializedValue>(sink, inst, type, reads.uninitializedReads);

            diagnoseUninitializedUses<
                Diagnostics::PossiblyUsingUninitializedVariable,
                Diagnostics::PossiblyUsingUninitializedValue>(
                sink,
                inst,
                type,
                reads.possiblyUninitializedReads);
        }
    }

    // Constructor diagnostics require return-specific field information.
    checkConstructor(func, reachability, sink);
}

/// Return whether host code supplies the value of `variable` through an exported C++ symbol.
static bool isHostProvidedGlobal(IRGlobalVar* variable)
{
    // CPU code can declare `__global public __extern_cpp int myGlobal;` and let its host write the
    // unmangled exported symbol. Such a declaration has no in-module initializer by design.
    if (!variable->findDecoration<IRExternCppDecoration>())
        return false;

    return variable->findDecoration<IRExportDecoration>() ||
           variable->findDecoration<IRHLSLExportDecoration>() ||
           variable->findDecoration<IRPublicDecoration>();
}

/// Diagnose reads from a global that has no in-module or external source of initialization.
static void checkUninitializedGlobals(IRGlobalVar* variable, DiagnosticSink* sink)
{
    // We first defer checking each resource global marked for replacement to
    // `legalizeResourceGlobalVars`. We then exclude globals whose type or decorations say that
    // another part of the compilation supplies their value. Finally, we look for an initializer
    // block or any possible write. Only when none exists do we diagnose each read from the global.
    if (isFileOrNamespaceScopeStaticResourceGlobalToReplace(variable))
    {
        // The linked pass checks the generated entry-point local after inspecting the combined
        // module and rejecting functions whose invocations cannot all be rewritten. Diagnosing the
        // original global during this earlier module-wide check would report the same source read
        // twice.
        return;
    }

    IRType* type = getTrackedValueType(variable);
    if (canSkipInitializationCheck(type, InterfaceValueDiagnosticOwner::UninitializedValueAnalysis))
        return;

    // A semantic, global-input, or hit-attribute decoration identifies externally supplied input.
    if (variable->findDecoration<IRSemanticDecoration>())
        return;

    if (variable->findDecoration<IRGlobalInputDecoration>())
        return;

    if (variable->findDecoration<IRVulkanHitAttributesDecoration>())
        return;

    if (isHostProvidedGlobal(variable))
        return;

    // A child block is the global's initializer computation.
    for (auto inst : variable->getChildren())
    {
        if (as<IRBlock>(inst))
            return;
    }

    auto flow = getTrackedValueFlow(variable);

    List<IRInst*> possibleWrites;
    List<IRInst*> reads;
    for (auto flowInst : flow.instructions)
    {
        for (auto use = flowInst->firstUse; use; use = use->nextUse)
        {
            collectInstructionUse(
                possibleWrites,
                nullptr,
                reads,
                use,
                flow.hasStorageFlow(flowInst),
                flow.hasDefiniteStorageFlow(flowInst));
        }
    }
    if (possibleWrites.getCount())
        return;

    // We keep nested SPIR-V operations as separate effect points while analyzing their order. We
    // report at most one warning for the enclosing source expression, which is the executable
    // instruction with a source location.
    HashSet<IRInst*> diagnosedInstructions;
    for (auto effectPoint : reads)
    {
        auto read = getExecutableInstructionForEffect(effectPoint);
        if (!diagnosedInstructions.add(read))
            continue;

        StringBuilder varNameSb;
        printDiagnosticArg(varNameSb, variable);
        sink->diagnose(Diagnostics::UsingUninitializedGlobalVariable{
            .varName = varNameSb.produceString(),
            .location = read->sourceLoc,
        });
    }
}

void checkForUsingUninitializedVariable(
    IRGlobalValueWithCode* code,
    IRInst* variable,
    ConstArrayView<UninitializedVariableUseEffect> useEffects,
    DiagnosticSink* sink)
{
    // The module-wide check cannot see a local created afterward. We rebuild reachability for its
    // function and run the same per-variable analysis. For uses listed in `useEffects`, we use the
    // caller's facts instead of inferring an effect from the lowered instruction. The variable must
    // belong to that function because its control-flow graph defines the paths we analyze.
    SLANG_RELEASE_ASSERT(isChildInstOf(variable, code));
    ReachabilityContext reachability(code);
    auto waveElection = collectWaveElectionContext(code);
    auto reads = getUninitializedReads(reachability, code, variable, waveElection, useEffects);
    auto type = getTrackedValueType(variable);

    diagnoseUninitializedUses<
        Diagnostics::UsingUninitializedVariable,
        Diagnostics::UsingUninitializedValue>(sink, variable, type, reads.uninitializedReads);
    diagnoseUninitializedUses<
        Diagnostics::PossiblyUsingUninitializedVariable,
        Diagnostics::PossiblyUsingUninitializedValue>(
        sink,
        variable,
        type,
        reads.possiblyUninitializedReads);
}

void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink)
{
    // We check functions stored directly in the module, functions returned by generics, and global
    // variables. Other module-scope instructions cannot introduce a source-level value covered by
    // this analysis.
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            checkUninitializedValues(func, sink);
        }
        else if (auto generic = as<IRGeneric>(inst))
        {
            auto retVal = findGenericReturnVal(generic);
            if (auto funcVal = as<IRFunc>(retVal))
                checkUninitializedValues(funcVal, sink);
        }
        else if (auto global = as<IRGlobalVar>(inst))
        {
            checkUninitializedGlobals(global, sink);
        }
    }
}
} // namespace Slang
