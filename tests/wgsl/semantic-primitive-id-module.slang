// A WGSL `enable` directive applies to the whole module, so compiling every entry point of this
// file into one module emits `enable primitive_index;` exactly once. The vertex entry point, which
// has no `SV_PrimitiveID` input, is still emitted into that module.
// https://github.com/shader-slang/slang/issues/13617

//TEST:SIMPLE(filecheck=WGSL): -target wgsl

//WGSL: enable primitive_index;
//WGSL-NOT: enable primitive_index;
//WGSL-DAG: fn fragmentWithPrimitive(@builtin(primitive_index) {{.*}}: u32)
//WGSL-DAG: @vertex
//WGSL-DAG: fn vertexWithoutPrimitive(@builtin(vertex_index)

[shader("fragment")]
float4 fragmentWithPrimitive(uint primitive : SV_PrimitiveID) : SV_Target0
{
    return float4(float(primitive));
}

[shader("vertex")]
float4 vertexWithoutPrimitive(uint vertexIndex : SV_VertexID) : SV_Position
{
    return float4(float(vertexIndex));
}
