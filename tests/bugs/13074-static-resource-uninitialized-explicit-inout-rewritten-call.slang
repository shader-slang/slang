//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> textures[1];
RWStructuredBuffer<float4> output;
uniform bool initializeTexture;
static Texture2D<float4> cachedTexture;
static Texture2D<float4> fallbackTexture = textures[0];

[noinline]
void maybeInitialize(inout Texture2D<float4> value)
{
    if (initializeTexture)
        value = textures[0];

    // Because this function reads `fallbackTexture` by name, legalization adds a parameter for
    // that resource. The source call also passes `cachedTexture` as its explicit argument. When we
    // rebuild the call, we must preserve the explicit argument and append the generated one.
    output[0] = fallbackTexture.Load(int3(0, 0, 0));
}

[numthreads(1, 1, 1)]
void main()
{
    maybeInitialize(cachedTexture);
//CHECK:           ^ use of uninitialized variable
//CHECK:           ^ use of uninitialized variable 'cachedTexture'
    output[1] = cachedTexture.Load(int3(0, 0, 0));
//CHECK:                          ^ possible use of uninitialized variable
//CHECK:                          ^ variable 'cachedTexture' may be uninitialized on some paths; it is only conditionally assigned
}
