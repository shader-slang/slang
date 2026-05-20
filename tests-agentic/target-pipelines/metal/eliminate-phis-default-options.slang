//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#eliminatephis-with-default-options
//META: doc_section_digest=2b0ff9304692be73626bf7bd3c50f028f4c346a134dc0949d2343d82c090c772
//META: purpose=`eliminatePhis` runs with default options on Metal: an if/else-merged value becomes a function-local variable assigned in each branch.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Metal's phi elimination uses the default options (contrast
// with SPIR-V's register-allocation strategy). The emitted MSL
// uses an explicit local `int v_0;` assigned inside each branch.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

uniform int u;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v;
    if (u > 0)
    {
        v = int(tid.x);
    }
    else
    {
        v = -int(tid.x);
    }
    outBuf[tid.x] = v;
}

// CHECK: int v_{{[0-9]+}};
// CHECK: if(
// CHECK: v_{{[0-9]+}} = int(
// CHECK: else
// CHECK: v_{{[0-9]+}} =
