// A NaN constant keeps its sign and payload when the generated code runs (#13909). The buffers
// are printed as raw 32-bit words, so the checks see the exact bits that were stored.
//
// An f32 signaling NaN is not covered here: the constant fold of `bit_cast<float>` quiets it before
// it reaches the emitters (#13845). The `double` constants cover a signaling NaN instead.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=floatBuffer
RWStructuredBuffer<float> floatBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=8):out,name=doubleBuffer
RWStructuredBuffer<double> doubleBuffer;

static const float kTable[3] = { bit_cast<float>(0xFFC12345u), 1.0f, bit_cast<float>(0x7F800000u) };

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // A negative quiet NaN with a payload, read from a module-scope constant.
    floatBuffer[0] = kTable[tid.x];
    // A negative quiet NaN with a payload.
    floatBuffer[1] = bit_cast<float>(0xFFC12345u);
    // A positive quiet NaN with a payload.
    floatBuffer[2] = bit_cast<float>(0x7FC12345u);
    // The canonical quiet NaN.
    floatBuffer[3] = bit_cast<float>(0x7FC00000u);
    // Infinities.
    floatBuffer[4] = bit_cast<float>(0x7F800000u);
    floatBuffer[5] = bit_cast<float>(0xFF800000u);

    // A signaling NaN with a payload.
    doubleBuffer[0] = bit_cast<double>(0x7FF0000000000123ull);
    // A negative quiet NaN with a payload.
    doubleBuffer[1] = bit_cast<double>(0xFFF8000000000ABCull);
}

// CHECK: FFC12345
// CHECK-NEXT: FFC12345
// CHECK-NEXT: 7FC12345
// CHECK-NEXT: 7FC00000
// CHECK-NEXT: 7F800000
// CHECK-NEXT: FF800000
// CHECK-NEXT: 123
// CHECK-NEXT: 7FF00000
// CHECK-NEXT: ABC
// CHECK-NEXT: FFF80000
