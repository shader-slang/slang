// WGSL rows check the wrapped non-finite literals at the default optimization level and at -O0,
// where fewer values are folded before emission. FINITE re-reads the default output for the
// values that must stay bare.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=FINITE): -target wgsl -entry computeMain -stage compute
// SPEC and SPECINF check module-scope `override` initializers of specialization constants.
//TEST:SIMPLE(filecheck=SPEC): -target wgsl -entry specMain -stage compute
//TEST:SIMPLE(filecheck=SPECINF): -target wgsl -entry specNonFiniteMain -stage compute
// Tint validates the emitted WGSL (via wgsl-spirv-asm) where the downstream compiler is available.
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -entry specMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -output-using-type

// Issue #13657: WGSL const-evaluates `unpack2x16float` of a literal operand, and a const-expression
// that evaluates to Inf or NaN is a shader-creation error. Inside a function body, literal half
// bits that are Inf or NaN are therefore passed through `_slang_nonConstU32`, which makes the call
// a runtime expression.

//TEST_INPUT:ubuffer(data=[0x3C00 0xC0003C00], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

// WGSL lets an implementation assume values are finite at run time, so results computed from a
// non-finite half (in either half of the packed pair, since the builtin's whole vec2 result is
// evaluated) are not checked: the shader only has to be accepted.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):name=nonFiniteBuffer
RWStructuredBuffer<float> nonFiniteBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// The array uses NaN payloads that appear nowhere else, so only its own initializer can satisfy
// the 31745/64513/32257 lines below.
static const float kNonFinite[3] = { f16tof32(0x7C01u), f16tof32(0xFC01u), f16tof32(0x7E01u) };

// The helper is defined once, and each non-finite literal goes through it: +Inf, -Inf and NaN
// through `f16tof32` (whose `$0` expansion adds a pair of parentheses), +Inf through
// `unpackHalf2x16ToFloat` (a plain-name call), a pair whose only non-finite half is the high one
// that `f16tof32` ignores, and the static-const array.
// WGSL: fn _slang_nonConstU32(x : u32) -> u32
// WGSL-NOT: fn _slang_nonConstU32
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(31744)))).x
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(64512)))).x
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(32256)))).x
// WGSL-DAG: unpack2x16float(_slang_nonConstU32(u32(31744))).x
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(2080390144)))).x
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(31745)))).x
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(64513)))).x
// WGSL-DAG: unpack2x16float({{\(*}}_slang_nonConstU32(u32(32257)))).x

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
    nonFiniteBuffer[4] = f16tof32(0x7C003C00u);
    nonFiniteBuffer[5] = kNonFinite[tid.x % 3];

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
    // BUF-NEXT: 65504.0
    outputBuffer[5] = f16tof32(0x7BFF7BFFu);
    float2 v = f16tof32(uint2(0x3C00u, 0x4000u));
    // BUF-NEXT: 3.0
    outputBuffer[6] = v.x + v.y;
}

// A specialization constant is a WGSL `override` declaration at module scope, where a call to a
// user-declared function is not allowed. Its finite initializer keeps the bare builtin call.
[SpecializationConstant]
const float kSpecOne = f16tof32(0x3C00u);
[SpecializationConstant]
const float kSpecTwo = unpackHalf2x16ToFloat(0x40003C00u).y;

// SPEC-NOT: _slang_nonConstU32
// SPEC: override kSpecOne{{.*}} = {{\(*}}unpack2x16float({{\(*}}u32(15360)
// SPEC: override kSpecTwo{{.*}} = unpack2x16float(u32(1073757184)).y
// SPEC-NOT: _slang_nonConstU32

[numthreads(1, 1, 1)]
void specMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = kSpecOne + kSpecTwo;
}

// A non-finite override-expression is a pipeline-creation error, so no spelling of this initializer
// is valid WGSL. It is left bare, so Tint reports the non-finite value rather than a call to
// `_slang_nonConstU32` that the user never wrote.
[SpecializationConstant]
const float kSpecInf = f16tof32(0x7C00u);

// SPECINF-NOT: _slang_nonConstU32
// SPECINF: override kSpecInf{{.*}} = {{\(*}}unpack2x16float({{\(*}}u32(31744)
// SPECINF-NOT: _slang_nonConstU32

[numthreads(1, 1, 1)]
void specNonFiniteMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = kSpecInf;
}
