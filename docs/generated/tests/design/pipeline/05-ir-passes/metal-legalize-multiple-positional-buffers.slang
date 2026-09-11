//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
//META: purpose=Expansion: metal-legalize attaches a positional `buffer(N)` marker to each resource in a multi-buffer entry point; the kernel qualifier also appears.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Multi-resource axis on metal-legalize: two RWStructuredBuffers
// each receive a positional `buffer(N)` marker on Metal. The
// `kernel` qualifier on the entry point also appears.

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main

uniform RWStructuredBuffer<int> bufA;
uniform RWStructuredBuffer<int> bufB;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bufA[0] = int(tid.x);
    bufB[0] = int(tid.y);
}

// METAL: kernel
// METAL-DAG: buffer(0)
// METAL-DAG: buffer(1)
