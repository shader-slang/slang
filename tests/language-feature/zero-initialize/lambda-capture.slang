//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -emit-spirv-directly -zero-initialize

// CHECK: OpEntryPoint GLCompute %computeMain
// CHECK: OpStore

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(RWStructuredBuffer<int> output)
{
    int captured = 1;
    let addCaptured = ((uint row, uint col, int value) => value + captured);
    output[0] = addCaptured(0, 0, 0);
}
