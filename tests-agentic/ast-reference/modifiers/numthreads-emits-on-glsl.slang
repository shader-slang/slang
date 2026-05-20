//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes
//META: doc_section_digest=5332741d22bd1f66cc006924c76c4414130d880c08abe1915fb243255799a71b
//META: purpose=`[numthreads(x,y,z)]` lowers to GLSL `layout(local_size_x=..., local_size_y=..., local_size_z=...)`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `NumThreadsAttribute` lowers to a GLSL `layout(local_size_*)`
// declaration. The emitted GLSL must mention the chosen dimensions.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(16, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

//CHECK: local_size_x = 16
