#include "slang-ir-insts-stable-names.h"

namespace Slang
{

static constexpr UInt kOpcodeToStableName[] = {
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

static constexpr IROp kStableNameToOpcode[] = {
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

#define SLANG_CHECK_IR_STABLE_NAME(op, stableName)        \
    static_assert(kOpcodeToStableName[op] == stableName); \
    static_assert(kStableNameToOpcode[stableName] == op)

SLANG_CHECK_IR_STABLE_NAME(kIROp_NodeLaunchDecoration, 867);
SLANG_CHECK_IR_STABLE_NAME(kIROp_NodeMaxDispatchGridDecoration, 868);
SLANG_CHECK_IR_STABLE_NAME(kIROp_NodeDispatchGridDecoration, 869);
SLANG_CHECK_IR_STABLE_NAME(kIROp_MaxRecordsDecoration, 870);
SLANG_CHECK_IR_STABLE_NAME(kIROp_NodeOutputRecordGetElementPtr, 871);
SLANG_CHECK_IR_STABLE_NAME(kIROp_GetEnumBarrierMemoryTypeFlags, 872);
SLANG_CHECK_IR_STABLE_NAME(kIROp_GetEnumBarrierSemanticFlags, 873);
SLANG_CHECK_IR_STABLE_NAME(kIROp_WorkGraphRecordTypeDecoration, 874);
SLANG_CHECK_IR_STABLE_NAME(kIROp_NodeIDDecoration, 875);
SLANG_CHECK_IR_STABLE_NAME(kIROp_NodeIsProgramEntryDecoration, 876);
SLANG_CHECK_IR_STABLE_NAME(kIROp_NodeArraySizeDecoration, 877);
SLANG_CHECK_IR_STABLE_NAME(kIROp_AllowSparseNodesDecoration, 878);
SLANG_CHECK_IR_STABLE_NAME(kIROp_WorkGraphRecordElementTypeDecoration, 879);
SLANG_CHECK_IR_STABLE_NAME(kIROp_DispatchNodeInputRecordType, 880);
SLANG_CHECK_IR_STABLE_NAME(kIROp_ThreadNodeInputRecordType, 881);
SLANG_CHECK_IR_STABLE_NAME(kIROp_GroupNodeInputRecordsType, 882);
SLANG_CHECK_IR_STABLE_NAME(kIROp_EmptyNodeInputType, 883);
SLANG_CHECK_IR_STABLE_NAME(kIROp_ThreadNodeOutputRecordsType, 884);
SLANG_CHECK_IR_STABLE_NAME(kIROp_GroupNodeOutputRecordsType, 885);
SLANG_CHECK_IR_STABLE_NAME(kIROp_NodeOutputType, 886);
SLANG_CHECK_IR_STABLE_NAME(kIROp_NodeOutputArrayType, 887);
SLANG_CHECK_IR_STABLE_NAME(kIROp_EmptyNodeOutputType, 888);
SLANG_CHECK_IR_STABLE_NAME(kIROp_EmptyNodeOutputArrayType, 889);

#undef SLANG_CHECK_IR_STABLE_NAME

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
