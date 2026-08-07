// Regression for #6319: an entry point that binds the SAME system value more than once must be
// rejected in the front end. Before this check the duplicate reached codegen — WGSL/HLSL emitted
// duplicate builtins and SPIR-V hit an "Unimplemented system value" internal error — so the new
// E30706 (added in validateEntryPoint) now diagnoses it uniformly on every target.
//
// Each function is compiled in isolation via `-entry`, so exactly the single E30706 is produced
// per case. Duplicates are keyed by (direction, output binding space, base name, index,
// no-perspective selector): two SV_VertexID inputs collide, two SV_Target0 outputs collide, and a
// duplicate appearing on the fields of a returned struct collides — while distinct indices
// (SV_Target0 vs SV_Target1) stay clean, which is covered by the positive control in
// entry-point-single-system-value.slang. The no-perspective selector is set only for
// SV_Barycentrics, the one system value where `noperspective` chooses a different builtin. The
// output binding space is empty for these graphics stages (all outputs share one space); it is
// non-empty for mesh shaders (the output category — vertices / primitives / indices, so the same
// system value on distinct categories does not collide) and for geometry shaders (the output
// stream parameter, so a duplicate within one stream element collides while the same value on two
// different streams does not).

//DIAGNOSTIC_TEST:SIMPLE(diag=DUPVERTEX):-target spirv -entry dupVertex -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=DUPTARGET):-target spirv -entry dupTarget -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=DUPSTRUCT):-target spirv -entry dupStruct -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=DUPCANON):-target spirv -entry dupCanon -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=DUPMESHVTX):-target spirv -entry dupMeshVertices -stage mesh
//DIAGNOSTIC_TEST:SIMPLE(diag=DUPGSSTREAM):-target spirv -entry dupGsStream -stage geometry
//DIAGNOSTIC_TEST:SIMPLE(diag=DUPBARY):-target spirv -entry dupBarycentrics -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric
//DIAGNOSTIC_TEST:SIMPLE(diag=DUPBARYIDX):-target spirv -entry dupIndexedBarycentrics -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric
//DIAGNOSTIC_TEST:SIMPLE(diag=DUPBARYINH):-target spirv -entry dupInheritedBarycentrics -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric

// Two parameters carry the same input system value (the reported repro; also the SPIR-V ICE case).
float4 dupVertex(uint a : SV_VertexID, uint b : SV_VertexID) : SV_Position
//DUPVERTEX: E30706
{
    return float4(float(a + b), 0, 0, 1);
}

// Two output parameters bind the same render target.
void dupTarget(out float4 a : SV_Target0, out float4 b : SV_Target0)
//DUPTARGET: E30706
{
    a = float4(1, 0, 0, 1);
    b = float4(0, 1, 0, 1);
}

struct FSOut
{
    float4 a : SV_Target0;
    float4 b : SV_Target0;
};

// The duplicate is on the fields of a returned struct, exercising the recursive walk.
FSOut dupStruct()
//DUPSTRUCT: E30706
{
    FSOut o;
    o.a = float4(1, 0, 0, 1);
    o.b = float4(0, 1, 0, 1);
    return o;
}

// The key is canonical: matching is case-insensitive and an omitted index means 0, so 'SV_Target'
// (which is SV_Target0) and 'sv_target0' name the same binding and collide.
void dupCanon(out float4 x : SV_Target, out float4 y : sv_target0)
//DUPCANON: E30706
{
    x = float4(1, 0, 0, 1);
    y = float4(0, 1, 0, 1);
}

// Mesh outputs are keyed by category (vertices / primitives / indices), so the same system value
// on a vertex output and on a primitive output do NOT collide (a legitimate mesh shader, covered by
// tests/spirv/mesh-primitive.slang). But a mesh shader has a single vertex output array, so two
// `OutputVertices` parameters both carrying SV_Position alias that one array and DO collide even
// though they are separate parameters — the case per-parameter keying would have missed.
struct MeshVertexA
{
    float4 posA : SV_Position;
}
struct MeshVertexB
{
    float4 posB : SV_Position;
}

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void dupMeshVertices(
    OutputVertices<MeshVertexA, 3> a,
    OutputVertices<MeshVertexB, 3> b,
    OutputIndices<uint3, 1> tris)
//DUPMESHVTX: E30706
{
    SetMeshOutputCounts(3, 1);
    a[0] = {float4(0, 0, 0, 1)};
    b[0] = {float4(0, 0, 0, 1)};
    tris[0] = uint3(0, 1, 2);
}

// Geometry outputs are keyed by their stream parameter, so a stream element that binds SV_Position
// twice is a duplicate within one stream's binding space — caught just like the fragment
// return-struct case above. (The same value on two *different* streams would use different keys and
// would not collide.) The stream is declared `inout`, so this also exercises that the spurious
// input half of the inout stream parameter is not collected a second time.
struct GsBadOut
{
    float4 posA : SV_Position;
    float4 posB : SV_Position;
}

[maxvertexcount(3)]
void dupGsStream(uint prim : SV_PrimitiveID, inout TriangleStream<GsBadOut> outStream)
//DUPGSSTREAM: E30706
{
    GsBadOut v;
    v.posA = float4(0, 0, 0, 1);
    v.posB = float4(0, 0, 0, 1);
    outStream.Append(v);
}

// SV_Barycentrics is the one system value whose binding `noperspective` splits in two, so the
// identity carries that selector. This pins the *other* side of that rule: two barycentric inputs
// in the SAME interpolation mode still name one builtin and so are still a duplicate. (The legal
// perspective + no-perspective pairing is the BARY case in
// entry-point-single-system-value.slang.)
void dupBarycentrics(
    float3 a : SV_Barycentrics,
    float3 b : SV_Barycentrics,
    out float4 result : SV_Target)
//DUPBARY: E30706
{
    result = float4(a + b, 1);
}

// The index participates in the identity only for a system value whose index selects a distinct
// binding. Barycentric builtin selection never consults it, so "SV_Barycentrics0" and
// "SV_Barycentrics1" both reach BaryCoordKHR and are the same binding declared twice. (Contrast
// SV_Target0 vs SV_Target1, distinct render targets, in entry-point-single-system-value.slang.)
void dupIndexedBarycentrics(
    float3 a : SV_Barycentrics0,
    float3 b : SV_Barycentrics1,
    out float4 result : SV_Target)
//DUPBARYIDX: E30706
{
    result = float4(a + b, 1);
}

struct InheritedDupBary
{
    // Neither field declares an interpolation mode, so both inherit `noperspective` from the
    // enclosing parameter and both reach BaryCoordNoPerspKHR — one binding, declared twice.
    float3 a : SV_Barycentrics;
    float3 b : SV_Barycentrics;
}

// Both fields inherit the parameter's mode, so this is a duplicate under the inherited-mode rule.
// Note it does NOT discriminate that rule on its own: reading the modifier off the field alone
// records both as perspective-correct, which is equally a duplicate, so E30706 fires either way and
// only the reason differs. The case that pins inheritance is the legal one it mirrors —
// `inheritedBarycentrics` in entry-point-single-system-value.slang, which is wrongly rejected
// without the fix. This case is kept because it is the shape a user is most likely to write.
void dupInheritedBarycentrics(noperspective InheritedDupBary input, out float4 result : SV_Target)
//DUPBARYINH: E30706
{
    result = float4(input.a + input.b, 1);
}
