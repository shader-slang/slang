// https://github.com/shader-slang/slang/issues/12764
//
// A generic function whose array parameter has a value-parameter extent
// (`Data values[N]`) is checked while `N` is still abstract, so an argument of
// type `Data[int(N)]` cannot select any fixed-extent `decode` overload. The
// resulting "no applicable overload" error is correct but context-free; a note
// should explain that the extent is a generic value parameter checked before
// specialization, and point at the two supported native formulations.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct Data
{
    int value;
}

void decode(Data values[1]) {}
void decode(Data values[2]) {}
void decode(Data values[3]) {}

void getData<let N : uint>(inout Data values[N])
{
    //CHECK: E39999
    //CHECK: no overload for 'decode'
    //CHECK: generic value parameter
    decode(values);
}

// Negative case that discriminates the paired-slot predicate from a naive
// "any generic-extent array argument AND any candidate expecting a
// literal-extent array" test. Here the generic-extent array `vals` matches both
// candidates' `flexible[M]` at argument 0, so it is NOT the mismatched slot; the
// mismatch is at argument 1, where a scalar `int` is passed to a literal-extent
// array parameter (`Data[2]`/`Data[3]`). A candidate DOES expect a literal-extent
// array, and an argument IS a generic-extent array — so the naive predicate
// would wrongly fire — but they are in different slots, so the migration note
// must NOT appear.
void twoArrays<let M : uint>(Data flexible[M], Data pair[2]) {}
void twoArrays<let M : uint>(Data flexible[M], Data trio[3]) {}

void mismatchElsewhere<let N : uint>(inout Data values[N], int notAnArray)
{
    //CHECK: E39999
    //CHECK: no overload for 'twoArrays'
    twoArrays(values, notAnArray);
}

// The migration note names a "generic value parameter"; it must appear exactly
// once (for the `decode` call above) and never for the mismatch-elsewhere case.
//CHECK-NOT: generic value parameter

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Data values[2];
    getData(values);
}
