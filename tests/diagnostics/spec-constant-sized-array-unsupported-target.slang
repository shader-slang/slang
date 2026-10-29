// HLSL, CUDA and Metal cannot size an array with a specialization constant (#14030, #10214).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -warnings-disable 39019
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute

[SpecializationConstant]
const int kTile = 4;

groupshared float tile[kTile];
//CHECK:          ^^^^ error E55222
//CHECK:          ^^^^ cannot size an array with a specialization constant

struct Data
{
    float values[kTile];
//CHECK:  ^^^^^^ error E55222
//CHECK:  ^^^^^^ cannot size an array with a specialization constant
}

// The array type here comes from a generic argument, so it exists only after specialization.
struct Box<T>
{
    T value;
/*CHECK:
      ^^^^^ error E55222
      ^^^^^ cannot size an array with a specialization constant
*/
}

RWStructuredBuffer<Data> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint gi: SV_GroupIndex)
{
    float local[kTile];
//CHECK:  ^^^^^ error E55222
//CHECK:  ^^^^^ cannot size an array with a specialization constant
    for (int i = 0; i < kTile; i++)
        local[i] = i;

    tile[gi] = local[gi];
    GroupMemoryBarrierWithGroupSync();

    Box<float[kTile]> box;
    for (int i = 0; i < kTile; i++)
        box.value[i] = tile[(gi + i) % kTile];

    Data d;
    for (int i = 0; i < kTile; i++)
        d.values[i] = box.value[i];
    outputBuffer[gi] = d;
}
