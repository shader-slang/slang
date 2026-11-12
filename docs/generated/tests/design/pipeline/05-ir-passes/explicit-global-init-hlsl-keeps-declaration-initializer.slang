//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T20:50:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=0fba7fe66a94de18a3a43af71b0a2a503d43e1e15dfc035e524c23f3c2bdbd9f
//META: purpose=On HLSL, moveGlobalVarInitializationToEntryPoints leaves a scalar static global with its declaration initializer instead of moving it into the entry point.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Explicit global init row says HLSL keeps declaration initializers
// except for arrays, cooperative vectors and selected resource globals.
// The same source on GLSL/WGSL (explicit-global-init-moves-initializer-
// into-entry-point.slang) loses the initializer; here the HLSL emit keeps
// `= int(7)` on the declaration.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

RWStructuredBuffer<int> outBuf;

static int gCounter = 7;

[numthreads(1, 1, 1)]
void main()
{
    gCounter = gCounter + 1;
    outBuf[0] = gCounter;
}

// HLSL: static int gCounter_{{[0-9]+}} = int(7);
