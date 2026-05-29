//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#standard-modules
//META: doc_section_digest=e736541fe370e6b141fb2ebd19d2f4b376079f81f8b0c4c945979076e75beca5
//META: purpose=Negative: the diff meta-module is embedded (implicit) — it is not importable by a `import diff;` filename lookup.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative: the diff meta-module ships embedded in libslang
// alongside core, not as an installed `.slang-module`. An explicit
// `import diff;` therefore triggers the file-search path and fails
// with "cannot open file 'diff.slang'". This is the inverse of the
// implicit-import behaviour exercised by the functional diff test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

import diff;
/*CHECK:
       ^^^^ cannot open file 'diff.slang'
*/
// CHECK: E1

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(tid.x);
}
