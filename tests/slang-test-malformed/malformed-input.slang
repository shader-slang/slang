// This test should fail due to ill-formed TEST_INPUT directive
// TEST_INPUT: set data = ubuffer(data=[42], stride=4)
//TEST:COMPARE_COMPUTE: -cpu

RWStructuredBuffer<int> data;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    data[0] = 1;
}