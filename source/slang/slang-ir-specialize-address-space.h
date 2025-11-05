// slang-ir-specialize-address-space.h
#pragma once

#include "core/slang-basic.h"

#include <cinttypes>

namespace Slang
{
struct IRModule;
struct IRInst;
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
void specializeAddressSpace(IRModule* module, InitialAddressSpaceAssigner* addrSpaceAssigner);

/// Traverse the user graph of the initial insts and fix up address spaces to make sure they are
/// consistent. This is needed after inlining a callee, the address space of the callee's
/// instructions should be propagated from the arguments.
void propagateAddressSpaceFromInsts(List<IRInst*>&& initialArgs);

} // namespace Slang
