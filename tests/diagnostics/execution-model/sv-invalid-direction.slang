//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry vertMain

//CHECK: E30702
struct VOut { uint vid : SV_VertexID; float4 pos : SV_Position; };
/*CHECK:
                   ^^^
*/
[shader("vertex")]
VOut vertMain() { VOut o; o.vid = 0; o.pos = float4(0); return o; }
