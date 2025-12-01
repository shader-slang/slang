module "module-b.slang";

public bool baz(float a, out float b) {
#pragma warning(disable: 41018) // Disable warning 41018 : returning without initializing some variables/parameters
    if (a > 0) return true;
#pragma warning(default: 41018)
    b = 0;
    return false;
}
