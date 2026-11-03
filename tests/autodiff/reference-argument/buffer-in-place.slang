// A buffer element passed to a reference parameter from a differentiated body is passed in place:
// the callee writes through the reference and then reads the same element directly, and sees its
// own write. A copy passed instead would leave the element unchanged until the copy is stored back
// after the call, so the direct read would see 1.0 instead of 2.0.
//
// The backward derivative runs the call once, in its primal pass, and does not run it again to
// compute the derivative, so the element ends at 3.0 after the backward and forward derivatives
// each run it once.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[1.0], stride=4):name=data
RWStructuredBuffer<float> data;

float bumpThenReadDirectly(__ref float a)
{
    a += 1.0;
    return data[0];
}

[Differentiable]
float elementByReference(float x)
{
    return no_diff(bumpThenReadDirectly(data[0])) * x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    var x = diffPair(1.0);
    bwd_diff(elementByReference)(x, 1.0);
    outputBuffer[0] = x.d;

    outputBuffer[1] = fwd_diff(elementByReference)(diffPair(1.0, 1.0)).d;
    outputBuffer[2] = data[0];
}

// CHECK: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 3.0
