//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#shape-of-the-emitted-file
//META: doc_section_digest=cd40f48a68f8c4cc01781946c78837919b2702a1a94124525d0a69aadc68ec8c
//META: purpose=`Buffer` is one of the base shapes the HLSL texture-type name is composed from, so `Buffer<T>` emits with a `t` register and the same shape under the `RW` access prefix emits `RWBuffer<T>` with a `u` register.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `Buffer` row of the base-shape enumeration in the
// texture-type composition — the only one of the five shapes with no
// `Texture` in its name, and therefore the one most likely to be
// special-cased away. Declaring both the plain and `RW` spellings in
// one module also shows the prefix and the register class agreeing:
// the read-only shape lands in `t`, the read-write shape in `u`.
// The read-only CHECK is line-anchored because `Buffer<` matches
// inside `RWBuffer<` as a substring.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Buffer<float4> buf;
RWBuffer<float4> rwbuf;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    rwbuf[tid.x] = buf[tid.x];
    outBuf[tid.x] = buf[tid.x];
}

// CHECK-DAG: {{^}}Buffer<float4 > buf_{{[0-9]+}} : register(t{{[0-9]+}});
// CHECK-DAG: {{^}}RWBuffer<float4 > rwbuf_{{[0-9]+}} : register(u{{[0-9]+}});
