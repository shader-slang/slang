// Metal reports E38204 at the declaration of a `StructuredBuffer` whose element
// type contains a texture even when the texture is reached through another
// buffer type or the buffer is not a shader parameter, because the check is
// made by the type legalization that cannot represent such an element (#13654).

//DIAGNOSTIC_TEST:SIMPLE(diag=CB):     -target metal -warnings-disable 31106,31107,41017 -entry main_cb -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED): -target metal -warnings-disable 41017 -entry main_nested -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC): -target metal -warnings-disable 41017 -entry main_static -stage compute

RWStructuredBuffer<float4> outBuf;

struct CBMaterial
{
    Texture2D t;
    float4 c;
}

struct NestedMaterial
{
    Texture2D t;
    float4 c;
}

struct StaticMaterial
{
    Texture2D t;
    float4 c;
}

StructuredBuffer<ConstantBuffer<CBMaterial>> cbMaterials;
//CB:                                        ^^^^^^^^^^^ resource type in StructuredBuffer
//CB:                                        ^^^^^^^^^^^ StructuredBuffer element type 'cbuffer<CBMaterial>' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_cb(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = cbMaterials[tid.x].c;
}

StructuredBuffer<StructuredBuffer<NestedMaterial>> nestedMaterials;
//NESTED:                                          ^^^^^^^^^^^^^^^ resource type in StructuredBuffer
//NESTED:                                          ^^^^^^^^^^^^^^^ StructuredBuffer element type 'NestedMaterial' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_nested(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = nestedMaterials[0][tid.x].c;
}

// A `static` buffer is not a shader parameter, so the up-front validator does
// not visit it on any target.
static StructuredBuffer<StaticMaterial> staticMaterials;
//STATIC:                               ^^^^^^^^^^^^^^^ resource type in StructuredBuffer
//STATIC:                               ^^^^^^^^^^^^^^^ StructuredBuffer element type 'StaticMaterial' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_static(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = staticMaterials[tid.x].c;
}
