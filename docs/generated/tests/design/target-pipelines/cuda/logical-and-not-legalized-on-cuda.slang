//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=4b70b0a669cc9b83124a252d2add0ed8fb3787692f5a88feaade1d3780fda053
//META: purpose=legalizeLogicalAndOr is filtered out for CUDA because CUDA is in none of the pass's four D3D / Khronos / WGPU / Metal arms, so a vector-operand && reaches emit as an infix && between the two constructed vectors.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies one of the passes the doc lists as skipped on CUDA. The
// skip is observable because a vector-operand && is exactly what
// the pass exists to rewrite on the targets it does run for; left
// alone, it survives as a single infix operator. Both operands are
// built from dispatch-thread components so neither side folds away,
// and the result is stored whole to a bool3 buffer so DCE cannot
// reduce it to a scalar. The CHECK pins the infix && sitting
// between the two make_bool3 constructions.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<bool3> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool3 a = bool3(tid.x > 0, tid.y > 0, tid.z > 0);
    bool3 b = bool3(tid.x < 8, tid.y < 8, tid.z < 8);
    outputBuffer[tid.x] = a && b;
}

// CHECK: make_bool3 ({{.*}}) && make_bool3 ({{.*}});
