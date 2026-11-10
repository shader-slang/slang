//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-09T10:40:55+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#legalizeoptixreportintersectionsforcuda
//META: doc_section_digest=d7e76c39deb19b733259283d94c088003a7270dc4cfd11cd05f2073ccecd799e
//META: purpose=Negative - legalizeOptiXReportIntersectionsForCUDA flattens the ReportHit attribute aggregate into scalar leaves and diagnoses E55218 when more than kMaxOptiXHitAttributeRegisters (8) leaves result.
//META: intent=negative
//META: pipeline_stage=ir-passes
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Nine float fields flatten to nine leaves, one past the documented
// eight-register OptiX hit-attribute limit.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target cuda -entry isect -stage intersection

struct Attr { float a0; float a1; float a2; float a3; float a4; float a5; float a6; float a7; float a8; };

[shader("intersection")]
void isect()
{
    Attr a = {};
    ReportHit(1.0, 0, a);
}

// CHECK: E55218
