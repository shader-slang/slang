//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> textures[1];
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture;

[noinline]
void readThenReplace()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
    cachedTexture = textures[0];
}

[numthreads(1, 1, 1)]
void main()
{
    readThenReplace();
//CHECK:           ^ use of uninitialized variable
//CHECK:           ^ use of uninitialized variable 'cachedTexture'
// The call reads the incoming value and then definitely assigns a replacement, so later reads are
// valid.
    output[1] = cachedTexture.Load(int3(0, 0, 0));
}
