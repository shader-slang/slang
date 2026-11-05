// A cast from a pointer to a signed integer such as `int64_t` is written on SPIR-V as an
// `OpConvertPtrToU` to the unsigned integer of the same width, then an `OpBitcast` to the signed
// type, because `OpConvertPtrToU` requires an unsigned result type
// (https://github.com/shader-slang/slang/issues/14087).

//TEST:SIMPLE(filecheck=SPIRV):-target spirv -emit-spirv-directly -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -output-using-type

//TEST_INPUT: set q = ubuffer(data=[3 7 11 13], stride=4);
uniform int* q;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

// SPIRV-NOT: OpConvertPtrToU %long
// SPIRV: [[U:%[0-9a-zA-Z_]+]] = OpConvertPtrToU %ulong
// SPIRV-NEXT: %s = OpBitcast %long [[U]]
// SPIRV-NOT: OpConvertPtrToU %long
// SPIRV: %u = OpConvertPtrToU %ulong
// SPIRV-NOT: OpConvertPtrToU %long

// BUF: 1
// BUF-NEXT: 7
// BUF-NEXT: 11
// BUF-NEXT: 12
// BUF-NEXT: 13

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int64_t s = (int64_t)q;
    uint64_t u = (uint64_t)q;
    // The signed and unsigned casts carry the same address bits.
    outputBuffer[0] = uint64_t(s) == u ? 1 : 0;
    // A signed address converts back to a pointer, also after signed arithmetic on it.
    outputBuffer[1] = Ptr<int>(int64_t(q))[1];
    outputBuffer[2] = Ptr<int>(s + 8)[0];
    // `q + 3` is 3 * sizeof(int) = 12 bytes past `q`.
    outputBuffer[3] = int((int64_t)(q + 3) - s);
    outputBuffer[4] = Ptr<int>(u)[3];
}
