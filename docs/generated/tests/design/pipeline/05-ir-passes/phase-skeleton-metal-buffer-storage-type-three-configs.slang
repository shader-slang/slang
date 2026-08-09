//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#phase-skeleton-of-linkandoptimizeir
//META: doc_section_digest=69eb2bad606734f77bdf61bc32fc8cab739a4c8a87aa3eda20b511e1e47a6cf7
//META: purpose=`lowerBufferElementTypeToStorageType` runs in three configurations for Metal, so a Metal compile produces three of its dump stages.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the phase-skeleton's explicit "(three configurations for
// Metal)" parenthetical. Compiling a buffer-using compute kernel to
// Metal while dumping only that pass and requiring three consecutive
// `### AFTER lowerBufferElementTypeToStorageType:` labels shows the
// orchestrator invoked the pass three separate times; a Metal pipeline
// with fewer configurations would leave a CHECK-LABEL unmatched.

//TEST:SIMPLE(filecheck=CHECK):-target metal -dump-ir-after lowerBufferElementTypeToStorageType -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x) + 1;
}

// CHECK-LABEL: ### AFTER lowerBufferElementTypeToStorageType:
// CHECK-LABEL: ### AFTER lowerBufferElementTypeToStorageType:
// CHECK-LABEL: ### AFTER lowerBufferElementTypeToStorageType:
