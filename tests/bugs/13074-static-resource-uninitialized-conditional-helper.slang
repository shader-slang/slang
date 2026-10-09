//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target cpp -entry computeMain -stage compute -validate-ir

// This test checks initialization through the call graph. The C++ target can represent local
// resource storage, so the expected analysis diagnostic is not replaced by a target-legality
// error.

Texture2D<float4> textures[1];
RWStructuredBuffer<float4> output;
uniform bool chooseTexture;
static Texture2D<float4> cachedTexture;

[noinline]
void maybeInitialize()
{
    if (chooseTexture)
        cachedTexture = textures[0];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    maybeInitialize();
    output[0] = cachedTexture.Load(int3(0, 0, 0));
//CHECK:                          ^ possible use of uninitialized variable
//CHECK:                          ^ variable 'cachedTexture' may be uninitialized on some paths; it is only conditionally assigned
}
