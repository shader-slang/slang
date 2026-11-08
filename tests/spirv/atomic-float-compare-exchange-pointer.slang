// A floating-point atomic compare-exchange through a physical storage buffer pointer is done as a
// bitwise one on the same memory, through a bitcast of the pointer.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly

//TEST_INPUT: set floats = ubuffer(data=[0.0 2.0], stride=4)
uniform Atomic<float>* floats;

//TEST_INPUT: set doubles = ubuffer(data=[0 0], stride=8)
uniform Atomic<double>* doubles;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = asuint(floats[0].compareExchange(0.0f, 1.0f));
    outputBuffer[1] = asuint(floats[tid.x + 1].compareExchange(0.0f, 5.0f));
    outputBuffer[2] = asuint(floats[0].load());
    outputBuffer[3] = asuint(floats[1].load());

    uint lo, hi;
    asuint(doubles[1].compareExchange(0.0, 1.5), lo, hi);
    outputBuffer[4] = lo;
    outputBuffer[5] = hi;
    asuint(doubles[1].load(), lo, hi);
    outputBuffer[6] = lo;
    outputBuffer[7] = hi;
}

// SPIRV-NOT: OpAtomicCompareExchange %float
// SPIRV-NOT: OpAtomicCompareExchange %double
// SPIRV-DAG: OpBitcast %_ptr_PhysicalStorageBuffer_uint
// SPIRV-DAG: OpBitcast %_ptr_PhysicalStorageBuffer_ulong
// SPIRV-DAG: OpAtomicCompareExchange %uint
// SPIRV-DAG: OpAtomicCompareExchange %ulong

// BUF: 0
// BUF-NEXT: 1073741824
// BUF-NEXT: 1065353216
// BUF-NEXT: 1073741824
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1073217536
