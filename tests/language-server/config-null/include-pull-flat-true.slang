//TEST:LANG_SERVER(config-pull, filecheck=CHECK):

// The client supplies `slang.searchInAllWorkspaceDirectories` and answers null for every other
// section, so the include below resolves from lib/, a folder that is never opened.

//CONFIG_REPLY:slang.searchInAllWorkspaceDirectories=true

#include "configNullHelper.slang"

int useHelper()
{
//HOVER:13,12
    return configNullHelperValue();
}

// CHECK: func configNullHelperValue() -> int
