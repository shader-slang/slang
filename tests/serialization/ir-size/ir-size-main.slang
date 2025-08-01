//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

import ir_size_import;

//TEST_INPUT: set output = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = __ir_size;       // CHECK: 7734
    output[1] = imported_size(); // CHECK: 3066
}
