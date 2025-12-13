module "basic-module-a.slang";

// This module has a long comment to ensure that any absolute location
// offset caused by content length doesn't affect pragma warning handling
// in other modules. The bug in issue 9109 was that pragma warnings could
// be rejected based on the length of comments in unrelated modules.

public bool foo(float a, out float b) {
#pragma warning(disable: 41018) // Disable warning 41018 : returning without initializing some variables/parameters
    if (a > 0) return true;
#pragma warning(default: 41018)
    b = 0;
    return false;
}
