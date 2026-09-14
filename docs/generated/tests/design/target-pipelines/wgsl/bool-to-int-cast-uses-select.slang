//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#wgslsourceemitter-policy-overrides-and-bool-conditioned-values
//META: doc_section_digest=d47eda324078a32dbc73f59419bad83ce4ed4cd8713fc5e48372759b3592f93c
//META: purpose=A scalar bool-to-int cast emits select(T(0), T(1), cond) because WGSL has no implicit bool-to-integer conversion
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the kIROp_IntCast arm of the WGSL emitter's tryEmitInstExprImpl:
// WGSL neither converts `bool` to an integer implicitly nor accepts an
// `i32(cond)` constructor call, so the emitter must spell the conversion as
// a `select`. The condition is derived from the dispatch id so the cast
// cannot be constant-folded away; CHECK pins the select(i32(0), i32(1), ...)
// shape, which maps false to 0 and true to 1.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool c = tid.x > 0u;
    outBuf[tid.x] = int(c);
}

// CHECK: select(i32(0), i32(1),
