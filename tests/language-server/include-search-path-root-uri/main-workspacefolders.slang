//TEST:LANG_SERVER:
// Positive control for shader-slang/slang#13179's with/without-workspaceFolders probe: with the
// modern `workspaceFolders` handshake (the harness default), the workspace-root scan already runs,
// so the subdir `#include` resolves and the hover produces a result. The main.slang and
// main-rootpath.slang variants show the rootUri / rootPath fallback reaching the same outcome.
#include "helper.slang"

int useHelper()
{
    return helperValue;
}
//HOVER:10,12
