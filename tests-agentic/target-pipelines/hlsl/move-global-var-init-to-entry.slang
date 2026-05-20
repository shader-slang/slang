//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination
//META: doc_section_digest=e8f525c4f3b39107c481bf03414c7c2ad6a1c96a430e84aae54c4f8e6a580692
//META: purpose=A `static` module-scope variable retains its `static T name = initial;` declaration in HLSL emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc Phase C, HLSL/GLSL/WGSL hit
// `moveGlobalVarInitializationToEntryPoints`. For a simple
// compile-time-constant initializer, the HLSL emit keeps the
// declaration with an inline initializer (the pass is a no-op
// for this shape because the constant literal is already legal
// at module scope).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

static int g_counter = 42;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = g_counter;
}

// CHECK: static int g_counter_{{[0-9]+}} = int(42)
// CHECK: void main(
// CHECK: outBuf_{{[0-9]+}}[{{.*}}] = g_counter_{{[0-9]+}};
