//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry main -stage compute -validate-ir

Texture2D<float4> input0;
Texture2D<float4> input1;
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture = input0;

[noinline]
void replaceThroughConstRef(__constref Texture2D<float4> value)
{
    Texture2D<float4>* address = (Texture2D<float4>*)&value;
    *address = input1;
}

[noinline]
void replaceCachedTexture()
{
    replaceThroughConstRef(cachedTexture);
}

[numthreads(1, 1, 1)]
void main()
{
    replaceCachedTexture();
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}

// The explicit pointer cast lets the callee write through a source-level `__constref` parameter.
// The generated parameter for `replaceCachedTexture` must therefore copy the updated resource back
// to `main` instead of treating the call as read-only.
//CHECK-LABEL: void replaceCachedTexture_0(
//CHECK: replaceThroughConstRef_0(&cachedTexture_1,
//CHECK: *cachedTexture_0 = cachedTexture_1;
