//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=6c19be2b98b712dfb44e27eac1a62628a55a8795cf5a111496e7292369d1578c
//META: purpose=A GenericDecl lowers to an `IRGeneric` (function-shaped IR inst whose body computes a type-level value); specialization is NOT performed at lowering.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The generic `pickFirst<T>` survives lowering as an `IRGeneric`
// (visible as `generic %{{.*}}` in the dump) whose body returns an
// inner `func %pickFirst`. The call site uses
// `call specialize(%generic, Int)(...)` rather than a fully
// specialized body — the doc's "Specialization itself is not
// performed during lowering" claim.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pickFirst(a, a + 1);
}

// CHECK: generic %{{.*}}
// CHECK: func %pickFirst
// CHECK-LABEL: func %main
// CHECK: call specialize({{.*}})
