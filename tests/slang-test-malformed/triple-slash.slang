// This test should fail due to ill-formed TEST directive (triple slash)
///TEST:COMPARE_COMPUTE: -cpu
//TEST_INPUT: set data = ubuffer(data=[42], stride=4)

RWStructuredBuffer<int> data;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    data[0] = 1;
}