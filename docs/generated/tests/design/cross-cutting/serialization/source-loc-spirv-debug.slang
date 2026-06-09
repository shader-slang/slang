//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:14:10Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#source-location-serialization
//META: doc_section_digest=a7785d3d0d63d41c707dce8b6259552b48fc0294b3f0028ba5ef1269c5af67e6
//META: purpose=When debug info is requested, source-location serialization expands to a debug-info stream that names the source file via an OpString in SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc notes that preserving readable diagnostics across
// deserialization needs the SourceFile records (path, content,
// expansion stack) to round-trip. The runtime symptom on SPIR-V is
// an `OpString "<path>"` referencing the source file plus the
// `OpSource Slang` header; we observe both with `-g2` enabled.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -g2 -skip-spirv-validation

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 99;
}

// CHECK: OpString "{{.*source-loc-spirv-debug\.slang}}"
// CHECK: OpSource Slang
