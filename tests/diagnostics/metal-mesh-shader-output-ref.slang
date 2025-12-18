//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target metal -entry meshMain

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
//CHECK: ([[# @LINE+1]]): error 56104
    verts[i].pos = float4(0.0);
  }

  // Output indices
  tris[0] = uint3(0, 1, 2);
}
