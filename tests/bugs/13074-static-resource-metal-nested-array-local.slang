// In this test, `loadTexture` accepts a nested resource array by value. The
// `transformParamsToConstRef` pass copies `textures` into a nested local array before the call.
// Existing `metallib` tests confirm that Apple's compiler accepts a local resource array with one
// fixed-size layer. They do not cover nested resource arrays. We therefore keep the nested form
// rejected until a test confirms that Apple's compiler accepts the generated source.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target metal -entry main -stage compute -validate-ir

uniform Texture2D<float4> textures[1][1];
RWStructuredBuffer<float4> output;

[noinline]
float4 loadTexture(Texture2D<float4> values[1][1])
{
    return values[0][0].Load(int3(0, 0, 0));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    output[threadID.x] = loadTexture(textures);
}

//CHECK: target cannot represent an opaque value in mutable local storage
//CHECK: mutable local storage contains an opaque value of type 'Texture2D', which target 'metal' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable
