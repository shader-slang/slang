//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies an `__intrinsic_asm "...", expr;` statement lowers its value argument as an r-value and substitutes it for the `$0` slot in the emitted target text.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the value-argument arm of `visitIntrinsicAsmStmt`: each comma-separated
// argument whose type is not a `TypeType` is lowered through
// `lowerRValueExpr` and appended to the GenericAsm inst after the format
// string. The body of `addOne` is nothing but the asm statement, so the only
// way `($0) + 1` can appear in the emitted HLSL with the caller's argument
// spliced into `$0` is if that lowered operand was recorded.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

int addOne(int x)
{
    __intrinsic_asm "($0) + 1", x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = addOne(int(tid.x));
}

// The lowered argument is spliced into the `$0` slot, parenthesized as written.
//CHECK: (((int(tid_{{[0-9]+}}.x))) + 1)
