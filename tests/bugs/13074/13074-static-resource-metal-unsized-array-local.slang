// In this test, `loadTexture` accepts an unsized resource array by value. The
// `transformParamsToConstRef` pass copies `textures` into a local array before the call. Metal
// cannot allocate storage for that copy because the array has no fixed size. This test
// distinguishes that invalid case from the fixed-size local array in `gh-518.slang`, which Metal
// supports.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target metal -entry main -stage compute -validate-ir

uniform Texture2D<float4> textures[];
RWStructuredBuffer<float4> output;

[noinline]
float4 loadTexture(Texture2D<float4> values[], uint index)
{
    return values[index].Load(int3(0, 0, 0));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    output[threadID.x] = loadTexture(textures, threadID.x);
}

//CHECK: target cannot represent an opaque value in mutable local storage
//CHECK: mutable local storage contains an opaque value of type 'Texture2D', which target 'metal' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable
