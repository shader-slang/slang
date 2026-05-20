//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#eliminatephis-with-default-options
//META: doc_section_digest=df36b14c0d0c1c8ecb065a968a44debf9939033194e920e3e5fa7c762e530478
//META: purpose=Stress: five-deep nested if/else still phi-eliminates to a single named local assigned in each leaf.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: control-flow nesting at five levels. The default
// phi elimination still produces one named local with branch-
// level assignments instead of a select chain.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = 0;
    if (tid.x > 0) {
        if (tid.x > 1) {
            if (tid.x > 2) {
                if (tid.x > 3) {
                    if (tid.x > 4) {
                        x = 5;
                    } else {
                        x = 4;
                    }
                } else {
                    x = 3;
                }
            } else {
                x = 2;
            }
        } else {
            x = 1;
        }
    }
    outBuf[tid.x] = x;
}

// CHECK: int x_{{[0-9]+}};
// CHECK: if(
// CHECK: x_{{[0-9]+}} = int(
// CHECK: else
