//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -d3d12 -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -shaderobj -output-using-type -render-features argument-buffer-tier-2

struct S1 {
    float m;
};
struct S2 {
    ParameterBlock<S1> s;
};
struct S3 {
    ParameterBlock<S2> s;
};
struct S4 {
    ParameterBlock<S3> s;
};
//TEST_INPUT: set s2.s.m = 1.0
ParameterBlock<S2> s2;
//TEST_INPUT: set s4.s.s.s.m = 2.0
ParameterBlock<S4> s4;

#define PB ParameterBlock
//TEST_INPUT: set direct1 = 3.0
PB<float> direct1;
//TEST_INPUT: set direct2 = 4.0
PB<PB<PB<float>>> direct2;

//TEST_INPUT: ubuffer(data=[0], stride=1, count=4):out,name outBuffer
RWStructuredBuffer<float> outBuffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 sv_dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: 1.000000
    outBuffer[0] = s2.s.m;
    // CHECK: 2.000000
    outBuffer[1] = s4.s.s.s.m;
    // FIXME: unable to check the result.
    outBuffer[2] = direct1;
    outBuffer[3] = direct2;
}
