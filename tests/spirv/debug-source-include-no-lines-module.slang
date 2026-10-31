// Non-test helper module for debug-source-include-no-lines.slang (no //TEST: directive). The
// entry point calls the function below, so this module contributes code, and the header it
// #includes must keep its DebugSource.
module "debug-source-include-no-lines-module";

#include "debug-source-include-no-lines-module-header.slang"

public float includeNoLinesModuleValue(float x)
{
    return x * MODULE_HEADER_SCALE_7203;
}
