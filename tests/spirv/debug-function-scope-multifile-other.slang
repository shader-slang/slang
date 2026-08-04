// Second top-level (non-included) source file for debug-function-scope-multifile-include.slang.
// No //TEST: directive: it is compiled together with that file as a separate compilation unit,
// and its compilation unit is emitted first — the case that must NOT capture the included helper.

float multifileOtherFunc(float x)
{
    return x + 1.0;
}
