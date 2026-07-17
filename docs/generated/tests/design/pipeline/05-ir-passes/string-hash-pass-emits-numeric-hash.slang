//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=99ce8775511c8e573c9ac6b4b3d96b8c25c6abb194e2f731be7d249c1f4d2bc8
//META: purpose=The string-hash pass replaces `getStringHash("...")` with a stable integer hash at IR level.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "String hash" row of the layout-and-binding table
// (`slang-ir-string-hash.cpp`) replaces the `getStringHash(...)`
// intrinsic with the precomputed hash value. The emit text contains
// a literal integer constant rather than a `getStringHash` call.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = getStringHash("hello");
}

// HLSL: void main
// HLSL-NOT: getStringHash
