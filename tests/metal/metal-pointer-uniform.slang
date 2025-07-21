//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHK_AIR): -target metallib -stage compute -entry computeMain

// This reproduces the specific case that was failing where uniform pointers
// are used for buffer copy operations with array indexing targeting Metal.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

// CHECK: uint{{.*}}device{{ *\*}} src
// CHECK: uint{{.*}}device{{ *\*}} dst

// "indirect_argument" means a pointer
//CHK_AIR: !"uint", !"src{{[^"]*}}", !"air.indirect_argument"

uniform uint32_t* src;
uniform uint32_t* dst;

[shader("compute")]
[numthreads(4,1,1)]
void computeMain(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID)
{
    var input = src[sv_dispatchThreadID.x];
    dst[sv_dispatchThreadID.x] = input;
}
