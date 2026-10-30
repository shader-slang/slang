// HLSL interpolation modifiers on Metal stage inputs: one MSL attribute combining sampling and
// perspective correction.
//   linear (perspective-correct)  -> default, no attribute ([[center_perspective]])
//   noperspective                 -> [[center_no_perspective]]
//   centroid / sample             -> [[centroid_perspective]] / [[sample_perspective]]
//   nointerpolation               -> [[flat]]
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage fragment -entry fsMain

struct Varyings
{
    float4 position : SV_Position;
    linear float a : A;
    noperspective float b : B;
    centroid float c : C;
    sample float d : D;
    nointerpolation float f : F;
};

// CHECK-DAG: {{^}}    float a_{{[0-9]+}} {{\[\[}}user(A){{\]\]}};
// CHECK-DAG: {{\[\[}}center_no_perspective{{\]\]}} float b_{{[0-9]+}} {{\[\[}}user(B){{\]\]}};
// CHECK-DAG: {{\[\[}}centroid_perspective{{\]\]}} float c_{{[0-9]+}} {{\[\[}}user(C){{\]\]}};
// CHECK-DAG: {{\[\[}}sample_perspective{{\]\]}} float d_{{[0-9]+}} {{\[\[}}user(D){{\]\]}};
// CHECK-DAG: {{\[\[}}flat{{\]\]}} float f_{{[0-9]+}} {{\[\[}}user(F){{\]\]}};
// CHECK-NOT: sample_no_perspective
float4 fsMain(Varyings v) : SV_Target
{
    return float4(v.a + v.b, v.c, v.d, v.f);
}
