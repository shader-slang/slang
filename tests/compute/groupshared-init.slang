//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type -use-dxil
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// CHECK: type: uint32_t
// CHECK-NEXT: 1231
// CHECK-NEXT: 1232
// CHECK-NEXT: 1233
// CHECK-NEXT: 1234

// This is a basic test for Slang compute shader.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

groupshared uint myGroupSharedValue = foo();

uint foo()
{
  return 1231;
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outputBuffer[i] = i + myGroupSharedValue;
}
