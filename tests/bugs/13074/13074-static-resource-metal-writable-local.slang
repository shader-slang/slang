// Resource-global replacement creates an entry-point local for `selectedTexture`. The conditional
// assignment prevents SSA simplification from removing that local. Metal has no `thread`
// representation for the remaining texture storage, so Slang must diagnose the program instead of
// emitting invalid MSL.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target metal -entry main -stage compute -validate-ir

RWTexture2D<float4> textureA;
RWTexture2D<float4> textureB;
uniform bool selectSecondTexture;
static RWTexture2D<float4> selectedTexture = textureA;
//CHECK:                   ^^^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//CHECK:                   ^^^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'Texture2D', which target 'metal' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable

[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    if (selectSecondTexture)
        selectedTexture = textureB;
    selectedTexture[threadID.xy] = 1.0;
}
