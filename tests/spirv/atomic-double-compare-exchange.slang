// SPIR-V and GLSL have no floating-point atomic compare-exchange, so a compare-exchange on a
// `double` is done as a bitwise one on a `uint64_t` view of the same buffer.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -vk -shaderobj -output-using-type -render-feature double,int64,atomic-int64
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -vk -shaderobj -output-using-type -render-feature double,int64,atomic-int64 -emit-spirv-via-glsl

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=8):name=doubles
RWStructuredBuffer<Atomic<double>> doubles;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint lo, hi;
    // The stored value matches, so it is replaced.
    asuint(doubles[tid.x + 1].compareExchange(0.0, 1.5), lo, hi);
    outputBuffer[0] = lo;
    outputBuffer[1] = hi;
    // The stored value does not match, so it is kept.
    asuint(doubles[1].compareExchange(0.0, 2.5), lo, hi);
    outputBuffer[2] = lo;
    outputBuffer[3] = hi;
    asuint(doubles[1].load(), lo, hi);
    outputBuffer[4] = lo;
    outputBuffer[5] = hi;
    asuint(doubles[0].load(), lo, hi);
    outputBuffer[6] = lo;
    outputBuffer[7] = hi;
}

// SPIRV-NOT: OpAtomicCompareExchange %double
// SPIRV: OpAtomicCompareExchange %ulong
// SPIRV-NOT: OpAtomicCompareExchange %double

// GLSL: atomicCompSwap({{.*}}doubleBitsToUint64

// BUF: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1073217536
// BUF-NEXT: 0
// BUF-NEXT: 1073217536
// BUF-NEXT: 0
// BUF-NEXT: 0
