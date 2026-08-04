//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=71044a2d3419fa798e104d5b77b4d7a78d1652bd8571bd4aa028984e8e95b794
//META: purpose=Expansion: cuda-immutable-load wraps reads of a struct-field uniform — `Settings.factor` is read via `__ldg(&...)` on CUDA. Struct-field axis alongside the scalar uniform case.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Uniform read shape variation on cuda-immutable-load. The uniform
// is a struct field, not a scalar. CUDA still wraps the read in
// `__ldg(&...)`.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

struct Settings { int factor; int bias; }

uniform Settings cfg;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = cfg.factor * int(tid.x) + cfg.bias;
}

// CUDA: __ldg
