//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Reading each caller's own lane has an independent result even when the hardware schedules
// only a subset of a source branch together. Cross-lane expectations would require a guaranteed
// converged participation set. Matrix helpers exercise _getActiveMask(), including its ballot.
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    matrix<int, 2, 2> value = matrix<int, 2, 2>(lane + 1, lane + 101, lane + 201, lane + 301);
    matrix<int, 2, 2> converged = WaveReadLaneAt(value, lane);
    bool ok = all(converged[0] == value[0]) & all(converged[1] == value[1]);
    if (lane < 16)
    {
        matrix<int, 2, 2> shuffled = WaveReadLaneAt(value, lane);
        ok &= all(shuffled[0] == value[0]) & all(shuffled[1] == value[1]);
        outputBuffer[lane] = ok ? 1000 + lane : 0;
    }
    else
    {
        matrix<int, 2, 2> shuffled = WaveReadLaneAt(value * 2, lane);
        ok &= all(shuffled[0] == value[0] * 2) & all(shuffled[1] == value[1] * 2);
        outputBuffer[lane] = ok ? 1000 + lane : 0;
    }
}

// CHECK: 1000
// CHECK: 1001
// CHECK: 1002
// CHECK: 1003
// CHECK: 1004
// CHECK: 1005
// CHECK: 1006
// CHECK: 1007
// CHECK: 1008
// CHECK: 1009
// CHECK: 1010
// CHECK: 1011
// CHECK: 1012
// CHECK: 1013
// CHECK: 1014
// CHECK: 1015
// CHECK: 1016
// CHECK: 1017
// CHECK: 1018
// CHECK: 1019
// CHECK: 1020
// CHECK: 1021
// CHECK: 1022
// CHECK: 1023
// CHECK: 1024
// CHECK: 1025
// CHECK: 1026
// CHECK: 1027
// CHECK: 1028
// CHECK: 1029
// CHECK: 1030
// CHECK: 1031
