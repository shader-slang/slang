//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHK_AIR): -target metallib -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=8):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

//CHECK: ulong{{.*}}device{{ *\*}} a
//CHECK-NEXT: long{{.*}}device{{ *\*}} b
//CHECK-NEXT: uint{{.*}}device{{ *\*}} c
//CHECK-NEXT: int{{.*}}device{{ *\*}} d
//CHECK-NEXT: float{{.*}}device{{ *\*}} e

// "indirect_argument" means a pointer
//CHK_AIR: !"ulong", !"a{{[^"]*}}", !"air.indirect_argument"
//CHK_AIR: !"long", !"b{{[^"]*}}", !"air.indirect_argument"
//CHK_AIR: !"uint", !"c{{[^"]*}}", !"air.indirect_argument"
//CHK_AIR: !"int", !"d{{[^"]*}}", !"air.indirect_argument"
//CHK_AIR: !"float", !"e{{[^"]*}}", !"air.indirect_argument"

[numthreads(1,1,1)]
void computeMain(uniform uint64_t* a, uniform int64_t* b, uniform uint32_t* c, uniform int32_t* d, uniform float* e)
{
    outputBuffer[0] = (uint64_t)(*a);
    outputBuffer[1] = (uint64_t)(*b);
    outputBuffer[2] = (uint64_t)(*c);
    outputBuffer[3] = (uint64_t)(*d);
    outputBuffer[4] = (uint64_t)(*e);
}
