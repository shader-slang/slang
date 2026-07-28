import "glsl";

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly
//CHECK: OpExecutionMode %compute LocalSize 1 2 3

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1u, 2u, 3)]
void compute()
{
    const int x = gl_WorkGroupSize.x;
    outputBuffer[0] = x;
}
