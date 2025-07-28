#include "slang-ir-insts-stable-names.h"

namespace Slang
{

static const UInt kOpcodeToStableName[] = {
#if 0 // FIDDLE TEMPLATE:
% local insts = require("source/slang/slang-ir-insts.lua")
% insts.traverse(function(inst)
%   if inst.is_leaf then
%     if inst.stable_name == nil then 
%       error("Instruction is missing stable name: " .. tostring(inst.struct_name)) 
%     end
%     local stable_name = tostring(inst.stable_name)
      $stable_name,
%   end
% end)
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-ir-insts-stable-names.cpp.fiddle"
#endif // FIDDLE END
};

static const IROp kStableNameToOpcode[] = {
#if 0 // FIDDLE TEMPLATE:
% local insts = require("source/slang/slang-ir-insts.lua")
% for i = 0, insts.max_stable_name do 
%   inst = insts.stable_name_to_inst[i]
%   if inst then
%     local struct_name = inst.struct_name
      kIROp_$struct_name,
%   else
      kIROp_Invalid,
%   end
% end
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 1
#include "slang-ir-insts-stable-names.cpp.fiddle"
#endif // FIDDLE END
};

UInt getOpcodeStableName(IROp op)
{
    // Check if the opcode is within valid range
    if (op >= SLANG_COUNT_OF(kOpcodeToStableName))
    {
        return kInvalidStableName;
    }
    return kOpcodeToStableName[op];
}

IROp getStableNameOpcode(UInt stableName)
{
    // Check if the stable name is within valid range
    if (stableName >= SLANG_COUNT_OF(kStableNameToOpcode))
    {
        return kIROp_Invalid;
    }
    return kStableNameToOpcode[stableName];
}
} // namespace Slang
