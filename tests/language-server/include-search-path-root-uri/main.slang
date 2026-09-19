//TEST:LANG_SERVER:-init-root-uri
// Regression for shader-slang/slang#13179: when a client announces its workspace root only through
// the deprecated `rootUri` handshake (no `workspaceFolders`), slangd must still scan the workspace
// so that a search-path `#include` resolves without the target being opened. `helper.slang` lives
// in a subdirectory that is never opened, so only the workspace-root scan can put it on the include
// search path. Hovering the symbol it defines therefore produces a result only when the root scan
// runs; without the root fallback the hover result is `null`.
#include "helper.slang"

int useHelper()
{
    return helperValue;
}
//HOVER:12,12
