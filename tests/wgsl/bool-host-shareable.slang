//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=NOBOOL):-target wgsl -entry fragMain -stage fragment

// `bool` is not host-shareable in WGSL, so a `bool` (or bool vector, array or matrix)
// in a uniform or storage buffer is stored as `u32` and converted on load and store
// (shader-slang/slang#13590).

uniform bool gFlag;
uniform bool2 gFlags2;
uniform bool3 gFlags3;
uniform bool gArr[4];
uniform bool4 gArr4[2];
uniform bool2x2 gMat;

cbuffer CB
{
    float cbScale;
    bool cbFlag;
};

struct Inner
{
    bool a;
    int b;
};

struct Params
{
    float2 scale;
    bool flag;
    Inner inner;
};

struct ElemSource
{
    bool a[2];
    bool2x2 m;
};

ConstantBuffer<Params> gCB;
ConstantBuffer<ElemSource> gCBElem;
ParameterBlock<Params> gPB;

struct Elem
{
    float x;
    bool f;
    bool a[2];
    bool2x2 m;
};

StructuredBuffer<Elem> gSB;
RWStructuredBuffer<Elem> gRWSB;
RWStructuredBuffer<bool> gRWSBBool;
StructuredBuffer<bool2> gSBBool2;
RWStructuredBuffer<bool2> gRWSBBool2;

// No buffer member or buffer element is declared with a `bool` type.
// NOBOOL-NOT: : bool,
// NOBOOL-NOT: : vec{{[234]}}<bool>,
// NOBOOL-NOT: array<bool
// NOBOOL-NOT: array<vec{{[234]}}<bool>

// CHECK-DAG: var<uniform> globalParams_0 :
// CHECK-DAG: var<uniform> CB_0 :
// CHECK-DAG: var<uniform> gCB_0 :
// CHECK-DAG: var<uniform> gPB_0 :
// CHECK-DAG: var<uniform> entryPointParams_0 :
// CHECK-DAG: gRWSBBool_0 : array<u32>;
// CHECK-DAG: gSBBool2_0 : array<vec2<u32>>;
// CHECK-DAG: gRWSBBool2_0 : array<vec2<u32>>;

// Loads convert the stored integer back to `bool`.
// CHECK-DAG: bool(globalParams_0.gFlag_0)
// CHECK-DAG: vec2<bool>(globalParams_0.gFlags2_0)
// CHECK-DAG: vec3<bool>(globalParams_0.gFlags3_0)
// CHECK-DAG: bool(globalParams_0.gArr_0.data_{{[0-9]+}}[i32(2)].x)
// CHECK-DAG: vec4<bool>(globalParams_0.gArr4_0.data_{{[0-9]+}}[i32(1)])
// CHECK-DAG: vec2<bool>(globalParams_0.gMat_0.data_{{[0-9]+}}[i32(1)].xy)
// CHECK-DAG: bool(CB_0.cbFlag_0)
// CHECK-DAG: bool(gCB_0.flag_0)
// CHECK-DAG: bool(gCB_0.inner_0.a_0)
// CHECK-DAG: bool(gPB_0.flag_0)
// CHECK-DAG: bool(entryPointParams_0.epFlag_0)
// CHECK-DAG: bool(gSB_0[i32(1)].f_0)
// CHECK-DAG: bool(gRWSB_0[i32(1)].f_0)
// CHECK-DAG: bool(gRWSBBool_0[i32(1)])
// CHECK-DAG: vec2<bool>(gSBBool2_0[i32(0)])
// CHECK-DAG: vec2<bool>(gRWSBBool2_0[i32(1)])

// Stores convert `bool` to the stored integer.
// CHECK-DAG: gRWSB_0[i32(0)].f_0 = select(u32(0), u32(1),
// CHECK-DAG: gRWSB_0[i32(2)].a_{{[0-9]+}}.data_{{[0-9]+}}[i32(1)] = select(u32(0), u32(1),
// CHECK-DAG: gRWSB_0[i32(2)].m_{{[0-9]+}}.data_{{[0-9]+}}[i32(1)] = select(vec2<u32>(0), vec2<u32>(1),
// CHECK-DAG: gRWSBBool_0[i32(0)] = select(u32(0), u32(1),
// CHECK-DAG: gRWSBBool2_0[i32(0)] = select(vec2<u32>(0), vec2<u32>(1),

[shader("fragment")]
float4 fragMain(uniform bool epFlag) : SV_Target
{
    float4 r = float4(0);
    if (gFlag) r.x += 1;
    if (gFlags2.y) r.x += 2;
    if (all(gFlags3)) r.x += 4;
    if (gArr[2]) r.y += 1;
    if (gArr4[1].z) r.y += 2;
    if (gMat[1][0]) r.y += 4;
    if (cbFlag) r.z += cbScale;
    if (gCB.flag) r.z += gCB.scale.x;
    if (gCB.inner.a) r.z += gCB.inner.b;
    if (gPB.flag) r.w += gPB.scale.y;
    if (epFlag) r.w += 1;
    if (gSB[1].f) r.w += gSB[0].x;

    gRWSB[0].f = !gRWSB[1].f;

    // The uniform (std140) and storage (std430) copies of these fields have different storage
    // types, so whole-value copies go through the logical type.
    gRWSB[2].a = gCBElem.a;
    gRWSB[2].m = gCBElem.m;

    gRWSBBool[0] = !gRWSBBool[1];
    if (gSBBool2[0].y) r.x += 8;
    gRWSBBool2[0] = !gRWSBBool2[1];
    return r;
}
