// Importing the `glsl` module into Slang source retains one legacy dialect behavior: the
// builtin-operator fast path must be disabled because some operators have different semantics
// owned by the module's overloads. `mat * mat` is an algebraic matrix product (not component-wise)
// and `vec == vec` yields a scalar `bool` (all-components-equal). The import does not otherwise
// turn this Slang source into GLSL.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import glsl;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = outputBuffer[0] + 1.0; // 1.0

    // GLSL `mat * mat` is the matrix product. With m2 = identity, the product is m1, so the
    // off-diagonal entry survives (2.0). A component-wise multiply would instead give 0.0.
    float2x2 m1 = float2x2(a, 2.0 * a, 3.0 * a, 4.0 * a); // {{1,2},{3,4}}
    float2x2 m2 = float2x2(a, 0.0, 0.0, a);              // identity
    float2x2 prod = m1 * m2;
    outputBuffer[0] = prod[0][1]; // product -> 2.0  (component-wise would be 0.0)
    outputBuffer[1] = prod[1][0]; // product -> 3.0  (component-wise would be 0.0)

    // GLSL `vec == vec` is a scalar bool (all-equal). Using it in a bool context only
    // type-checks if the scalar-bool GLSL overload is selected (a vector<bool> result could
    // not initialize a `bool`).
    float4 va = float4(a, a, a, a);
    float4 vb = float4(a, a, a, a);
    bool allEq = (va == vb);
    outputBuffer[2] = allEq ? 100.0 : 0.0; // -> 100.0

    float4 vc = float4(a, a, a, a + 1.0);
    bool someNe = (va != vc);
    outputBuffer[3] = someNe ? 50.0 : 0.0; // -> 50.0

    // GLSL `mat * vec` and `vec * mat` are algebraic products via the glsl module's rank-15
    // overloads. The fallback `operator*` declarations that diagnose these shapes in plain
    // Slang rank below them (-10 vs 15), so the GLSL semantics keep winning here. With m2
    // the identity, both products equal v.
    float2 vg = float2(a, 2.0 * a); // (1, 2)
    float2 mv = m2 * vg;
    float2 vm = vg * m2;
    outputBuffer[4] = mv[0] + mv[1] + vm[0] + vm[1]; // 1+2+1+2 = 6.0

    // Non-square algebraic case: the glsl module's mat-vec overload takes a vector sized
    // to the matrix's row count, so a 3x4 matrix times a 3-vector is its product (and the
    // fallback, which keys on the column count, does not apply).
    float3x4 m34 = float3x4(a, 0.0, 0.0, 0.0, 0.0, a, 0.0, 0.0, 0.0, 0.0, a, 0.0); // 3x4, first three columns of identity
    float3 vg3 = float3(a, a, a);
    float4 mv34 = m34 * vg3; // -> (1, 1, 1, 0)
    outputBuffer[5] = mv34[0] + mv34[1] + mv34[2] + mv34[3]; // 3.0

    // Non-square mat-mat: both the glsl module's rank-15 algebraic overload and the
    // rank -10 fallback apply to these shapes with different result types; the GLSL
    // semantics must win, and the test only compiles when the product is a 3x3.
    float2x3 mg = float2x3(a, 0.0, 0.0, 0.0, 0.0, a); // rows: [1,0,0],[0,0,1]
    float3x2 mg2 = float3x2(a, 0.0, 0.0, a, 0.0, 0.0); // rows: [1,0],[0,1],[0,0]
    float3x3 mm = mg * mg2; // algebraic: [[1,0,0],[0,0,1],[0,0,0]]
    outputBuffer[6] = mm[0][0] + mm[1][2]; // 1.0 + 1.0 = 2.0

    // CHECK: 2.0
    // CHECK: 3.0
    // CHECK: 100.0
    // CHECK: 50.0
    // CHECK: 6.0
    // CHECK: 3.0
    // CHECK: 2.0
}
