//TEST:SIMPLE(filecheck=CHECK_O1): -target spirv-assembly -emit-spirv-via-glsl -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=CHECK_O3): -target spirv-assembly -emit-spirv-via-glsl -entry computeMain -stage compute -O3

// CHECK_O1: OpEntryPoint GLCompute
// CHECK_O1: OpReturn
// CHECK_O3: OpEntryPoint GLCompute
// CHECK_O3: OpReturn

RWStructuredBuffer<int> result;

int helper(int x)
{
    if (x < 0)
    {
        return -1;
    }
    if (x == 0)
    {
        return 0;
    }
    return x * x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    result[dispatchThreadID.x] = helper(int(dispatchThreadID.x) - 1);
}
