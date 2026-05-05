// Regression test: `legalizeDefUse` (called from `eliminateMultiLevelBreak`)
// must not leave a fully-expanded `MakeStruct(MakeArray(zeros))` default-init
// store in the emitted source for an aggregate var that is later
// unconditionally overwritten by user code.
//
// Trigger pattern: a `[ForceInline]` helper function that
//   1. declares a local struct-of-array,
//   2. fully writes the array via a `[ForceUnroll]` for-loop, and
//   3. returns the struct by value.
// After inlining + `[ForceUnroll]` lowering + `eliminateMultiLevelBreak`,
// the var ends up with a `commonDominator` that is a loop's condition block
// while having uses past the loop's break block. `legalizeDefUse` then
// hoists the var out and emits a default-init store. Without the fix that
// store is the fully-expanded form `MakeStruct(MakeArray(0,0,...))`, which
// every backend renders as a literal zero array followed by a struct copy.
// In Metal / HLSL / GLSL output it shows up as
//   `array<float, int(8)> _S = { 0.0, 0.0, ... }; output_0.data_0 = _S;`,
// in WGSL as `var _S : array<f32, i32(8)> = array<f32, i32(8)>( 0.0f, ... )`,
// and in SPIR-V assembly as
//   `OpConstantComposite ... %float_0 %float_0 ... %float_0`.
//
// The user's `[ForceUnroll]` writes overwrite every element, so the
// initialization is dead. For larger arrays this bloats codegen with
// hundreds of bytes of zero-stores in the kernel prologue and prevents
// downstream register allocation from keeping the value in registers.
//
// The fix emits `kIROp_DefaultConstruct` directly from `legalizeDefUse`
// and runs `removeRawDefaultConstructors` after `eliminateMultiLevelBreak`
// so the dead default-init is stripped before code emission.

//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):  -target hlsl  -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):  -target glsl  -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):  -target wgsl  -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):   -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-mtl -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -compute -entry computeMain -output-using-type -profile cs_6_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -compute -entry computeMain -output-using-type

static const int N = 8;

struct OutVec
{
    float data[N];
};

uniform RWStructuredBuffer<float> outBuf;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outBuf

[ForceInline]
OutVec makeBag()
{
    OutVec output;

    [ForceUnroll]
    for (int i = 0; i < N; i++)
        output.data[i] = float(i);

    return output;
}

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    if (tid != 0)
        return;

    let result = makeBag();
    for (int i = 0; i < N; i++)
        outBuf[tid * uint(N) + uint(i)] = result.data[i];

    // BUF: 0.0
    // BUF-NEXT: 1.0
    // BUF-NEXT: 2.0
    // BUF-NEXT: 3.0
    // BUF-NEXT: 4.0
    // BUF-NEXT: 5.0
    // BUF-NEXT: 6.0
    // BUF-NEXT: 7.0
}

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain
// METAL-NOT: {{0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?}}

// HLSL:     void computeMain
// HLSL-NOT: {{0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?}}

// GLSL:     void main(
// GLSL-NOT: {{0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?}}

// WGSL:     fn computeMain
// WGSL-NOT: {{0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?}}

// SPV:     OpEntryPoint
// SPV-NOT: OpConstantComposite {{.*}} %float_0 %float_0 %float_0 %float_0 %float_0 %float_0 %float_0 %float_0
