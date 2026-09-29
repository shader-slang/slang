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

// The note requires the generic-extent array itself to be the mismatched
// argument. Here the generic-extent array `values` matches both candidates'
// `flexible[M]` at argument 0; the mismatch is at argument 1, where a scalar is
// passed to a fixed-extent array parameter. The generic-extent array is not the
// mismatched slot, so the note must NOT appear.
void twoArrays<let M : uint>(Data flexible[M], Data pair[2]) {}
void twoArrays<let M : uint>(Data flexible[M], Data trio[3]) {}

void mismatchElsewhere<let N : uint>(inout Data values[N], int notAnArray)
{
    //CHECK: E39999
    //CHECK: no overload for 'twoArrays'
    twoArrays(values, notAnArray);
}

// The note also requires the actual and expected array element types to match.
// Here the generic-extent argument is `Data[int(N)]` but the candidates expect
// `Other[1]`/`Other[2]`, so the real problem is the element type, not the
// extent, and the migration note must NOT appear.
struct Other { float w; }
void takeOther(Other values[1]) {}
void takeOther(Other values[2]) {}

void elementTypeMismatch<let N : uint>(inout Data values[N])
{
    //CHECK: E39999
    //CHECK: no overload for 'takeOther'
    takeOther(values);
}

// The migration note names a "generic value parameter"; it must appear exactly
// once (for the `decode` call above) and never for either negative case.
//CHECK-NOT: generic value parameter

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Data values[2];
    getData(values);
}
