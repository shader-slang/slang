//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T20:50:17+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#raytracing-payload
//META: doc_section_digest=9082ae54ded6cb49d7f3a159b2f8dd6bcb051518267d30ba67d1ca1dc471a6f4
//META: purpose=On `-target cuda`, `ReportHit` in an intersection shader is lowered through `reportOptiXIntersection` and emitted as `optixReportIntersection(tHit, hitKind, a0..aN)` with the attribute aggregate flattened to its scalar leaves.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The two-float attribute struct must appear as two scalar operands after
// tHit and hitKind, which is what the legalization pass produces.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage intersection -entry isect

struct Attr
{
    float2 uv;
};

[shader("intersection")]
void isect()
{
    Attr a;
    a.uv = float2(0.25, 0.5);
    ReportHit(1.0, 0, a);
}

// CUDA: optixReportIntersection(1.0f, 0U, {{.*}}uv{{.*}}, {{.*}}uv{{.*}})
