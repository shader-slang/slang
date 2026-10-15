//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=FINITE): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPEC): -target wgsl -entry specMain -stage compute
// Tint validates the emitted WGSL (via wgsl-spirv-asm) where the downstream compiler is available.
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -output-using-type

// Issue #13657: WGSL const-evaluates `unpack2x16float` of a literal operand, and a const-expression
// that evaluates to Inf or NaN is a shader-creation error. Literal half bits that are Inf or NaN
// must therefore be evaluated at run time, via `_slang_nonConstU32`.

//TEST_INPUT:ubuffer(data=[0x3C00 0xC0003C00], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

// WGSL lets an implementation assume values are finite at run time, so the non-finite results
// are not checked: the shader only has to be accepted.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):name=nonFiniteBuffer
RWStructuredBuffer<float> nonFiniteBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

static const float kNonFinite[3] = { f16tof32(0x7C00u), f16tof32(0xFC00u), f16tof32(0x7E00u) };

// The helper is defined once, and each non-finite literal goes through it: +Inf, -Inf, NaN,
// `unpackHalf2x16ToFloat`, and a pair whose high half (0x7C00) is +Inf even though `f16tof32`
// only reads the low half (0x3C00 = 1.0), since the builtin's whole vec2 result is evaluated.
// WGSL: fn _slang_nonConstU32(x : u32) -> u32
// WGSL-NOT: fn _slang_nonConstU32
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(31744)))).x
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(64512)))).x
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(32256)))).x
// WGSL-DAG: unpack2x16float(_slang_nonConstU32(u32(31744))).x
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(2080390144)))).x

// Finite literals (1.0, -2.0, the largest finite pair 0x7BFF7BFF) remain const-expressions that
// Tint can fold; runtime operands stay unwrapped.
// FINITE: unpack2x16float({{\(*}}u32(15360)
// FINITE-NOT: _slang_nonConstU32(u32(15360))
// FINITE-NOT: _slang_nonConstU32(u32(49152))
// FINITE-NOT: _slang_nonConstU32(u32(2080340991))
// FINITE-NOT: _slang_nonConstU32({{.*}}inputBuffer

// SPV: OpEntryPoint

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    nonFiniteBuffer[0] = f16tof32(0x7C00u);
    nonFiniteBuffer[1] = f16tof32(0xFC00u);
    nonFiniteBuffer[2] = f16tof32(0x7E00u);
    nonFiniteBuffer[3] = unpackHalf2x16ToFloat(0x7C00u).x;
    nonFiniteBuffer[4] = kNonFinite[tid.x % 3];

    // BUF: 1.0
    outputBuffer[0] = f16tof32(0x3C00u);
    // BUF-NEXT: -2.0
    outputBuffer[1] = f16tof32(0xC000u);
    // BUF-NEXT: 1.0
    outputBuffer[2] = f16tof32(inputBuffer[0]);
    float2 r = unpackHalf2x16ToFloat(inputBuffer[1]);
    // BUF-NEXT: 1.0
    outputBuffer[3] = r.x;
    // BUF-NEXT: -2.0
    outputBuffer[4] = r.y;
    // BUF-NEXT: 1.0
    outputBuffer[5] = f16tof32(0x7C003C00u);
    // BUF-NEXT: 65504.0
    outputBuffer[6] = f16tof32(0x7BFF7BFFu);
    float2 v = f16tof32(uint2(0x3C00u, 0x4000u));
    // BUF-NEXT: 3.0
    outputBuffer[7] = v.x + v.y;
}

// A specialization constant is a WGSL `override` declaration at module scope, where a call to a
// user-declared function is not allowed. Its finite initializer must keep the bare builtin call.
[SpecializationConstant]
const float kSpecOne = f16tof32(0x3C00u);
[SpecializationConstant]
const float kSpecTwo = unpackHalf2x16ToFloat(0x40003C00u).y;

// SPEC: override kSpecOne{{.*}} = {{\(*}}unpack2x16float({{\(*}}u32(15360)
// SPEC: override kSpecTwo{{.*}} = unpack2x16float(u32(1073757184)).y
// SPEC-NOT: _slang_nonConstU32

[numthreads(1, 1, 1)]
void specMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = kSpecOne + kSpecTwo;
}
