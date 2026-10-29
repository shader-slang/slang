// WGSL permits override-sized arrays only in workgroup storage; CPU targets reject them all
// (#14030, #10214).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute

[SpecializationConstant]
const int kTile = 4;

static float privateArray[kTile];
//CHECK:     ^^^^^^^^^^^^ error E55219
//CHECK:     ^^^^^^^^^^^^ cannot size an array with a specialization constant

struct Data
{
    float values[kTile];
//CHECK:  ^^^^^^ error E55219
//CHECK:  ^^^^^^ cannot size an array with a specialization constant
}

RWStructuredBuffer<Data> outputBuffer;

void fill(out float result[kTile])
//CHECK:            ^^^^^^ error E55219
//CHECK:            ^^^^^^ cannot size an array with a specialization constant
{
    for (int i = 0; i < kTile; i++)
        result[i] = i;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    float local[kTile];
//CHECK:  ^^^^^ error E55219
//CHECK:  ^^^^^ cannot size an array with a specialization constant
    fill(local);

    Data d;
    for (int i = 0; i < kTile; i++)
    {
        privateArray[i] = local[i];
        d.values[i] = privateArray[i];
    }
    outputBuffer[tid] = d;
}
