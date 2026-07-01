//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=6c19be2b98b712dfb44e27eac1a62628a55a8795cf5a111496e7292369d1578c
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
