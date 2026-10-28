// A NaN constant keeps its sign and payload in the text targets (#13909). Each writes it as a bit
// cast of its bit pattern rather than as `0.0 / 0.0`, which gives one fixed NaN.
//
// An f32 signaling NaN is not covered here: the constant fold of `bit_cast<float>` quiets it before
// it reaches the emitters (#13845). The `double` constants cover a signaling NaN instead.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMainF32 -stage compute -O1
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -O1

// glslang accepts the module-scope `const` array, whose NaN keeps `0.0 / 0.0` (see below).
//TEST:SIMPLE(filecheck=GLSLANG): -target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute -O1

RWStructuredBuffer<float> floatBuffer;
RWStructuredBuffer<double> doubleBuffer;

// A module-scope constant whose NaN is emitted in a module-scope declaration. GLSL needs a constant
// expression there, and glslang does not fold `uintBitsToFloat`, so GLSL keeps `0.0 / 0.0` for it.
static const float kTable[3] = { bit_cast<float>(0xFFC12345u), 1.0f, bit_cast<float>(0x7F800000u) };

// GLSL: const float {{.*}}[3] = float[]((0.0 / 0.0), 1.0, (1.0 / 0.0));
// HLSL: static const float {{.*}}[int(3)] = { asfloat(0xFFC12345U), 1.0f, (1.0f / 0.0f) };
// CPP: static const FixedArray<float, 3> {{.*}} = { { U32_asfloat(0xFFC12345U), 1.0f, SLANG_INFINITY } };
// CUDA: __device__ static const FixedArray<float, 3> {{.*}} = { {
// CUDA-NEXT: __builtin_bit_cast(float, 0xFFC12345U), 1.0f, __builtin_bit_cast(float, 0x7F800000U)

void writeFloats(uint index)
{
    floatBuffer[0] = kTable[index];
    // A negative quiet NaN with a payload.
    floatBuffer[1] = bit_cast<float>(0xFFC12345u);
    // A positive quiet NaN with a payload.
    floatBuffer[2] = bit_cast<float>(0x7FC12345u);
    // The canonical quiet NaN.
    floatBuffer[3] = bit_cast<float>(0x7FC00000u);
    // Infinities are unchanged.
    floatBuffer[4] = bit_cast<float>(0x7F800000u);
    floatBuffer[5] = bit_cast<float>(0xFF800000u);
}

// GLSL: uintBitsToFloat(0xFFC12345u)
// GLSL: uintBitsToFloat(0x7FC12345u)
// GLSL: uintBitsToFloat(0x7FC00000u)
// GLSL: (1.0 / 0.0)
// GLSL: (-1.0 / 0.0)
// HLSL: asfloat(0xFFC12345U)
// HLSL: asfloat(0x7FC12345U)
// HLSL: asfloat(0x7FC00000U)
// HLSL: (1.0f / 0.0f)
// HLSL: (-1.0f / 0.0f)
// METAL: as_type<float>(0xFFC12345U)
// METAL: as_type<float>(0x7FC12345U)
// METAL: as_type<float>(0x7FC00000U)
// METAL: (1.0 / 0.0)
// METAL: (-1.0 / 0.0)
// CPP: U32_asfloat(0xFFC12345U)
// CPP: U32_asfloat(0x7FC12345U)
// CPP: U32_asfloat(0x7FC00000U)
// CPP: SLANG_INFINITY
// CPP: (-SLANG_INFINITY)
// CUDA: __builtin_bit_cast(float, 0xFFC12345U)
// CUDA: __builtin_bit_cast(float, 0x7FC12345U)
// CUDA: __builtin_bit_cast(float, 0x7FC00000U)
// CUDA: SLANG_INFINITY
// CUDA: (-SLANG_INFINITY)

[numthreads(1, 1, 1)]
void computeMainF32(uint3 tid: SV_DispatchThreadID)
{
    writeFloats(tid.x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    writeFloats(tid.x);
    // A signaling NaN with a payload.
    doubleBuffer[0] = bit_cast<double>(0x7FF0000000000123ull);
    // A negative quiet NaN with a payload.
    doubleBuffer[1] = bit_cast<double>(0xFFF8000000000ABCull);
}

// GLSL: packDouble2x32(uvec2(0x00000123u, 0x7FF00000u))
// GLSL: packDouble2x32(uvec2(0x00000ABCu, 0xFFF80000u))
// HLSL: asdouble(0x00000123U, 0x7FF00000U)
// HLSL: asdouble(0x00000ABCU, 0xFFF80000U)
// CPP: U32_asdouble(0x00000123U, 0x7FF00000U)
// CPP: U32_asdouble(0x00000ABCU, 0xFFF80000U)
// CUDA: __builtin_bit_cast(double, 0x7FF0000000000123ULL)
// CUDA: __builtin_bit_cast(double, 0xFFF8000000000ABCULL)

// GLSLANG: OpEntryPoint
