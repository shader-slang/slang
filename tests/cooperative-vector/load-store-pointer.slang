//TEST:SIMPLE(filecheck=CHECK):-entry computeMain -stage compute -target SPIRV

// HLSL doesn't support structured buffers for CoopVec
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -dx12-experimental -use-dxil -output-using-type -profile cs_6_8 -Xslang... -Xdxc -Vd -X.

// CHECK: type: int32_t
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

struct A
{
int32_t* ptr;
}

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4),name=buf
ConstantBuffer<A> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
ConstantBuffer<int32_t*> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let a = CoopVec<int32_t, 4>.load(inputBuffer.ptr, 0);
    let b = coopVecLoad<4>(inputBuffer.ptr, 4*4);
    b.store(outputBuffer, 0);
    a.store(outputBuffer, 4*4);
}
