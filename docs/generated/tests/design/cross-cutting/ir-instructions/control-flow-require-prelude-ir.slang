//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=0b68b77b6156c67db4c587635b263df5f1c7f5c3d702914334c92fc6b5e88c90
//META: purpose=`__requirePrelude` lowers to the `RequirePrelude` backend-hint opcode carrying the prelude text, which the emitter hoists ahead of the generated code.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `RequirePrelude` member of the "other control-flow /
// backend-hint opcodes" row. The call sits inside a helper that the entry
// point actually calls, so the hint is reachable. The IR CHECK asserts the
// dump prints `RequirePrelude("...")` with the text as an inline operand,
// and the per-target CHECKs assert the same text is hoisted to the top of
// the generated source on each C-family target — the behaviour that makes it
// a "backend hint" rather than an ordinary statement, since the text appears
// before the function it was written inside.

//TEST:SIMPLE(filecheck=IR):-target cpp -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> outbuf;

int helper(int x)
{
    __requirePrelude("#define SLANG_TEST_PRELUDE_MARKER 1\n");
    return x + 1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = helper(a);
}

// IR: RequirePrelude("#define SLANG_TEST_PRELUDE_MARKER 1\n")

// CPP: #define SLANG_TEST_PRELUDE_MARKER 1
// CUDA: #define SLANG_TEST_PRELUDE_MARKER 1
// HLSL: #define SLANG_TEST_PRELUDE_MARKER 1
