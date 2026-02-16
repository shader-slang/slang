//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):


struct VertexOutput{float Input;}
float4 fragmentMain(VertexOutput vertex) : SV_Target
{
  static float GetValue(int val) { return vertex.Input; } // Simplified example
//CHECK:       ^ function is not allowed here.
  float a = GetValue(vertex.Input);

  // etc
}
