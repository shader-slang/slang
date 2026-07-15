//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -compile-arg -O3 -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj

// We are going to pass -Gis to DXC.
// Test can only work on DXC.
//
// Issue #12121: render-test now accepts the same -X<compiler> forms slangc accepts, so
// these no longer need to be tunneled through -Xslang... -Xdxc -Gis -X.  The single-option
// form (-Xdxc <opt>) and the scoped block form (-Xdxc... <opts> -X.) are both covered here;
// -Xdxc -Vd from the issue body is the same single-option grammar.

// Single-option form: -Xdxc <opt>
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -Xdxc -Gis -shaderobj

// Scoped block form: -Xdxc... <opts> -X.
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -Xdxc... -Gis -X. -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = dispatchThreadID.x;
}