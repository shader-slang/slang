// shader-slang/slang#12558: a geometry-shader input primitive qualifier (`triangle`)
// belongs on a geometry-shader parameter, not on an aggregate type declaration, where it
// is meaningless. Applying it to a `struct` must warn (E31232) exactly once while still
// compiling. Exhaustive `diag` mode binds the annotation to the `triangle` token, so a
// spurious extra diagnostic anywhere would fail the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target spirv

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
