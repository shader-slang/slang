//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=3ebd10067ff5f6123adbd965e4471fda38311d82079a24af41d8faab3a3f45cc
//META: purpose=A module-scope `ByteAddressBuffer` declaration surfaces a `global_param` with the bare `ByteAddressBuffer` resource type, with no element-type parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a byte-address buffer parameter carries its type
// directly on the `global_param`, with no element-type parameterization
// in parentheses. Both the read-only and the writable spelling are
// declared so the CHECKs can pin each bare type name separately.

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
