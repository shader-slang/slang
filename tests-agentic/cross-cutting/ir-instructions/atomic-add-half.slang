//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=An `Atomic<half>.add(v)` lowers to the documented `atomicAdd` opcode with a `Half` result type — narrow-float probe of the `AtomicOperation` family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc admits `atomicAdd` over the `AtomicOperation` family without
// constraining the element-type axis to integers. A `half` element
// exercises the lower edge of the floating-point width axis and
// confirms that the same `atomicAdd` opcode is reused with a `Half`
// result type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<Atomic<half> > buf;
uniform half v;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0].add(v);
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{.*}}, %v, {{.*}})
