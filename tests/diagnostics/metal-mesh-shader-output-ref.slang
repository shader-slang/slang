//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry meshMain

struct VsOutput {
  float4 pos : SV_Position;
};

[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void meshMain(out vertices VsOutput verts[3], out indices uint3 tris[1])
{
  SetMeshOutputCounts(3, 1);

  // Output vertices
  for (uint i = 0; i < 3; i++) {
    verts[i].pos = float4(0.0);
//CHECK:         ^ mesh output must be assigned as whole struct
//CHECK:         ^ whole struct must be assiged to mesh output at once for Metal target.
  }

  // Output indices
  tris[0] = uint3(0, 1, 2);
}
