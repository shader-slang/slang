// Mesh vertex and primitive outputs and fragment inputs must spell an indexed user semantic
// identically, or Metal cannot match the interstage attributes.

//TEST:SIMPLE(filecheck=MESH): -entry meshMain -stage mesh -target metal
//TEST:SIMPLE(filecheck=FRAG): -entry fragMain -stage fragment -target metal
//TEST:SIMPLE(filecheck=METALLIB): -entry meshMain -stage mesh -target metallib

// METALLIB: result code = 0

struct V
{
    float4 pos : SV_Position;
    float2 a : TEXCOORD0;
    float2 b : TEXCOORD1;
    nointerpolation uint c : TEXCOORD3;
    float d : FOO;
    // BAR and BAR0 are the same semantic; the second is relabeled the same way in both stages.
    float2 e : BAR;
    float2 f : BAR0;
};

struct P
{
    uint id : SV_PrimitiveID;
    float2 uv : TEXCOORD4;
    nointerpolation uint m : MATERIAL1;
    float2 g : QUX;
    float2 h : QUX0;
};

// MESH: position]]
// MESH: user(TEXCOORD)
// MESH: user(TEXCOORD_1)
// MESH: user(TEXCOORD_3)
// MESH: user(FOO)
// MESH: user(BAR)
// MESH: user(BAR_1)
// MESH: primitive_id]]
// MESH: user(TEXCOORD_4)
// MESH: user(MATERIAL_1)
// MESH: user(QUX)
// MESH: user(QUX_1)
[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void meshMain(out vertices V verts[3], out indices uint3 tris[1], out primitives P prims[1])
{
    SetMeshOutputCounts(3, 1);
    V v;
    v.pos = float4(0, 0, 0, 1);
    v.a = 0;
    v.b = 0;
    v.c = 0;
    v.d = 0;
    v.e = 0;
    v.f = 0;
    verts[0] = v;
    verts[1] = v;
    verts[2] = v;
    tris[0] = uint3(0, 1, 2);
    P p;
    p.id = 0;
    p.uv = 0;
    p.m = 1;
    p.g = 0;
    p.h = 0;
    prims[0] = p;
}

// FRAG: user(TEXCOORD)
// FRAG: user(TEXCOORD_1)
// FRAG: user(TEXCOORD_3)
// FRAG: user(FOO)
// FRAG: user(BAR)
// FRAG: user(BAR_1)
// FRAG: user(TEXCOORD_4)
// FRAG: user(MATERIAL_1)
// FRAG: user(QUX)
// FRAG: user(QUX_1)
[shader("fragment")]
float4 fragMain(V i, P p) : SV_Target
{
    return float4(i.a + i.b + i.e + i.f, p.uv + p.g + p.h) + float(i.c) + i.d + float(p.m);
}
