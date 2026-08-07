//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#logical
//META: doc_section_digest=a02f1013f3939bcd733b43d3c9c2644ac90b70974d479007af65779a682e1e59
//META: purpose=The short-circuiting `&&` operator is NOT the lowering of `logicalAnd`; it lowers to an `ifElse` plus a join-block `param` so the right-hand side is evaluated only on the taken edge.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out that separates the short-circuit operators from
// the `logicalAnd` / `logicalOr` opcodes. The right-hand operand here is
// a division that must not execute when the left operand is false, so a
// branch-free opcode would be wrong. CHECK asserts the emitted shape is
// an `ifElse` terminator whose join block takes a `param` (the merged
// bool), and that the right-hand `cmpGT` sits in its own block rather
// than beside the left-hand one — that block split is exactly what a
// `logicalAnd` opcode would not produce.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (a > 0 && b > 0) ? 1 : 0;
}

// IR-LABEL: func %main
// IR: cmpGT(%a, 0 : Int)
// IR: ifElse(
// IR: cmpGT(%b, 0 : Int)
// IR: param %{{[A-Za-z0-9_]+}}	: Bool
