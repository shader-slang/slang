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

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Data values[2];
    getData(values);
}
