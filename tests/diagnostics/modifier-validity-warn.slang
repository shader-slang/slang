// A geometry-shader input primitive qualifier (`triangle`) belongs on a geometry-shader
// parameter, not on an aggregate type declaration, where it is meaningless. The rule for this
// pair is diagnosed leniently (`ErrorInNewLanguageVersion`), so in a language version before 202c
// applying it to a `struct` must warn (E31232) exactly once while still compiling. Exhaustive
// `diag` mode binds the annotation to the `triangle` token, so a spurious extra diagnostic
// anywhere would fail the test. The language version is pinned so this warning case keeps its
// coverage even if the default version advances to 202c (where it would become an error — see
// `modifier-validity-warn-202c-error.slang`).

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target spirv -std 2026

triangle struct MeaninglessQualifier
/*diag:
^^^^^^^^ modifier not applicable here
^^^^^^^^ modifier 'triangle' is not applicable here.
*/
{
    int x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    MeaninglessQualifier v;
    v.x = 0;
}
