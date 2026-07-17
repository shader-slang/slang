// #include'd by debug-function-scope-multifile-include.slang (no //TEST: directive). The function
// here is part of the including file's compilation unit, not the other top-level file's.

float multifileHelperFromInclude(float x)
{
    return x * 3.0;
}
