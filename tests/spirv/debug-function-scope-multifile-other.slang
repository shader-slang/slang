// Second top-level (non-included) source file for debug-function-scope-multifile-include.slang.
// No //TEST: directive: it is compiled together with that file as a separate compilation unit,
// and its compilation unit is emitted first — the case that must NOT capture the included helper.
//
// Do not delete this file as unused. It is what gives that test its power: the module-global scope
// a DebugFunction falls back to is pinned to the entry point's compilation unit, so with only one
// compilation unit the fallback and the correct answer are the same inst and the test passes even
// when the resolution is broken. A second compilation unit is what makes them differ.

float multifileOtherFunc(float x)
{
    return x + 1.0;
}
