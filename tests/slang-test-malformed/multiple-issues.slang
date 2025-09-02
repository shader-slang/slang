// This test has multiple ill-formed directives
///TEST:COMPARE_COMPUTE: -cpu
// TEST_CATEGORY: compute
////TEST_INPUT: set data = ubuffer(data=[42], stride=4)

RWStructuredBuffer<int> data;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    data[0] = 1;
}