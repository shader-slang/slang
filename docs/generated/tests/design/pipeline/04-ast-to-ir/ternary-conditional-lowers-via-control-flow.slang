//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#builtin-operators-bypass-call-lowering
//META: doc_section_digest=e70a29d1d218a8caa31a9b05ae27e6e9e79c2c46f8f65751d8428df1ad6141c2
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
