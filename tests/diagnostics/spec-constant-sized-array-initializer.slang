// No target can build an array sized by a specialization constant from an initializer list, so
// SPIR-V and GLSL, which accept the declarations, report the initializer (#14030).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

[SpecializationConstant]
const int kTile = 4;

struct Data
{
    float values[kTile];
//CHECK:        ^ error E55223
//CHECK:        ^ initialized from a list
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    // The default value of `Data.values` is reported at the field.
    Data d = {};
    int local[kTile] = {};
//CHECK:               ^ error E55223
//CHECK:               ^ initialized from a list
    local[tid % kTile] = 5;
    for (int i = 0; i < kTile; i++)
        outputBuffer[i] = d.values[i] + local[i];
}
