//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#standard-modules
//META: doc_section_digest=abb9a956a43e4e1ea596ed4360d78f64fbdfe46832376c6ddcc947f7b31c37c1
//META: purpose=import experimental.workgraph resolves the separately compiled workgraph standard module and brings its record types and Barrier flag enums into scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the qualified import name maps onto a standard module
// shipped beside the compiler: `experimental.workgraph` must resolve
// to that module's compiled artefact and supply both a record type
// and the Barrier flag enum. The CHECK asserts the parameter type
// survives to emitted HLSL and that the enumerator folds to its
// declared bit value (NODE_INPUT_MEMORY = 4), which would be
// unavailable if only the name had been stubbed out.

//TEST:SIMPLE(filecheck=HLSL):-experimental-feature -target hlsl -entry computeMain -stage compute

import experimental.workgraph;

struct Record { uint value; }

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(DispatchNodeInputRecord<Record> rec)
{
    outBuf[0] = rec.Get().value + uint(BarrierMemoryTypeFlags.NODE_INPUT_MEMORY);
}

// HLSL: DispatchNodeInputRecord<Record_{{[0-9]+}}>
// HLSL: 4U
