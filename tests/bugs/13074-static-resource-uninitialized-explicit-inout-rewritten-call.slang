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

    // We access `fallbackTexture` by name so legalization adds its hidden state parameter to
    // `maybeInitialize`. The caller also passes `cachedTexture` explicitly, so rebuilding the call
    // must preserve both argument roles.
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
//CHECK:                          ^ variable 'cachedTexture' is not definitely initialized before this use
}
