//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=7c4335ef62cc708fc063656220fb481b4e3fdbe6592973946fe16a93c217487e
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
