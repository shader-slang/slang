//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=6c19be2b98b712dfb44e27eac1a62628a55a8795cf5a111496e7292369d1578c
//META: purpose=A single generic function called with three distinct type arguments lowers to a single `IRGeneric` and three independent `call specialize(%g, T_k)` sites — each specialization is deferred and emitted at the use site, not at the definition.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: generic-type-argument distinctness axis. The doc says
// specialization is deferred to a later IR pass; lowering must keep
// the body as a single IRGeneric regardless of how many distinct
// specializations the call sites request. Verifies three distinct
// `call specialize(...)` sites (Int, Float, UInt) on a single
// generic.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = pickFirst<int>(a, a);
    float f = pickFirst<float>(1.0f, 2.0f);
    uint u = pickFirst<uint>(1u, 2u);
    buf[0] = s + int(f) + int(u);
}

// CHECK-LABEL: func %main
// CHECK: call specialize({{.*}}, Int)
// CHECK: call specialize({{.*}}, Float)
// CHECK: call specialize({{.*}}, UInt)
