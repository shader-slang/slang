//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#inlining-and-call-graph
//META: doc_section_digest=5916f676576778a4bdb5917c7cf917a7fca962142a97cb9c75be16910e0e08a2
//META: purpose=The dll-export pass marks exported entry-point functions with an `export` decoration visible in the IR dump.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "DLL export" row of the inlining / call-graph category
// (`slang-ir-dll-export.cpp`) attaches an export decoration to
// entry-point functions. The IR dump shows
// `[export("...")]` decorating `main`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// CHECK: [export(
// CHECK: func %main
