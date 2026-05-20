//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=21e167102f1daeca19e02f308c112de6dba84ede8a917f058d59ae80138afd50
//META: purpose=A module-scope `ByteAddressBuffer` declaration surfaces a `global_param` with the bare `ByteAddressBuffer` resource type, with no element-type parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists byte-address-buffer types alongside structured
// buffers; the `global_param` carries the type directly. This test
// observes the bare-type form (no `(...)` parameterization) and
// complements `structured-buffer-types-globalparam.slang`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform ByteAddressBuffer babuf;
uniform RWByteAddressBuffer rwbabuf;
uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    uint v = babuf.Load(idx * 4);
    rwbabuf.Store(idx * 4, v + 1);
    rwbuf[idx] = v;
}

// IR: %babuf{{.*}}: ByteAddressBuffer{{.*}}= global_param
// IR: %rwbabuf{{.*}}: RWByteAddressBuffer{{.*}}= global_param
