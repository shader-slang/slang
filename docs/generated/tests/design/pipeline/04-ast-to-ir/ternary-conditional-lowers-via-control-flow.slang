//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#builtin-operators-bypass-call-lowering
//META: doc_section_digest=32b31a97f5e15fb2fec9486a881ce13cddbd02cb72566084734c0db4939ddf9b
//META: purpose=A scalar `cond ? x : y` is excluded from the builtin fast path and lowers through its dedicated control-flow path: an `ifElse(...)` joining through a block parameter, not a single pure-value inst.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ternary carve-out: `?:` is listed with `&&`/`||` as
// NOT handled by lowerBuiltinOperatorExpr. A scalar-condition ternary
// must therefore lower as control flow — an `ifElse` whose join block
// carries the selected Int as a block `param` — and no `select`
// pure-value inst may stand in for it in the lowered body.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform int c;
uniform int d;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = (a > b) ? c : d;
}

// CHECK-LABEL: func %main
// CHECK-NOT: = select
// CHECK: ifElse(
// CHECK: param
// CHECK-NOT: = select
// CHECK: return_val
