// Metal reports E38204 at the declaration of a `StructuredBuffer<T>` whose
// element type is a struct with a texture field, however the element is read
// (#13654).

//DIAGNOSTIC_TEST:SIMPLE(diag=SAMPLE): -target metal -entry main_sample -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=LOAD):   -target metal -entry main_load -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=LOCAL):  -target metal -entry main_local -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RW):     -target metal -entry main_rw -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=MIXED):  -target metal -entry main_mixed -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PB):     -target metal -entry main_pb -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DUP):    -target metal -entry main_dup -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NEG):    -target metal -entry main_neg -stage compute

RWStructuredBuffer<float4> outBuf;
SamplerState samp;

struct Material
{
    Texture2D t;
}

struct LoadMaterial
{
    Texture2D t;
}

struct LocalMaterial
{
    Texture2D t;
}

struct RWMaterial
{
    Texture2D t;
}

struct Mixed
{
    float4 c;
    Texture2D t;
}

struct PBMaterial
{
    Texture2D t;
}

struct DupMaterial
{
    Texture2D t;
}

struct WithoutResource
{
    float4 c;
}

StructuredBuffer<Material> materials;
//SAMPLE:                  ^^^^^^^^^ resource type in StructuredBuffer
//SAMPLE:                  ^^^^^^^^^ StructuredBuffer element type 'Material' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_sample(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = materials[tid.x].t.SampleLevel(samp, float2(0.5, 0.5), 0);
}

// Element read through `Load(i)`.
StructuredBuffer<LoadMaterial> loadMaterials;
//LOAD:                        ^^^^^^^^^^^^^ resource type in StructuredBuffer
//LOAD:                        ^^^^^^^^^^^^^ StructuredBuffer element type 'LoadMaterial' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_load(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = loadMaterials.Load(tid.x).t.Load(int3(0));
}

// Element copied to a local first.
StructuredBuffer<LocalMaterial> localMaterials;
//LOCAL:                        ^^^^^^^^^^^^^^ resource type in StructuredBuffer
//LOCAL:                        ^^^^^^^^^^^^^^ StructuredBuffer element type 'LocalMaterial' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_local(uint3 tid : SV_DispatchThreadID)
{
    LocalMaterial m = localMaterials[tid.x];
    outBuf[tid.x] = m.t.Load(int3(0));
}

// RWStructuredBuffer.
RWStructuredBuffer<RWMaterial> rwMaterials;
//RW:                          ^^^^^^^^^^^ resource type in StructuredBuffer
//RW:                          ^^^^^^^^^^^ StructuredBuffer element type 'RWMaterial' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_rw(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = rwMaterials[tid.x].t.Load(int3(0));
}

// Only the ordinary field is read; the element type is still invalid.
StructuredBuffer<Mixed> mixed;
//MIXED:                ^^^^^ resource type in StructuredBuffer
//MIXED:                ^^^^^ StructuredBuffer element type 'Mixed' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_mixed(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mixed[tid.x].c;
}

// StructuredBuffer reached through a ParameterBlock, which Metal lowers to an
// argument buffer before type legalization.
struct PB
{
    StructuredBuffer<PBMaterial> mats;
//PB:                            ^^^^ resource type in StructuredBuffer
//PB:                            ^^^^ StructuredBuffer element type 'PBMaterial' must not be a resource type or contain an opaque handle type
}

ParameterBlock<PB> pb;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_pb(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = pb.mats[tid.x].t.SampleLevel(samp, float2(0.5, 0.5), 0);
}

// Two buffers with one element type share an IR type, and each is reported.
StructuredBuffer<DupMaterial> dupA;
//DUP:                        ^^^^ resource type in StructuredBuffer
//DUP:                        ^^^^ StructuredBuffer element type 'DupMaterial' must not be a resource type or contain an opaque handle type
StructuredBuffer<DupMaterial> dupB;
//DUP:                        ^^^^ resource type in StructuredBuffer
//DUP:                        ^^^^ StructuredBuffer element type 'DupMaterial' must not be a resource type or contain an opaque handle type

[shader("compute")]
[numthreads(1, 1, 1)]
void main_dup(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = dupA[tid.x].t.Load(int3(0)) + dupB[tid.x].t.Load(int3(0));
}

// Negative case: no annotations under `NEG`, so any diagnostic fails the test.
StructuredBuffer<WithoutResource> plain;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_neg(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = plain[tid.x].c;
}
