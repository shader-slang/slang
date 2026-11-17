//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-fragdepth-and-geometry-primitive-refinements
//META: doc_section_digest=101470099f314769a6fe398920091b45f975c62ef00a261100bcd790353ab103
//META: purpose=A fragment entry point declaring two depth outputs is rejected by the front end with E30705, so the DepthReplacing collapse is unreachable from source.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative: mixing SV_DepthGreaterEqual and SV_DepthLessEqual in one fragment output
// struct is rejected before any SPIR-V legalization runs. CHECK pins E30705.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target spirv-asm -entry main -stage fragment

struct Out
{
    float a : SV_DepthGreaterEqual;
    float b : SV_DepthLessEqual;
};

[shader("fragment")]
Out main()
{
    Out o;
    o.a = 1.0;
    o.b = 1.0;
    return o;
}

// CHECK: E30705
