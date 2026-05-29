//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_5_1
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain -profile cs_5_1
//TEST(windows):COMPILE: tests/bugs/gh-11021-dxil-default-profile.slang -target dxil -o tests/bugs/gh-11021-dxil-default-profile.dxil
//TEST(windows):COMPILE: tests/bugs/gh-11021-dxil-default-profile.slang -target dxil -entry computeMain -profile cs_5_1 -whole-program -o tests/bugs/gh-11021-dxil-whole-program-profile.dxil

// DXIL: define void @computeMain()

StructuredBuffer<float> buffer0;
StructuredBuffer<float> buffer1;
RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    uint index = threadId.x;
    result[index] = buffer0[index] + buffer1[index];
}
