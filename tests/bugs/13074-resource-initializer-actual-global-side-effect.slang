//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -lang slang -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> texture;
static Texture2D<float4> cachedTexture = texture;
__global float actualValue;
//CHECK: resource-dependent state cannot have externally preserved storage
//CHECK: variable 'actualValue' has storage that must remain externally addressable

float initializeValue()
{
    actualValue = 1.0;
    return cachedTexture.Load(int3(0, 0, 0)).x;
}

static float cachedValue = initializeValue();
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedValue + actualValue;
}
