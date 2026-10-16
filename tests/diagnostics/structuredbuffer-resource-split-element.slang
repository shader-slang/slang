// Type legalization reports E38204 at each declaration of a `StructuredBuffer`
// whose element type it would split. These buffers are ones the up-front
// validator does not reject: on Metal it skips every buffer, and on every
// target it does not look inside `static` buffers, nested buffers or
// `DescriptorHandle`s (#13654).

// -warnings-disable 41017: every entry point compiles the uninitialized `static`
// buffer below. 31106/31107: `ConstantBuffer<CBMaterial>` moves its texture to
// another binding slot before the error is reported.
//DIAGNOSTIC_TEST:SIMPLE(diag=CB):     -target metal -warnings-disable 31106,31107,41017 -entry main_cb -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED): -target metal -warnings-disable 41017 -entry main_nested -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC): -target metal -warnings-disable 41017 -entry main_static -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC): -target spirv -warnings-disable 41017 -entry main_static -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC): -target hlsl -warnings-disable 41017 -entry main_static -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC): -target glsl -warnings-disable 41017 -entry main_static -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC): -target wgsl -warnings-disable 41017 -entry main_static -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DH):     -target metal -warnings-disable 41017 -entry main_dh -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DH):     -target spirv -warnings-disable 41017 -entry main_dh -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DH):     -target hlsl -warnings-disable 41017 -entry main_dh -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=APPEND): -target metal -warnings-disable 41017 -entry main_append -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FN):     -target metal -warnings-disable 41017 -entry main_fn -stage compute

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

struct DHMaterial
{
    Texture2D t;
    float4 c;
}

struct FnMaterial
{
    Texture2D t;
    float4 c;
}

struct AppendMaterial
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

static StructuredBuffer<StaticMaterial> staticMaterials;
//STATIC:                               ^^^^^^^^^^^^^^^ resource type in StructuredBuffer
//STATIC:                               ^^^^^^^^^^^^^^^ StructuredBuffer element type 'StaticMaterial' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_static(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = staticMaterials[tid.x].c;
}

uniform DescriptorHandle<StructuredBuffer<DHMaterial>> dhMaterials;
//DH:                                                  ^^^^^^^^^^^ resource type in StructuredBuffer
//DH:                                                  ^^^^^^^^^^^ StructuredBuffer element type 'DHMaterial' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_dh(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = dhMaterials[tid.x].t.Load(int3(0));
}

// A function parameter is reported as a declaration. The local `b` merges the
// two globals in an SSA phi, which is not a declaration and is not reported.
StructuredBuffer<FnMaterial> fnA;
//FN:                        ^^^ resource type in StructuredBuffer
//FN:                        ^^^ StructuredBuffer element type 'FnMaterial' must not be a resource type or contain an opaque handle type
StructuredBuffer<FnMaterial> fnB;
//FN:                        ^^^ resource type in StructuredBuffer
//FN:                        ^^^ StructuredBuffer element type 'FnMaterial' must not be a resource type or contain an opaque handle type

[noinline]
float4 readMaterial(StructuredBuffer<FnMaterial> materials, uint i)
//FN:                                            ^^^^^^^^^ resource type in StructuredBuffer
//FN:                                            ^^^^^^^^^ StructuredBuffer element type 'FnMaterial' must not be a resource type or contain an opaque handle type
{
    return materials[i].c;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main_fn(uint3 tid : SV_DispatchThreadID)
{
    StructuredBuffer<FnMaterial> b = (tid.x > 3) ? fnA : fnB;
    outBuf[tid.x] = readMaterial(b, tid.x);
}

// `AppendStructuredBuffer<S>` is lowered to a generated struct whose `elements`
// field has no location of its own; the report still names the declaration.
AppendStructuredBuffer<AppendMaterial> appendMaterials;
//APPEND:                              ^^^^^^^^^^^^^^^ resource type in StructuredBuffer
//APPEND:                              ^^^^^^^^^^^^^^^ StructuredBuffer element type 'AppendMaterial' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_append(uint3 tid : SV_DispatchThreadID)
{
    AppendMaterial m;
    m.c = float4(0);
    appendMaterials.Append(m);
    appendMaterials.Append(m);
}
