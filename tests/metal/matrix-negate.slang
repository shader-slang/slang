//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute

// CHECK: (matrix<float,int(4),int(4)>{{.*}}(0) -

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name output
RWStructuredBuffer<float4> output;

[numthreads(1,1,1)]
void computeMain(uniform float4x4 m)
{
    // This test checks for negative zero, so we need to do a hex comparison
    // instead of a float comparison.
    // BUF: 80000000
    // BUF: 80000000
    // BUF: 80000000
    // BUF: 80000000
    output[0] = (-m)[0];
}
