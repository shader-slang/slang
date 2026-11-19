// Leading-form type-position spellings of the unbounded-array and
// multi-argument-bracket diagnostics. `int[3][]` must re-nest to the same type
// as the declaration spelling `int x[3][]` — an array of three unbounded
// arrays — so it triggers the same arrays-of-non-addressable-type error, and an
// expression-position chain containing a multi-argument bracket keeps its
// existing diagnostic instead of being re-nested.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -stage compute -entry computeMain

int[3][] leadingUnbounded;
//CHECK: arrays of non-addressable type not allowed

static const int G[2][3] = int[2,3][4](1, 2, 3, 4, 5, 6);
//CHECK: multi-dimensional array is not supported

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}
