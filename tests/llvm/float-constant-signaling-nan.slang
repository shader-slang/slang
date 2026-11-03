// A `float` constant that is a signaling NaN keeps its bits on the LLVM targets.
// `ConstantFP::get` narrows a `double` to `float` with `APFloat::convert`, which sets the quiet
// bit, so 0x7F800001 would be emitted as 0x7FC00001. The constant is built from its bits instead.
// See float-constant-signaling-nan-ir.slang for the constants in the LLVM IR.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):name=indexBuffer
StructuredBuffer<uint> indexBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// The NaNs in an aggregate constant keep their bits too: 0x7F800001, 0xFF800001, 0x7FBFFFFF
// (signaling) and 0x7FC12345 (quiet).
static const float kNaNs[4] = {
    bit_cast<float>(0x7F800001u),
    bit_cast<float>(0xFF800001u),
    bit_cast<float>(0x7FBFFFFFu),
    bit_cast<float>(0x7FC12345u),
};

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // CHECK: 7F800001
    outputBuffer[0] = bit_cast<float>(0x7F800001u);
    // CHECK-NEXT: FF800001
    outputBuffer[1] = bit_cast<float>(0xFF800001u);
    // CHECK-NEXT: 7FC12345
    outputBuffer[2] = bit_cast<float>(0x7FC12345u);
    // CHECK-NEXT: 3F800000
    outputBuffer[3] = 1.0;

    // CHECK-NEXT: 7F800001
    outputBuffer[4] = kNaNs[indexBuffer[0]];
    // CHECK-NEXT: FF800001
    outputBuffer[5] = kNaNs[indexBuffer[1]];
    // CHECK-NEXT: 7FBFFFFF
    outputBuffer[6] = kNaNs[indexBuffer[2]];
    // CHECK-NEXT: 7FC12345
    outputBuffer[7] = kNaNs[indexBuffer[3]];
}
