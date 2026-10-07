//TEST:LANG_SERVER(config-pull, filecheck=CHECK):

// The client supplies `slang.searchInAllWorkspaceDirectories` as a flat key and answers null for
// every other section, so the include target in lib/, a folder that is never opened, resolves.

//CONFIG_REPLY:slang.searchInAllWorkspaceDirectories=true

#include "configNullHelper.slang"

int useHelper()
{
//HOVER:13,12
    return configNullHelperValue();
}

// CHECK: func configNullHelperValue() -> int
