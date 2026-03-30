//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):


struct VertexOutput{float Input;}
float4 fragmentMain(VertexOutput vertex) : SV_Target
{
  static float GetValue(int val) { return vertex.Input; } // Simplified example
//CHECK:       ^^^^^^^^ declaration not allowed here
//CHECK:       ^^^^^^^^ function is not allowed here.
//CHECK:       ^^^^^^^^ declaration not allowed in this context
//CHECK:       ^^^^^^^^ function declaration is not allowed inside function.
  return float4(0);
}
