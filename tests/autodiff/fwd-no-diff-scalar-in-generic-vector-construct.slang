//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Test that forward-mode autodiff correctly handles a no_diff scalar parameter
// used in a vector construction (via lerp(float3, float3, float)) inside a
// generic function with IDifferentiable interface constraints.
//
// Previously this crashed with "unexpected IR opcode during code emit" because
// the forward-mode pass emitted void_constant (RTTIType) instead of returning
// nullptr for the all-void construct differential.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IA : IDifferentiable {
    [Differentiable]
    float3 f(float3 x);
}

struct SA : IA {
    float s;
    [Differentiable]
    float3 f(float3 x) { return s * x; }
}

[Differentiable]
float3 my_func<A : IA>(A a, float3 x, float3 y, no_diff float alpha) {
    return a.f(lerp(x, y, alpha));
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    SA a;
    a.s = 2.0;

    // fwd_diff of my_func with dx=(1,1,1), dy=(0,0,0)
    // lerp(x, y, 0.5) = (2,2,2), a.f(v) = 2*v = (4,4,4)
    // d/dx lerp = (1-alpha)*dx = 0.5*(1,1,1), d(a.f) = 2*0.5 = (1,1,1)
    DifferentialPair<float3> dpx = diffPair(float3(1, 1, 1), float3(1, 1, 1));
    DifferentialPair<float3> dpy = diffPair(float3(3, 3, 3), float3(0, 0, 0));
    DifferentialPair<SA> dpa = diffPair(a, SA.dzero());

    DifferentialPair<float3> result = __fwd_diff(my_func)(dpa, dpx, dpy, 0.5);

    outputBuffer[0] = result.p.x; // CHECK: 4.0
    outputBuffer[1] = result.d.x; // CHECK: 1.0
    outputBuffer[2] = result.d.y; // CHECK: 1.0
    outputBuffer[3] = result.d.z; // CHECK: 1.0
}
