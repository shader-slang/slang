//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry main -stage compute -validate-ir

Texture2D<float4> input0;
Texture2D<float4> input1;
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture = input0;

[noinline]
void replaceThroughRef(inout Texture2D<float4> value)
{
    value = input1;
}

[noinline]
void replaceCachedTexture()
{
    replaceThroughRef(cachedTexture);
}

[numthreads(1, 1, 1)]
void main()
{
    replaceCachedTexture();
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}

// The callee writes the resource through an `inout` parameter, so the call must copy the
// updated resource back to `main`.
//CHECK-LABEL: void replaceCachedTexture_0(
//CHECK: replaceThroughRef_0(&cachedTexture_1,
//CHECK: *cachedTexture_0 = cachedTexture_1;
