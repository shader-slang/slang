module "module-a.slang";

// This file deliberately includes a long comment to test the original bug.
// The bug in issue 9109 was that pragma warnings could be incorrectly
// rejected in one module based on the length of comments in another module.
// The longer this comment, the more it would trigger the bug if it weren't fixed.
// This helps ensure that pragma warning states are properly isolated between modules.

public bool foo(float a, out float b) {
#pragma warning(disable: 41018) // Disable warning 41018 : returning without initializing some variables/parameters
    if (a > 0) return true;
#pragma warning(default: 41018)
    b = 0;
    return false;
}
