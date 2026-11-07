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

    /// Return the address space for a still-`Generic` pointer parameter of an exported function.
    /// Ordinary functions get parameter address spaces from their call sites, but an exported
    /// function may have no caller. A concrete result makes `specializeAddressSpace` treat every
    /// exported function as a root, applying the result to its parameters and specializing the
    /// calls in its body. The base `Generic` turns both off, leaving exported functions to be
    /// reached only through calls.
    virtual AddressSpace getDefaultAddressSpaceForExportedFunctionParam();
};

struct NoOpInitialAddressSpaceAssigner : public InitialAddressSpaceAssigner
{
    virtual bool tryAssignAddressSpace(IRInst*, AddressSpace&) { return false; }
    virtual AddressSpace getAddressSpaceFromVarType(IRInst* type);
    virtual AddressSpace getLeafInstAddressSpace(IRInst* inst);
};

/// Propagate address space information through the IR module.
/// Specialize functions with reference/pointer parameters to use the correct address space
/// based on the address space of the arguments. Entry points are the roots of this
/// specialization; when `addrSpaceAssigner` supplies a default for exported-function parameters,
/// exported functions are roots too.
///
/// When `sink` is non-null, additionally runs the local-pointer-slot reconciliation pre-pass and
/// reports `inconsistent-pointer-address-space` if a single local pointer slot is written pointer
/// values in two different concrete address spaces. Only the SPIR-V legalizer passes a sink; with
/// the default `nullptr` the pre-pass and that diagnostic are skipped (the other backends' behavior
/// is unchanged), so a caller that wants the diagnostic must supply a sink.
void specializeAddressSpace(
    IRModule* module,
    InitialAddressSpaceAssigner* addrSpaceAssigner,
    DiagnosticSink* sink = nullptr);

/// Traverse the user graph of the initial insts and fix up address spaces to make sure they are
/// consistent. This is needed after inlining a callee, the address space of the callee's
/// instructions should be propagated from the arguments.
void propagateAddressSpaceFromInsts(List<IRInst*>&& initialArgs);

} // namespace Slang
