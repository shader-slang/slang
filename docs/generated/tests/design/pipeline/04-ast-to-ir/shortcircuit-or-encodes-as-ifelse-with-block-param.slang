//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#builtin-operators-bypass-call-lowering
//META: doc_section_digest=32b31a97f5e15fb2fec9486a881ce13cddbd02cb72566084734c0db4939ddf9b
//META: purpose=A short-circuit logical-OR of `(a > b)` and `(c < d)` is excluded from the builtin fast path and lowers through its dedicated control-flow path — an `ifElse(...)` whose join block receives the Bool via a block-parameter branch argument, with the RHS evaluated only on the false edge.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out: `||` never reaches lowerBuiltinOperatorExpr;
// it lowers as a control-flow split so the RHS is skipped when the LHS
// is already true. The true edge branches to the join with the literal
// `true`; the false edge computes the RHS and branches with that value;
// no explicit `phi` opcode appears.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform int c;
uniform int d;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if ((a > b) || (c < d))
        buf[0] = 1;
    else
        buf[0] = 0;
}

// CHECK-LABEL: func %main
// CHECK: ifElse(
// CHECK: unconditionalBranch(%{{.*}}, true)
// CHECK: unconditionalBranch(%{{.*}}, %{{.*}})
// CHECK-NOT: = phi(
