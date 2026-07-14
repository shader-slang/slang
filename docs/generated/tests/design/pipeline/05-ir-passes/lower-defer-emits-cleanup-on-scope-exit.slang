//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=87cc303db4e3bb3825cd2dfa57f040c4ffad9180d465ffc6172dea9a41fbd518
//META: purpose=The lower-defer pass routes a `defer` statement's body to run at scope exit; the deferred write appears in the emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Lower defer" row of the type-and-value legalization table
// (`slang-ir-lower-defer.cpp`) lowers `defer { ... }` to a cleanup
// block. Both the eager and deferred writes appear in the emit, and
// the deferred write is sequenced after the body.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    defer { buf[1] = a + 100; }
    buf[0] = a + 1;
}

// HLSL: void main
// HLSL: int(1)
// HLSL: int(100)
