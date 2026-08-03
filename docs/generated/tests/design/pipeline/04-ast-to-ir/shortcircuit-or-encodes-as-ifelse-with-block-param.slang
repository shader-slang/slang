//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#builtin-operators-bypass-call-lowering
//META: doc_section_digest=e70a29d1d218a8caa31a9b05ae27e6e9e79c2c46f8f65751d8428df1ad6141c2
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
