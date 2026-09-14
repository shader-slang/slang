//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#standard-modules
//META: doc_section_digest=abb9a956a43e4e1ea596ed4360d78f64fbdfe46832376c6ddcc947f7b31c37c1
//META: purpose=Standard-module declarations are not implicitly in scope: the workgraph module's DispatchNodeInputRecord is an undefined identifier without import experimental.workgraph.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies standard modules are loaded on demand rather than embedded
// like the core module: a name that the workgraph standard module
// declares is unresolved when the import is absent. The CHECK pins
// E30015 on the DispatchNodeInputRecord use, which distinguishes
// "the module was never loaded" from "the module loaded but the name
// is spelled differently".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<uint> outBuf;

struct Record { uint value; }

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    DispatchNodeInputRecord<Record> rec;
/*CHECK:
    ^^^^^^^^^^^^^^^^^^^^^^^ undefined identifier 'DispatchNodeInputRecord'.
*/
    // The error code stays pinned separately so a renumbering fails too.
    //CHECK: E30015
    outBuf[tid.x] = 0;
}
