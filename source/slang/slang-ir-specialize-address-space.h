// slang-ir-specialize-address-space.h
#pragma once

#include "core/slang-basic.h"

#include <cinttypes>

namespace Slang
{
struct IRModule;
struct IRInst;
class DiagnosticSink;
enum class AddressSpace : uint64_t;

struct AddressSpaceSpecializationContext
{
public:
    virtual AddressSpace getAddrSpace(IRInst* inst) = 0;
};

struct InitialAddressSpaceAssigner
{
    virtual bool tryAssignAddressSpace(IRInst* inst, AddressSpace& outAddressSpace) = 0;
    virtual AddressSpace getAddressSpaceFromVarType(IRInst* type) = 0;
    virtual AddressSpace getLeafInstAddressSpace(IRInst* inst) = 0;

    /// Whether `specializeAddressSpace` should run its local-pointer-slot reconciliation pre-pass:
    /// reconcile each local pointer slot's contained address space to the pointers stored into it,
    /// and diagnose a slot written pointers in two different concrete classes. This is meaningful
    /// only for a target that infers a pointer's address space in this pass -- SPIR-V, whose split
    /// of logical and physical pointers makes such a slot ill-typed. Targets that carry the address
    /// space in the pointer type (Metal/WGSL) leave it disabled.
    virtual bool shouldReconcileLocalPointerSlots() { return false; }
};

struct NoOpInitialAddressSpaceAssigner : public InitialAddressSpaceAssigner
{
    virtual bool tryAssignAddressSpace(IRInst*, AddressSpace&) { return false; }
    virtual AddressSpace getAddressSpaceFromVarType(IRInst* type);
    virtual AddressSpace getLeafInstAddressSpace(IRInst* inst);
};

/// Propagate address space information through the IR module.
/// Specialize functions with reference/pointer parameters to use the correct address space
/// based on the address space of the arguments.
///
/// `sink` (optional) receives diagnostics for target-invalid results this pass detects, such as
/// a function that returns pointers in more than one storage class. Every codegen caller (SPIR-V,
/// GLSL, Metal, WGSL) passes a sink for that return-conflict diagnostic.
///
/// When the assigner opts in via `shouldReconcileLocalPointerSlots` (only the SPIR-V assigner
/// does), this additionally runs a local-pointer-slot reconciliation pre-pass and reports
/// `inconsistent-pointer-address-space` if a single local pointer slot is written pointer values
/// in two different concrete address spaces.
void specializeAddressSpace(
    IRModule* module,
    InitialAddressSpaceAssigner* addrSpaceAssigner,
    DiagnosticSink* sink = nullptr);

/// Traverse the user graph of the initial insts and fix up address spaces to make sure they are
/// consistent. This is needed after inlining a callee, the address space of the callee's
/// instructions should be propagated from the arguments.
void propagateAddressSpaceFromInsts(List<IRInst*>&& initialArgs);

} // namespace Slang
