//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=becaee0c42e1c1b7be4278315b96c53245e3eeb522315d56a2556cf99cbbc250
//META: purpose=A generic function declaring two distinct type parameters lowers without specialization; the call site emits `call specialize(%g, Int, Float)(...)` carrying both type arguments in source order.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: generic-type-arg axis at "more than one". Two type
// parameters (`T, U`) survive lowering as the `specialize(%generic,
// Int, Float)` operand triple; the body is still inside an
// `IRGeneric` because specialization is deferred to an IR pass.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

T pickF<T, U>(T x, U y) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pickF<int, float>(a, 1.0f);
}

// CHECK: generic %{{.*}}
// CHECK-LABEL: func %main
// CHECK: call specialize({{.*}}, Int, Float)
