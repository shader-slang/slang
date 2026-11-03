// Non-test helper module for debug-source-unused-import.slang (no //TEST: directive).
// The compiled entry point of that test uses nothing from this module (only its other, uncompiled
// entry point does), so none of this module's debug records belong in the output.
module "debug-source-unused-import-module";

#include "debug-source-unused-import-header.slang"

public float unusedModuleMarker13982(float x)
{
    return x * 3.0;
}
