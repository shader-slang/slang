//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// A hardware mask is a scheduling snapshot, not all lanes taking a source branch. Check
// independently valid membership invariants without assuming exact convergence. Every caller
// must name itself, and the real divergent branch must exclude lanes on the other path.
bool validMask(uint mask, uint lane, uint allowed)
{
    return (mask & (1u << lane)) != 0 && (mask & ~allowed) == 0;
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint converged = WaveGetConvergedMask();
    uint4 multi = WaveGetConvergedMulti();
    bool ok = validMask(converged, lane, 0xffffffffu)
        && validMask(multi.x, lane, 0xffffffffu) && all(multi.yzw == uint3(0));
    // The writes and separate mask reads retain real branches in the inspected O0/O3 PTX.
    if (lane < 16)
    {
        uint mask = WaveGetConvergedMask();
        uint4 branchMulti = WaveGetConvergedMulti();
        ok &= validMask(mask, lane, 0xffffu);
        ok &= validMask(branchMulti.x, lane, 0xffffu) && all(branchMulti.yzw == uint3(0));
        outputBuffer[lane] = ok ? 1000 + lane : 0;
    }
    else
    {
        uint mask = WaveGetConvergedMask();
        uint4 branchMulti = WaveGetConvergedMulti();
        ok &= validMask(mask, lane, 0xffff0000u);
        if (lane == 31)
        {
            // A single caller has an exact mask regardless of scheduling granularity.
            ok &= WaveGetConvergedMask() == 0x80000000u;
            uint4 singleton = WaveGetConvergedMulti();
            ok &= singleton.x == 0x80000000u && all(singleton.yzw == uint3(0));
        }
        ok &= validMask(branchMulti.x, lane, 0xffff0000u) && all(branchMulti.yzw == uint3(0));
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
