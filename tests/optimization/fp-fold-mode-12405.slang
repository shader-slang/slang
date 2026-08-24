// Regression test for issue #12405. The IR peephole arithmetic-identity folds that drop a
// floating-point operand (`0 + x`, `x + 0`, `x - 0`, `x - x`, `x * 0`) are value-preserving only
// under fast math: for float they discard the signed-zero / NaN / Inf distinctions IEEE keeps
// (e.g. `0.0 + (-0.0)` is `+0.0`, `NaN - NaN` is `NaN`, not `0`). The effective mode is resolved
// per instruction from the global `-fp-mode` option, overridden by a function's
// `IRFloatingPointModeOverrideDecoration` (auto-diff stamps `Fast` on generated derivatives).
//
// This exercises (1) the global-mode gate, via SPIR-V opcode survival under precise/fast/default,
// and (2) the per-function override and the absence of cross-function mode leakage, via HLSL
// emission under global precise where one function carries the auto-diff `Fast` override and a
// sibling does not.

//TEST:SIMPLE(filecheck=PRECISE): -target spirv-asm -entry computeMain -stage compute -fp-mode precise
//TEST:SIMPLE(filecheck=FAST): -target spirv-asm -entry computeMain -stage compute -fp-mode fast
//TEST:SIMPLE(filecheck=DEFAULT): -target spirv-asm -entry computeMain -stage compute -fp-mode default
//TEST:SIMPLE(filecheck=LEAK): -target hlsl -stage compute -entry leakMain -line-directive-mode none -fp-mode precise

StructuredBuffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;
StructuredBuffer<int> inputBufferI;
RWStructuredBuffer<int> outputBufferI;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Operands come from a buffer so nothing is constant-folded upstream; each identity is observed
    // by the presence (survived) or absence (folded) of its float opcode in the SPIR-V.
    float x = inputBuffer[tid.x];
    outputBuffer[0] = 0.0f + x;
    outputBuffer[1] = x + 0.0f;
    outputBuffer[2] = x - 0.0f;
    outputBuffer[3] = x - x;
    outputBuffer[4] = x * 0.0f;

    // Integer control: the gate is unconditionally true for integral types, so this folds in every
    // mode (asserted by the OpISub absence checks below), guarding against a regression that
    // over-broadens the gate to suppress integer folds.
    int n = inputBufferI[tid.x];
    outputBufferI[0] = n - n;
}

// Precise mode: no float identity folds, so both Adds, both Subs and the Mul all survive. The exact
// counts are the positive control that makes the FAST absence checks non-vacuous, and prove each
// distinct Add/Sub branch survives rather than one masking another. The integer `n - n` always
// folds, so no OpISub survives in any mode.
//PRECISE-COUNT-2: OpFAdd
//PRECISE-COUNT-2: OpFSub
//PRECISE-COUNT-1: OpFMul
//PRECISE-NOT: OpISub

// Default mode is conservative like precise (the gate is `== Fast`), so all the float ops survive.
//DEFAULT-COUNT-2: OpFAdd
//DEFAULT-COUNT-2: OpFSub
//DEFAULT-COUNT-1: OpFMul
//DEFAULT-NOT: OpISub

// Fast mode: every float identity folds away, so none of these opcodes remain anywhere.
//FAST-NOT: OpFAdd
//FAST-NOT: OpFSub
//FAST-NOT: OpFMul
//FAST-NOT: OpISub

// `passthroughBefore` is undecorated, so under global precise it keeps the precise mode and its
// `x * 0` must survive -- a function-level override on a sibling must not change its mode.
//LEAK: passthroughBefore{{[_0-9]*}}(
//LEAK: {{[a-zA-Z_0-9]+}} * 0.0f
[PreferRecompute]
float passthroughBefore(float x)
{
    return x * 0.0f;
}

[Differentiable]
[PreferRecompute]
float foldingDerivative(float x)
{
    return x * 0.0f;
}

// The generated derivative carries the auto-diff `Fast` override, so its `x * 0` folds even under
// global precise -- confirming the override is honored, not ignored. The LEAK-NOT scopes to the
// region between this function's header and the next matched header, so it asserts no multiply
// survives inside the derivative itself.
//LEAK: s_fwd_foldingDerivative{{[_0-9]*}}(
//LEAK-NOT: {{[a-zA-Z_0-9]+}} * 0.0f

// `passthroughAfter` is undecorated and emitted *after* the override-producing derivative, so it
// covers the forward-leak direction: the derivative's `Fast` must not carry over to a function the
// walk reaches later. Its `x * 0` must survive.
//LEAK: passthroughAfter{{[_0-9]*}}(
//LEAK: {{[a-zA-Z_0-9]+}} * 0.0f
[PreferRecompute]
float passthroughAfter(float x)
{
    return x * 0.0f;
}

[numthreads(1, 1, 1)]
void leakMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = passthroughBefore(inputBuffer[tid.x]);
    var dp = diffPair(inputBuffer[tid.x], 1.0f);
    outputBuffer[1] = fwd_diff(foldingDerivative)(dp).d;
    outputBuffer[2] = passthroughAfter(inputBuffer[tid.x]);
}
