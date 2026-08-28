//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_NVVM): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0

// Positive companion to shader-slang/slang#10188: a genuine `ConstantBuffer<>` (not the SBT)
// is truly immutable and MUST still use `__ldg` — guards against the SBT exclusion being too
// broad (it only fires for addresses rooted at `GetOptiXSbtDataPtr`, which a CB never is).

struct Params
{
    uint value;
};

ConstantBuffer<Params> gParams;

//CHECK: __ldg
//CHECK_NVVM: .const .align 8 .b8 SLANG_globalParams[8]
//CHECK_NVVM: ld.const.u64
//CHECK_NVVM: ld.global.nc.u32

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(RWStructuredBuffer<uint> output, uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = gParams.value + tid.x;
}
