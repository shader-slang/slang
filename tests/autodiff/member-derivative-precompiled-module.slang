//TEST:COMPILE: tests/autodiff/member-derivative-library-module.slang -o tests/autodiff/member-derivative-library-module.slang-module
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// `__fwd_diff(c.eval)` and `__bwd_diff(c.eval)` for a method from a precompiled module, whose
// declaration has no scope to find `this` in. `Cubic.eval(t) = t^3`, so `eval'(t) = 3t^2`.

import member_derivative_library_module;

float tangent<C : ICurve>(C c, float t) { return __fwd_diff(c.eval)(diffPair(t, 1.0)).d; }

float gradient<C : ICurve>(C c, float t)
{
    var p = diffPair(t);
    __bwd_diff(c.eval)(p, 1.0);
    return p.d;
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Cubic c;
    // CHECK: 12.0
    outputBuffer[0] = tangent(c, 2.0);
    // CHECK: 27.0
    outputBuffer[1] = gradient(c, 3.0);
}
