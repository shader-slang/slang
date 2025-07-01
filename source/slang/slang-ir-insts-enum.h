#pragma once

#include <cstdint>

namespace Slang
{

/* Bit usage of IROp is as follows

           MainOp | Other
Bit range: 0-10   | Remaining bits

For doing range checks (for example for doing isa tests), the value is masked by kIROpMask_OpMask,
such that the Other bits don't interfere. The other bits can be used for storage for anything that
needs to identify as a different 'op' or 'type'. It is currently used currently for storing the
TextureFlavor of a IRResourceTypeBase derived types for example.

TODO: We should eliminate the use of the "other" bits so that the entire value/state
of an instruction is manifest in its opcode, operands, and children.
*/
enum IROp : int32_t
{

#if 0 // FIDDLE TEMPLATE:
% require("source/slang/slang-ir.h.lua").instEnums()
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-ir-insts-enum.h.fiddle"
#endif // FIDDLE END

    /// The total number of valid opcodes
    kIROpCount,

    /// An invalid opcode used to represent a missing or unknown opcode value.
    kIROp_Invalid = kIROpCount,
};
} // namespace Slang
