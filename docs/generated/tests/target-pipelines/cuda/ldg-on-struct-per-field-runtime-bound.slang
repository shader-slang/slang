//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=f99045fe923d667d28ba075f821f8ecea38aaffb44e4d1ea1d4c3d94fce11929
//META: purpose=Verifies the per-field __ldg(&ptr->field) expansion fires for a three-field struct read at a runtime-bounded index.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: struct-element immutable-buffer load with > 2
// fields and a runtime index. The pass should still issue one
// `__ldg(&ptr->field)` read per field.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

struct Triple { float a; float b; float c; };
StructuredBuffer<Triple> roBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Triple t = roBuf[tid.x];
    outBuf[tid.x] = t.a + t.b + t.c;
}

// CHECK: __ldg(&{{.*}}a_{{[0-9]+}}
// CHECK: __ldg(&{{.*}}b_{{[0-9]+}}
// CHECK: __ldg(&{{.*}}c_{{[0-9]+}}
