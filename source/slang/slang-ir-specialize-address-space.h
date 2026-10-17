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

    /// Whether `specializeAddressSpace` should reconcile local pointer slots: retype each local
    /// pointer slot's contained address space to the pointers stored into it, and diagnose a slot
    /// written pointers in two different concrete classes. Only SPIR-V opts in, because only its
    /// split of logical and physical pointers makes such a merged slot ill-typed. Metal and WGSL do
    /// assign pointer address spaces in this pass, but a merged slot is not a type error for them,
    /// and GLSL's assigner infers nothing -- so on those targets reconciling slots would be a
    /// SPIR-V-shaped change with no correctness benefit, hence disabled by default.
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
/// does), this additionally reconciles local pointer slots and diagnoses a single local pointer
/// slot written pointer values in two different concrete address spaces: reported as
/// `inconsistent-pointer-address-space` (E58003) when the slot's value is only used locally, or as
/// `conflicting-return-pointer-storage-classes` (E58005) when a load of the slot is directly
/// returned.
void specializeAddressSpace(
    IRModule* module,
    InitialAddressSpaceAssigner* addrSpaceAssigner,
    DiagnosticSink* sink = nullptr);

/// Traverse the user graph of the initial insts and fix up address spaces to make sure they are
/// consistent. This is needed after inlining a callee, the address space of the callee's
/// instructions should be propagated from the arguments.
void propagateAddressSpaceFromInsts(List<IRInst*>&& initialArgs);

} // namespace Slang
