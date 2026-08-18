// shader-slang/slang#12558: a geometry-shader input primitive qualifier (`triangle`)
// belongs on a geometry-shader parameter, not on an aggregate type declaration, where it
// is meaningless. Applying it to a `struct` must warn (E31232) while still compiling.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: modifier 'triangle' is not applicable here
triangle struct MeaninglessQualifier
{
    int x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    MeaninglessQualifier v;
    v.x = 0;
}
