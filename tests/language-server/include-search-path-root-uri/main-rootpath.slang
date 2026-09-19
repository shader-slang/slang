//TEST:LANG_SERVER:-init-root-path
// Companion to main.slang for shader-slang/slang#13179, exercising the other deprecated single-root
// field: the client announces its root through `rootPath` (a plain filesystem path) instead of
// `rootUri`. This drives the distinct `URI::fromLocalFilePath` conversion in the fallback. As in
// main.slang, `inc/helper.slang` is never opened, so its symbol resolves only once the workspace
// root scan runs; without the root fallback the hover result is `null`.
#include "helper.slang"

int useHelper()
{
    return helperValue;
}
//HOVER:11,12
