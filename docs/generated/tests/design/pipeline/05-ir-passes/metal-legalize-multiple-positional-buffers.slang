//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=71044a2d3419fa798e104d5b77b4d7a78d1652bd8571bd4aa028984e8e95b794
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
