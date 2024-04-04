// Test that a mesh shader generates PerPrimitiveNV decoration for OutputPrimitives
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage mesh -emit-spirv-directly

// CHECK: OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
// CHECK: OpDecorate %gl_PrimitiveID PerPrimitiveNV

const static let color: float3[] = {
  float3(1.0, 0.0, 0.0),
  float3(0.0, 1.0, 0.0),
  float3(0.0, 0.0, 1.0),
};

const static let position: float3[] = {
  float3( 0.5,  0.5, 0.0),
  float3( 0.0, -0.5, 0.0),
  float3(-0.5,  0.5, 0.0),
};

struct SVertexOutput {
  float4 Position: SV_Position;
  float3 Color;
};

struct SPrimitiveOutput {
  uint PrimitiveId: SV_PrimitiveID;
};

[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
func main(
  OutputVertices<SVertexOutput, 3> outputVertices,
  OutputPrimitives<SPrimitiveOutput, 1> outputPrimitives,
  OutputIndices<uint3, 1> outputIndices,
) -> void {
  SetMeshOutputCounts(3, 1);
  outputVertices[0] = { float4(position[0], 1.0), color[0] };
  outputVertices[1] = { float4(position[1], 1.0), color[1] };
  outputVertices[2] = { float4(position[2], 1.0), color[2] };
  outputIndices[0] = uint3(0, 1, 2);

  outputPrimitives[0] = { 0 };
}

