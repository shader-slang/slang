// Regression test for GitHub issue #14084: on GLSL, `bit_cast<uint64_t>(double)` and
// `bit_cast<int64_t>(double)` emit `uint64_t(doubleBitsToUint64(x))` and
// `int64_t(doubleBitsToInt64(x))`. When no other 64-bit integer type appears in the shader,
// the GLSL must still request GL_EXT_shader_explicit_arithmetic_types_int64, or glslang
// rejects it. The unsigned and signed casts compile separately (`SIGNED`) so that neither
// can request the extension on the other's behalf, and each bit_cast result is used exactly
// once so that it is folded into its use instead of declaring a 64-bit integer variable.

//TEST:SIMPLE(filecheck=GLSL_U): -target glsl -stage compute -entry computeMain -DSIGNED=0
//TEST:SIMPLE(filecheck=GLSL_I): -target glsl -stage compute -entry computeMain -DSIGNED=1
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -emit-spirv-via-glsl -render-feature double,int64 -output-using-type -xslang -DSIGNED=0
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -emit-spirv-via-glsl -render-feature double,int64 -output-using-type -xslang -DSIGNED=1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type -xslang -DSIGNED=0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type -xslang -DSIGNED=1

// The NOT lines reject a 64-bit integer declaration, vector or literal, any of which would
// request the extension by itself and hide a regression in the bit_cast arms.
// GLSL_U: #extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
// GLSL_U-NOT: {{int64_t [_a-zA-Z]|64vec|[0-9]U?L[^a-zA-Z0-9_]}}
// GLSL_U: uint64_t(doubleBitsToUint64(
// GLSL_U-NOT: {{int64_t [_a-zA-Z]|64vec|[0-9]U?L[^a-zA-Z0-9_]}}
// GLSL_I: #extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
// GLSL_I-NOT: {{int64_t [_a-zA-Z]|64vec|[0-9]U?L[^a-zA-Z0-9_]}}
// GLSL_I: int64_t(doubleBitsToInt64(
// GLSL_I-NOT: {{int64_t [_a-zA-Z]|64vec|[0-9]U?L[^a-zA-Z0-9_]}}

#if SIGNED
typealias Bits64 = int64_t;
#else
typealias Bits64 = uint64_t;
#endif

// Each pair is the (low, high) words of one double.
//TEST_INPUT:ubuffer(data=[0x89ABCDEF 0x01234567 0x00000000 0x3FF00000 0xDEADBEEF 0x40091EB8 0x00000000 0xC0040000], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    double a = asdouble(inputBuffer[0], inputBuffer[1]);
    double b = asdouble(inputBuffer[2], inputBuffer[3]);
    double c = asdouble(inputBuffer[4], inputBuffer[5]);
    double d = asdouble(inputBuffer[6], inputBuffer[7]);

    // BUF: 2309737967
    outputBuffer[0] = uint(bit_cast<Bits64>(a));
    // BUF-NEXT: 1072693248
    outputBuffer[1] = uint(bit_cast<Bits64>(b) >> 32);
    // BUF-NEXT: 3735928559
    outputBuffer[2] = uint(bit_cast<Bits64>(c));
    // BUF-NEXT: 3221487616
    outputBuffer[3] = uint(bit_cast<Bits64>(d) >> 32);
}
