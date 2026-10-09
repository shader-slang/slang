//TEST:LANG_SERVER(config-pull, filecheck=CHECK):

// The client supplies a non-default value for `slang.searchInAllWorkspaceDirectories` and answers
// null for every other section. The server applies that value, so workspace search is off and the
// include target in lib/, a folder that is never opened, does not resolve. This checks that a
// reply value takes effect, which the all-null test cannot show on its own.

//CONFIG_REPLY:slang.searchInAllWorkspaceDirectories=false

#include "configNullHelper.slang"

int useHelper()
{
//HOVER:15,12
    return configNullHelperValue();
}

// CHECK: --------
// CHECK-NEXT: null
// CHECK-NOT: configNullHelperValue
