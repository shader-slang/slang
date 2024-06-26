// slang-ir-specialize-address-space.h
#pragma once

namespace Slang
{
    struct IRModule;

    /// Propagate address space information through the IR module.
    /// Specialize functions with reference/pointer parameters to use the correct address space
    /// based on the address space of the arguments.
    /// 
    void specializeAddressSpace(
        IRModule*       module);
}
