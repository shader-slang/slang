//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#standard-modules
//META: doc_section_digest=e736541fe370e6b141fb2ebd19d2f4b376079f81f8b0c4c945979076e75beca5
//META: purpose=Negative: `import` of a non-existent module name is rejected at the module-loader stage.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative: the doc describes `import slang.<name>` as the on-demand
// load path for standard modules. Importing a name with no
// corresponding source/precompiled module on the search path must
// produce a "cannot open file" diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

import nonexistent_module_qwerty;
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^ cannot open file 'nonexistent_module_qwerty.slang'
*/
// CHECK: E1

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(tid.x);
}
