//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -skip-spirv-validation

// Test that per-primitive semantics used in a vertex output generate a diagnostic.
// Per-primitive semantics must only appear in 'out primitives' / OutputPrimitives<> parameters.

struct MeshOutput {
    float3 Color    : COLOR;
    bool Cull : SV_CullPrimitive;
//CHECK: ^^^^ per-primitive system value semantic 'SV_CullPrimitive' must be placed in an 'OutputPrimitives' (or 'out primitives') parameter
    uint Prim : SV_PrimitiveID;
//CHECK: ^^^^ per-primitive system value semantic 'SV_PrimitiveID' must be placed in an 'OutputPrimitives' (or 'out primitives') parameter
    uint Layer : SV_RenderTargetArrayIndex;
//CHECK: ^^^^^ per-primitive system value semantic 'SV_RenderTargetArrayIndex' must be placed in an 'OutputPrimitives' (or 'out primitives') parameter
    uint Viewport : SV_ViewportArrayIndex;
//CHECK: ^^^^^^^^ per-primitive system value semantic 'SV_ViewportArrayIndex' must be placed in an 'OutputPrimitives' (or 'out primitives') parameter
    uint Rate : SV_ShadingRate;
//CHECK: ^^^^ per-primitive system value semantic 'SV_ShadingRate' must be placed in an 'OutputPrimitives' (or 'out primitives') parameter
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void msmain(out indices uint3 triangles[3], out vertices MeshOutput vertices[9]) {
    SetMeshOutputCounts(6, 2);
    triangles[0] = uint3(0, 1, 2);
    vertices[0].Color = float3(0, 0, 0);
}

// Test that per-primitive semantics nested inside a struct field are also caught.
struct PerPrimInner {
    bool Cull : SV_CullPrimitive;
//CHECK: ^^^^ per-primitive system value semantic 'SV_CullPrimitive' must be placed in an 'OutputPrimitives' (or 'out primitives') parameter
};

struct MeshOutputNested {
    float3 Color : COLOR;
    PerPrimInner prim;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void msmain_nested(out indices uint3 triangles[3], out vertices MeshOutputNested vertices[9]) {
    SetMeshOutputCounts(6, 2);
    triangles[0] = uint3(0, 1, 2);
    vertices[0].Color = float3(0, 0, 0);
}

struct IndexWithPrimSemantic {
    uint3 idx;
    uint primId : SV_PrimitiveID;
//CHECK: ^^^^^^ per-primitive system value semantic 'SV_PrimitiveID' must be placed in an 'OutputPrimitives' (or 'out primitives') parameter
};

struct MeshOutput2 {
    float3 Color    : COLOR;
    bool Cull : SV_CullPrimitive;
    uint Prim : SV_PrimitiveID;
    uint Layer : SV_RenderTargetArrayIndex;
    uint Viewport : SV_ViewportArrayIndex;
    uint Rate : SV_ShadingRate;
};

struct SimpleVertex {
    float4 pos : SV_Position;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void msmain_indices(out indices IndexWithPrimSemantic triangles[3], out vertices SimpleVertex verts[9], out primitives MeshOutput2 prims[9]) {
    SetMeshOutputCounts(6, 2);
}
