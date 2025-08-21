//TEST:SIMPLE(filecheck=CHECK_MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_AIR): -target metallib -stage compute -entry computeMain
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -emit-spirv-directly
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -render-features argument-buffer-tier-2
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu

// This reproduces the specific case that was failing where uniform pointers
// are used for buffer copy operations with array indexing targeting Metal.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//CHECK_MTL: int{{.*}}device{{ *\*}} src

// "indirect_argument" means a pointer
//CHECK_AIR: !"int", !"src{{[^"]*}}", !"air.indirect_argument"

//TEST_INPUT: set src = ubuffer(data=[1 2 3 4 5 6 7 8],stride=4);
uniform int* src;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 threadID: SV_DispatchThreadID)
{
    let input = src[threadID.x];
    outputBuffer[threadID.x] = input;
}

//CHECK:1
//CHECK-NEXT:2
//CHECK-NEXT:3
//CHECK-NEXT:4
