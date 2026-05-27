//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -emit-spirv-via-glsl -entry computeMain -stage compute -O1

// CHECK: OpEntryPoint GLCompute
// CHECK: OpReturn

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
