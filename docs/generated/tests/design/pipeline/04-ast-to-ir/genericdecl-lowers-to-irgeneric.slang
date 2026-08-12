//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=d11d731c1f470905e85d4dccd140c912ec50e6d88afbda6ff67159fa80cd78b2
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
