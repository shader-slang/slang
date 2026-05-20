//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
//META: purpose=Boundary: a shader with zero module-scope uniforms does not produce a `GlobalParams` aggregate in the HLSL emit — the collect-global-uniforms pass has nothing to pack.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Zero-uniform edge of the collect-global-uniforms pass. Only an
// `RWStructuredBuffer` is referenced; no scalar / vector uniform is
// declared, so no `GlobalParams` aggregate is built or emitted.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// HLSL: void main
// HLSL-NOT: GlobalParams
// HLSL-NOT: globalParams
