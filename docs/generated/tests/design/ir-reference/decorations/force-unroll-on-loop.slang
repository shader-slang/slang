//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=647327ee76a1bc33d8bcf7d2abc19a2f16ee4e63d10bad4d1d5d48c120029d9d
//META: purpose=Boundary - the [ForceUnroll] attribute is loop-statement-only and produces a ForceUnroll decoration on the loop with an integer operand recording the requested unroll count (0 when omitted).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive side of the placement rule its negative sibling
// pins, and the observed operand shape. The decoration is listed with no
// operands, but the dump prints one integer, so the CHECK matches
// `[ForceUnroll(N : Int)]` with the count wildcarded; the mismatch with the
// documented arity is recorded as a doc gap rather than hidden.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; i++) s += int(tid.x) * i;
    buf[0] = s;
}

// CHECK: [ForceUnroll({{[0-9]+}} : Int)]
