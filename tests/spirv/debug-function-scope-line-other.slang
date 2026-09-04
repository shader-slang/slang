// Second top-level file for debug-function-scope-line.slang, holding the `#line`-remapped function.
// No //TEST: directive — it is passed on that test's command line.
//
// Keep the `#line`-remapped function here rather than in the file holding the entry point. The
// module-global scope a DebugFunction falls back to is pinned to the entry point's compilation unit,
// so if this function moved into the entry-point file the broken fallback and the correct answer would
// be the same inst, and the test would pass even when the resolution is broken.

#line 1 "debug-function-scope-line-presumed.slang"
float remappedHelper(float x)
{
    return x * 4.0;
}

#line 2000
float plainHelperInSameFile(float x)
{
    return x + 1.0;
}
