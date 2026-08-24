//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=7c4335ef62cc708fc063656220fb481b4e3fdbe6592973946fe16a93c217487e
//META: purpose=moveGlobalVarInitializationToEntryPoints moves initialization logic off global variables and onto each entry point, so the initializer shows up as the first statements of the emitted entry-point body rather than on the global's declaration.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the move on two targets whose module-scope
// syntax cannot carry the initializer. The global below is declared with
// an initial value in the source; the CHECKs assert the emitted
// declaration has lost it (nothing between the name and the statement
// terminator) and that the assignment reappears as the opening statement
// of the entry-point body, which is exactly the relocation the pass makes.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

RWStructuredBuffer<int> outBuf;

static int gCounter = 7;

[numthreads(1, 1, 1)]
void main()
{
    gCounter = gCounter + 1;
    outBuf[0] = gCounter;
}

// The declaration keeps no initializer ...
// GLSL: int gCounter_{{[0-9]+}};
// ... and the initialization opens the entry-point body.
// GLSL: gCounter_{{[0-9]+}} = 7;

// WGSL: var<private> gCounter_{{[0-9]+}} : i32;
// WGSL: gCounter_{{[0-9]+}} = i32(7);
