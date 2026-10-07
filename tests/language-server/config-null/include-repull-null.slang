//TEST:LANG_SERVER(config-pull, filecheck=CHECK):

// After workspace search has been turned off, the client removes the setting and asks the server
// to pull its settings again. The client now answers null for every section, so workspace search
// returns to its built-in default and the include target in lib/, a folder that is never opened,
// resolves again.

#include "configNullHelper.slang"

int useHelper()
{
//CONFIG:slang.searchInAllWorkspaceDirectories=false
//HOVER:16,12
//CONFIG_REPULL
//HOVER:16,12
    return configNullHelperValue();
}

// CHECK: config: slang.searchInAllWorkspaceDirectories=false
// CHECK-NEXT: --------
// CHECK-NEXT: null
// CHECK: config: re-pulled
// CHECK: func configNullHelperValue() -> int
