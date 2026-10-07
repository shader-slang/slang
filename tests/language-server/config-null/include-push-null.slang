//TEST:LANG_SERVER(filecheck=CHECK):

// A `workspace/didChangeConfiguration` notification that sets a setting to null restores its
// built-in default. Turning workspace search off hides the include target in lib/, a folder that
// is never opened; setting it to null turns workspace search back on.

#include "configNullHelper.slang"

int useHelper()
{
//CONFIG:slang.searchInAllWorkspaceDirectories=false
//HOVER:15,12
//CONFIG:slang.searchInAllWorkspaceDirectories=null
//HOVER:15,12
    return configNullHelperValue();
}

// CHECK: config: slang.searchInAllWorkspaceDirectories=false
// CHECK-NEXT: --------
// CHECK-NEXT: null
// CHECK: config: slang.searchInAllWorkspaceDirectories=null
// CHECK: func configNullHelperValue() -> int
