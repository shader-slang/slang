//TEST:LANG_SERVER(config-pull, filecheck=CHECK):

// The client answers JSON null for every section of the server's `workspace/configuration`
// request, as Zed does when no Slang settings are configured. A null means the client has no
// value, so workspace search keeps its default and the include below resolves from lib/, a
// folder other than this file's that is never opened.

#include "configNullHelper.slang"

int useHelper()
{
//HOVER:13,12
    return configNullHelperValue();
}

// CHECK: func configNullHelperValue() -> int
