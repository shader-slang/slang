//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=8):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

//METAL: ulong{{.*}}device{{ *\*}} a
//METAL-NEXT: long{{.*}}device{{ *\*}} b
//METAL-NEXT: uint{{.*}}device{{ *\*}} c
//METAL-NEXT: int{{.*}}device{{ *\*}} d
//METAL-NEXT: float{{.*}}device{{ *\*}} e

// "indirect_argument" means a pointer
//METALLIB: !"ulong", !"a{{[^"]*}}", !"air.indirect_argument"
//METALLIB: !"long", !"b{{[^"]*}}", !"air.indirect_argument"
//METALLIB: !"uint", !"c{{[^"]*}}", !"air.indirect_argument"
//METALLIB: !"int", !"d{{[^"]*}}", !"air.indirect_argument"
//METALLIB: !"float", !"e{{[^"]*}}", !"air.indirect_argument"

[numthreads(1,1,1)]
void computeMain(uniform uint64_t* a, uniform int64_t* b, uniform uint32_t* c, uniform int32_t* d, uniform float* e)
{
    outputBuffer[0] = (uint64_t)(*a);
    outputBuffer[1] = (uint64_t)(*b);
    outputBuffer[2] = (uint64_t)(*c);
    outputBuffer[3] = (uint64_t)(*d);
    outputBuffer[4] = (uint64_t)(*e);
}
