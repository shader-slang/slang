//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T14:59:37Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#source-location-serialization
//META: doc_section_digest=55c014b9f8b8d8577ae45cd742d91a9a5bd7995aef490be8dd213262e93a5aee
//META: purpose=When debug info is requested, source-location serialization expands to a debug-info stream that names the source file via an OpString in SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-file path captured by source-location serialization
// (path + location range + line-start tables; the file's content is
// not serialized) is what debug info names. The symptom on SPIR-V is
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
