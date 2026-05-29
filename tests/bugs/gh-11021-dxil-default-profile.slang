//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_5_1
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain -profile cs_5_1
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain -profile cs_5_0
//TEST:SIMPLE(filecheck=LIB): -target dxil-assembly -entry computeMain -profile cs_5_1 -whole-program
//TEST(windows):COMPILE: tests/bugs/gh-11021-dxil-default-profile.slang -target dxil -o tests/bugs/gh-11021-dxil-default-profile.dxil
//TEST(windows):COMPILE: tests/bugs/gh-11021-dxil-default-profile.slang -target dxil -entry computeMain -profile cs_5_1 -whole-program -o tests/bugs/gh-11021-dxil-whole-program-profile.dxil

// DXIL: define void @computeMain()

// LIB: !dx.shaderModel = !{![[SM:[0-9]+]]}
// LIB: ![[SM]] = !{!"lib", i32 6, i32 3}

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
