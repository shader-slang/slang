//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Test that load instructions from global parameters don't cause
// "Unhandled global inst in spirv-emit" errors.

struct Data
{
    uint value;
};

layout(set = 0, binding = 0) StructuredBuffer<Data> inputBuffer;
layout(set = 0, binding = 1) RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    
    // This pattern previously caused "Unhandled global inst in spirv-emit"
    // when a load from a global parameter appeared in certain contexts
    let data = inputBuffer[idx].value;
    
    if (data > 0)
    {
        outputBuffer[idx] = data;
    }
    else
    {
        outputBuffer[idx] = 0;
    }
    
    //CHECK: 42
}
