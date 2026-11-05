// byte-address-buffer-grouped-bool-11593.slang

// Feature test for https://github.com/shader-slang/slang/issues/11593. A `bool` vector whose
// promised alignment covers only part of it is accessed one component at a time, never as a
// `vector<bool, 2>` group: SPIR-V cannot place `OpTypeBool` in a storage buffer, and on CPU a
// `vector<bool, 2>` is 2 bytes wide while each `bool` in a byte-address buffer occupies 4 bytes.
// Plain `Load<T>` at a constant location that is 8- but not 16-aligned takes the same grouping decision, so it is
// covered too.

//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[1 0 1 1 0 1 0 1 0 0 0 0 0 0 0 0], stride=4):name=buffer
RWByteAddressBuffer buffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// SPV-NOT: v2bool
// SPV-COUNT-8: OpINotEqual %bool
// SPV-NOT: v2bool

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    bool4 a = buffer.LoadAligned<bool4>(tid.x * 16, 8);
    bool4 b = buffer.Load<bool4>(8);
    buffer.Store<bool4>(tid.x * 16 + 32, a, 8);

    outputBuffer[0] = a.x ? 1 : 0;
    outputBuffer[1] = a.y ? 1 : 0;
    outputBuffer[2] = a.z ? 1 : 0;
    outputBuffer[3] = a.w ? 1 : 0;
    outputBuffer[4] = b.x ? 1 : 0;
    outputBuffer[5] = b.y ? 1 : 0;
    outputBuffer[6] = b.z ? 1 : 0;
    outputBuffer[7] = b.w ? 1 : 0;
    for (uint i = 0; i < 4; ++i)
        outputBuffer[8 + i] = int(buffer.Load<uint>(32 + 4 * i));
}

// CHECK: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
