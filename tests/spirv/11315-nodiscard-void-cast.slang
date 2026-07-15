// Regression test for:
// https://github.com/shader-slang/slang/issues/11315
//
// A `(void)` cast is the idiomatic way to silence the `[NoDiscard]`
// discarded-result error: casting the result to `void` counts as *using*
// it, so `(void)incrementValue(...)` type-checks without the error. That
// cast used to lower to a `kIROp_CastToVoid` IR instruction which no backend
// handles, so before the #11315 fix emit aborted with an internal
// "unimplemented" error. A cast to `void` carries no data, so lowering now
// yields the canonical void value (`IRVoidLit`) and never materializes the op.
//
// Compiling this shader therefore pins both halves: if the `void` cast
// stopped silencing `[NoDiscard]` the (fatal) discarded-result error would
// abort compilation, and if the void-cast lowering regressed emit would
// abort. A successful compile with an entry point proves both.

//TEST:SIMPLE(filecheck=CHECK_SPIRV):-target spirv-asm -entry main -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute

// CHECK_SPIRV: OpEntryPoint
// CHECK_HLSL: void main

RWStructuredBuffer<uint> output;

enum StatusCode
{
    Success = 0,
    Overflow = 1
}

[NoDiscard] StatusCode incrementValue(inout uint val)
{
    uint prevVal = val;

    ++val;

    // overflow detection
    return prevVal < val ? StatusCode.Success : StatusCode.Overflow;
}

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // ignore overflow status code
    (void)incrementValue(output[0]);
}
