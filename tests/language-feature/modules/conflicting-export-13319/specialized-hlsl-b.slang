//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang. Conflicts with
// specialized-hlsl-a.slang when compiling for HLSL.

import specialized_contract;

__specialized_for_target(hlsl)
export uint pickForTarget() { return 4; }
