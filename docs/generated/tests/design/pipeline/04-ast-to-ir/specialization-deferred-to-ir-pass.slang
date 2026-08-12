//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=d11d731c1f470905e85d4dccd140c912ec50e6d88afbda6ff67159fa80cd78b2
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
