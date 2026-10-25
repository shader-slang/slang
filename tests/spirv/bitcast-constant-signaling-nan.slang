// A bit cast of a constant to `float` is folded, and the folded value is held as a double.
// Widening or narrowing a signaling NaN with a hardware conversion quiets it, so the fold must
// move the bits instead: `bit_cast<float>(0x7F800001u)` must keep 0x7F800001, not 0x7FC00001.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -O1
//TEST:SIMPLE(filecheck=FOLD): -target spirv -stage compute -entry computeMain -O1
//TEST:SIMPLE(filecheck=VEC): -target spirv -stage compute -entry computeMain -O1

//TEST:SIMPLE(filecheck=NOQ): -target spirv -stage compute -entry computeMain -O1
//TEST:SIMPLE(filecheck=NOQ): -target spirv -stage compute -entry computeMain -O0

//TEST:SIMPLE(filecheck=O0): -target spirv-asm -stage compute -entry computeMain -O0

//TEST:SIMPLE(filecheck=CAST): -target spirv -stage compute -entry castMain -O1
//TEST:SIMPLE(filecheck=WIDEN): -target spirv -stage compute -entry widenMain -O1

// Signaling NaNs folded through `float` and back to `uint` keep their bits:
// 0x7F800001, 0xFF800001 and 0x7FBFFFFF.
// CHECK-DAG: OpConstant %uint 2139095041
// CHECK-DAG: OpConstant %uint 4286578689
// CHECK-DAG: OpConstant %uint 2143289343

// The two lanes of the vector bit cast keep their bits, 0xFF800001 and 0x7FA00000.
// VEC-DAG: OpConstant %uint 4286578689
// VEC-DAG: OpConstant %uint 2141192192

// At -O1 every bit cast is folded, so the constants above are folded results, not the inputs.
// FOLD-NOT: OpBitcast

// The quieted patterns 0x7FC00001, 0xFFC00001 and 0x7FFFFFFF appear nowhere in the module.
// NOQ-NOT: OpConstant %uint 2143289345
// NOQ-NOT: OpConstant %uint 4290772993
// NOQ-NOT: OpConstant %uint 2147483647

// At -O0, `storedNan` stays a float constant, and its bits are 0x7F800001 rather than 0x7FC00001.
// O0: OpConstant %float 0x1.000002p+128
// O0-NOT: OpConstant %float 0x1.800002p+128

// A value conversion from `double` to `float` quiets a signaling NaN, as it does at run time:
// the double 0x7FF0000040000000 becomes 0x7FC00002, not 0x7F800002.
// CAST: OpConstant %uint 2143289346
// CAST-NOT: OpConstant %uint 2139095042

// A value conversion from `float` to `double` quiets it too: the float 0x7F800001 becomes the
// double 0x7FF8000020000000, whose high word is 0x7FF80000, not 0x7FF00000.
// WIDEN: OpConstant %uint 2146959360
// WIDEN-NOT: OpConstant %uint 2146435072

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -Xslang -O1
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -Xslang -O0

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

uint roundTrip(uint bits)
{
    return bit_cast<uint>(bit_cast<float>(bits));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Signaling NaNs: positive, negative, and the largest positive payload through `int`.
    outputBuffer[0] = roundTrip(0x7F800001u);
    outputBuffer[1] = roundTrip(0xFF800001u);
    outputBuffer[2] = bit_cast<uint>(bit_cast<float>(int(0x7FBFFFFF)));
    // Quiet NaNs with a payload, the infinities and ordinary values are unchanged.
    outputBuffer[3] = roundTrip(0x7FC12345u);
    outputBuffer[4] = roundTrip(0xFFC00000u);
    outputBuffer[5] = roundTrip(0x7F800000u);
    outputBuffer[6] = roundTrip(0xFF800000u);
    outputBuffer[7] = roundTrip(0x3F800000u);
    outputBuffer[8] = roundTrip(0x00000001u);
    outputBuffer[9] = roundTrip(0x80000000u);
    // A vector bit cast is split per element, and each constant lane keeps its bits. Checked by
    // VEC at compile time only: CI also runs the Vulkan lanes through GLSL
    // (`-emit-spirv-via-glsl`), and the GLSL emitter writes every NaN constant as `0.0 / 0.0`,
    // so there these elements read back 0x7FC00000.
    float2 v = bit_cast<float2>(uint2(0xFF800001u, 0x7FA00000u));
    outputBuffer[10] = asuint(v.x);
    outputBuffer[11] = asuint(v.y);
    // Not checked at run time: at -O0 the driver may fold `asuint` of a float constant and
    // quiet the NaN itself.
    float storedNan = bit_cast<float>(0x7F800001u);
    outputBuffer[12] = asuint(storedNan);
}

// BUF: 7F800001
// BUF-NEXT: FF800001
// BUF-NEXT: 7FBFFFFF
// BUF-NEXT: 7FC12345
// BUF-NEXT: FFC00000
// BUF-NEXT: 7F800000
// BUF-NEXT: FF800000
// BUF-NEXT: 3F800000
// BUF-NEXT: 1
// BUF-NEXT: 80000000

[numthreads(1, 1, 1)]
void castMain()
{
    outputBuffer[0] = asuint(float(bit_cast<double>(0x7FF0000040000000ull)));
}

[numthreads(1, 1, 1)]
void widenMain()
{
    uint64_t bits = bit_cast<uint64_t>(double(bit_cast<float>(0x7F800001u)));
    outputBuffer[0] = uint(bits >> 32);
}
