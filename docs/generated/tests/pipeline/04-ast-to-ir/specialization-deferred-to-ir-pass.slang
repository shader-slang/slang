//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=6c19be2b98b712dfb44e27eac1a62628a55a8795cf5a111496e7292369d1578c
//META: purpose=Lowering does not specialize generics; a generic call appears as `call specialize(%generic, <Type>)(...)`, not as an inlined body.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "Specialization itself is not performed during
// lowering; it is handled by IR passes." Observe a generic function
// in the IR dump (still an `IRGeneric`) and a `call specialize(%g,
// Int)(...)` at the call site. The generic body has not been
// substituted in place.

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
// CHECK-LABEL: func %main
// CHECK: call specialize({{.*}}, Int){{.*}}
