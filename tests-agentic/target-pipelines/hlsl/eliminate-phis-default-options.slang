//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#eliminatephis-with-default-options
//META: doc_section_digest=df36b14c0d0c1c8ecb065a968a44debf9939033194e920e3e5fa7c762e530478
//META: purpose=eliminatePhis runs with default options on HLSL: an if/else-merged value becomes a named local temporary assigned in each branch.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// DXC accepts and re-SSAs explicit temporaries, so Slang's
// HLSL emit uses the default phi-elimination strategy: lift
// the phi to a named local, assign it inside each branch.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x;
    if (tid.x > 0)
        x = 10;
    else
        x = 20;
    outBuf[tid.x] = x;
}

// CHECK: int x_{{[0-9]+}};
// CHECK: if(
// CHECK: x_{{[0-9]+}} = int(10);
// CHECK: else
// CHECK: x_{{[0-9]+}} = int(20);
