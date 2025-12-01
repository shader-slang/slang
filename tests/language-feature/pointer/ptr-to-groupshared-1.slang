//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// By default slang-test uses `-g` and it requires `VariablePointers`, which
// doesn't produce the correct result due to the bug on the graphics driver.
// Tracked by github issue #9061
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// Tests if we handle passing groupshared address-space pointers correctly to a function
// when that data-type needs legalization (Data -> Data_natural due to `lower-buffer-element-type`).
// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
uniform int* outputBuffer;
groupshared uint4 shared[2];

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=inputBuffer
uniform int* inputBuffer;

void foo(Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared> ptr)
{
    ptr[0] = uint4(1, 2, 3, 4);
    ptr[1] = uint4(5, 6, 7, 8);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    shared[0] = uint4(0, 0, 0, 0);
    shared[1] = uint4(0, 0, 0, 0);

    foo(__getAddress(shared[0]));

    outputBuffer[0] = shared[0].x;
    outputBuffer[1] = shared[0].y;
    outputBuffer[2] = shared[0].z;
    outputBuffer[3] = shared[0].w;

    outputBuffer[4] = shared[1].x;
    outputBuffer[5] = shared[1].y;
    outputBuffer[6] = shared[1].z;
    outputBuffer[7] = shared[1].w;
}
