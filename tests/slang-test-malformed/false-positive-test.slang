//TEST:COMPARE_COMPUTE: -cpu
//TEST_INPUT: set data = ubuffer(data=[42], stride=4)

// This comment mentions TEST but should not trigger error
// Some random TEST_SOMETHING that is not a real directive
// This is just a TEST comment
// Another line that has TESTCASE in it

RWStructuredBuffer<int> data;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    data[0] = 1;
}