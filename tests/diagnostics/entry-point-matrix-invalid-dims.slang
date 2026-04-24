// Entry-point coverage for issue #9450: invalid matrix row/column counts.
// The matrix-dimension check fires at entry-point varying sites only; out-of-
// range matrix types can still be declared and used internally on targets
// that can support them.

//DIAGNOSTIC_TEST:SIMPLE(diag=PARAM,non-exhaustive):-target spirv -entry matDimParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=RET,non-exhaustive):-target spirv -entry matDimReturn -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD,non-exhaustive):-target spirv -entry matDimField -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=ZERO,non-exhaustive):-target spirv -entry matDimZero -stage vertex
//TEST:SIMPLE(filecheck=BOUNDS): -target spirv -entry matDimBounds -stage vertex

// R=4, C=5 (from issue #9450 reproducer)
[shader("vertex")]
float4 matDimParam(matrix<float, 4, 5> m : A) : SV_Position
//PARAM: matrix row and column counts must be between 1 and 4
{
    return float4(0);
}

// Return-type variant
[shader("vertex")]
matrix<float, 4, 5> matDimReturn()
//RET: matrix row and column counts must be between 1 and 4
{
    return {};
}

// Struct field with bad matrix dimensions reached through varying recursion
struct BadMat
{
    matrix<float, 5, 5> m;
}

[shader("vertex")]
float4 matDimField(BadMat b : A) : SV_Position
//FIELD: matrix row and column counts must be between 1 and 4
{
    return float4(0);
}

// Boundary: R=0 is out of range on the low end.
[shader("vertex")]
float4 matDimZero(matrix<float, 0, 1> m : A) : SV_Position
//ZERO: matrix row and column counts must be between 1 and 4
{
    return float4(0);
}

// Boundary: R=1, C=4 and R=4, C=1 are valid and must not trip the rule.
struct BoundsMat
{
    matrix<float, 1, 4> rowVec;
    matrix<float, 4, 1> colVec;
    matrix<float, 4, 4> full;
}

[shader("vertex")]
float4 matDimBounds(BoundsMat b : A) : SV_Position
{
    return b.full[0];
}
// BOUNDS-NOT: E38050
// BOUNDS-NOT: E38051
// BOUNDS-NOT: matrix row and column counts
