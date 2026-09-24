//TEST:LANG_SERVER:-init-options-array
// Approach B robustness for shader-slang/slang#13179: a client may send a non-object
// `initializationOptions` (LSP `LSPAny` permits arrays/scalars). The server must handle it
// gracefully — `updateConfigFromJSON` returns for a non-object rather than calling getObject, whose
// `type == Object` assertion would otherwise fire (a debug assert / release undefined behavior).
// With the default workspaceFolders handshake the subdir `#include` still resolves, so a normal
// hover result here means the server survived the array payload.
#include "helper.slang"

int useHelper()
{
    return helperValue;
}
//HOVER:12,12
