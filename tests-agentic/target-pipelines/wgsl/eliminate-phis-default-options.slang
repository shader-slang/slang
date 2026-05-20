//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/wgsl.md#eliminatephis-with-default-options
//META: doc_section_digest=4ed51270c0dcaa3abac402e52c0eb5952d3358f00eeb4c73bf7b76cd95a1d0d6
//META: purpose=eliminatePhis with default options replaces phi nodes with an explicit var and per-branch assignments
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL accepts default PhiEliminationOptions: the merged value
// becomes a function-local 'var : T' that is assigned in each
// branch of an if/else.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v;
    if (tid.x > 0u)
    {
        v = 10;
    }
    else
    {
        v = 20;
    }
    outBuf[tid.x] = v;
}

// CHECK: var
// CHECK: if
// CHECK: else
