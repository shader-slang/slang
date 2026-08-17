//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=f9fbc5ab50420dd2b267ddb86a5eff193a88f8bd53dbaeb86a9ca9fdcaf05b23
//META: purpose=A generic with one type parameter but four value arguments lowers to a `call specialize(%g, Int)(%a, %a, %a, %a)` — value arguments flow through the inner call operand list, the type list stays at length one.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: generic value-argument-count axis. The `specialize`
// operand split separates type args (after the generic) from value
// args (in the outer call operand list). Four value args appear in
// the second tuple.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

T pick<T>(T x, T y, T z, T w) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pick<int>(a, a, a, a);
}

// CHECK: generic %{{.*}}
// CHECK-LABEL: func %main
// CHECK: call specialize({{.*}}, Int)(%a, %a, %a, %a)
