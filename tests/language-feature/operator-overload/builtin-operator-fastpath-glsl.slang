// The builtin-operator fast path must be *disabled* in GLSL-compatibility scope, where some
// operators have different semantics owned by the `glsl` module's overloads: `mat * mat` is an
// algebraic matrix product (not component-wise) and `vec == vec` yields a scalar `bool`
// (all-components-equal). This test pins those semantics so a future change that re-enables the
// fast path under GLSL scope would be caught.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -allow-glsl -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -allow-glsl -output-using-type

import glsl;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
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

    // CHECK: 2.0
    // CHECK: 3.0
    // CHECK: 100.0
    // CHECK: 50.0
}
