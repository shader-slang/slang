//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang.

import specialized_contract;

__specialized_for_target(hlsl)
export uint pickForTarget() { return 3; }
