//TEST(compute):SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

// CHECK: OpEntryPoint
// CHECK-NOT: error

[vk::push_constant] GLSLShaderStorageBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

__generic<T>
void foo(T v)
{
};

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    foo(inputBuffer);
    outputBuffer[0] = 0;
}
