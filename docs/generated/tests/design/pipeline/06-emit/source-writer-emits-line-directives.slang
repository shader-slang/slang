//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#source-writer-abstraction
//META: doc_section_digest=ce4762aa739250058ecaca2d4fb4a4efacf40deea256f925596042f766e09876
//META: purpose=`SourceWriter::advanceToSourceLocation` emits `#line` directives naming the original Slang file so a downstream compiler reports its errors at the user's source position; the C-style directive form appears on HLSL, Metal, CUDA, and C++.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that emitted text is annotated back to the Slang source. The
// kernel body is spread over several statements so the writer has to advance
// the location more than once. Each prefix asserts a C-style `#line <n>
// "<file>"` directive naming this test file; the file name is matched with a
// wildcard because the emitted path is absolute. Without these directives a
// DXC/nvrtc error would point into generated text the user never wrote.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    int b = int(tid.y);

    outBuf[0] = a * b;

    outBuf[1] = a - b;
}

// HLSL: {{^}}#line {{[0-9]+}} "{{.*}}source-writer-emits-line-directives.slang"
// METAL: {{^}}#line {{[0-9]+}} "{{.*}}source-writer-emits-line-directives.slang"
// CUDA: {{^}}#line {{[0-9]+}} "{{.*}}source-writer-emits-line-directives.slang"
// CPP: {{^}}#line {{[0-9]+}} "{{.*}}source-writer-emits-line-directives.slang"
