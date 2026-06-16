// builtin-no-volatile-graphics-stages.slang
//
// Regression test for shader-slang/slang#10528. The
// `isRaytracingStageRequiringVolatileBuiltins` predicate is the gate
// for the subgroup/SM Volatile path on a builtin's referencing entry
// points. The positive RT stages (raygen, closesthit, miss,
// intersection, callable) are pinned by sibling tests; this test pins
// the negative case from each non-RT graphics stage that can read
// `WaveGetLaneIndex()` (vertex, fragment, geometry, mesh, amplification).
// A regression that widened the gate to `true` for any graphics stage
// would emit `Volatile` on `SubgroupLocalInvocationId` and silently
// regress these stages — closesthit-only coverage cannot catch that.
//
// Tessellation (hull/domain) is deliberately scoped out: hull entry
// points have additional patch-constant function plumbing that doesn't
// add coverage to the gate test, and the same `WaveGetLaneIndex()` →
// `SubgroupLocalInvocationId` lowering is exercised by the five stages
// here.

//TEST:SIMPLE(filecheck=CHECK_VS): -target spirv-asm -entry vsMain -stage vertex -emit-spirv-directly
// Fragment uses `-skip-spirv-validation`: reading
// `SubgroupLocalInvocationId` from a fragment shader trips
// `VUID-StandaloneSpirv-Flat-04744` (Input integer interfaces require
// `Flat`), which is unrelated to the gate this test is asserting. The
// disassembly is still produced and the `Volatile`-absence pattern can
// be checked.
//TEST:SIMPLE(filecheck=CHECK_PS): -target spirv-asm -entry psMain -stage fragment -emit-spirv-directly -skip-spirv-validation
//TEST:SIMPLE(filecheck=CHECK_GS): -target spirv-asm -entry gsMain -stage geometry -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_MS): -target spirv-asm -entry msMain -stage mesh -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_AS): -target spirv-asm -entry asMain -stage amplification -emit-spirv-directly

RWStructuredBuffer<uint> output;

// Each stage's `CHECK_*-NOT` brackets the positive `CHECK_*`: leading
// `-NOT` scans file start → match, trailing scans match → EOF, so the
// `Volatile` absence is asserted across the whole disassembly per
// stage.

// === Vertex ===
// CHECK_VS-NOT: Volatile
// CHECK_VS: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK_VS-NOT: Volatile
[shader("vertex")]
float4 vsMain(uint vid : SV_VertexID) : SV_Position
{
    output[vid] = WaveGetLaneIndex();
    return float4(0, 0, 0, 1);
}

// === Fragment ===
// CHECK_PS-NOT: Volatile
// CHECK_PS: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK_PS-NOT: Volatile
[shader("fragment")]
float4 psMain() : SV_Target
{
    output[0] = WaveGetLaneIndex();
    return float4(0, 0, 0, 1);
}

// === Geometry ===
// CHECK_GS-NOT: Volatile
// CHECK_GS: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK_GS-NOT: Volatile
struct GsIn { float4 pos : SV_Position; };
struct GsOut { float4 pos : SV_Position; };
[shader("geometry")]
[maxvertexcount(3)]
void gsMain(triangle GsIn input[3], inout TriangleStream<GsOut> stream)
{
    output[0] = WaveGetLaneIndex();
    GsOut o;
    o.pos = input[0].pos;
    stream.Append(o);
}

// === Mesh ===
// CHECK_MS-NOT: Volatile
// CHECK_MS: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK_MS-NOT: Volatile
struct MsVtx { float4 pos : SV_Position; };
[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void msMain(out vertices MsVtx verts[3], out indices uint3 tris[1])
{
    SetMeshOutputCounts(3, 1);
    output[0] = WaveGetLaneIndex();
    verts[0].pos = float4(0, 0, 0, 1);
    verts[1].pos = float4(0, 0, 0, 1);
    verts[2].pos = float4(0, 0, 0, 1);
    tris[0] = uint3(0, 1, 2);
}

// === Amplification ===
// CHECK_AS-NOT: Volatile
// CHECK_AS: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK_AS-NOT: Volatile
struct AsPayload { uint v; };
[shader("amplification")]
[numthreads(1, 1, 1)]
void asMain()
{
    AsPayload p;
    p.v = WaveGetLaneIndex();
    output[0] = p.v;
    DispatchMesh(1, 1, 1, p);
}
