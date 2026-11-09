//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute

// This test checks initialization across entry-point control flow. The C++ target can represent
// local resource storage, so the expected analysis diagnostic is not replaced by a target-legality
// error.

Texture2D<float4> textures[1];
RWStructuredBuffer<float4> output;
uniform bool selectTexture;
static Texture2D<float4> selectedTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    if (selectTexture)
        selectedTexture = textures[0];
    output[0] = selectedTexture.Load(int3(0, 0, 0));
//CHECK:                            ^ possible use of uninitialized variable
//CHECK:                            ^ variable 'selectedTexture' may be uninitialized on some paths; it is only conditionally assigned
}
