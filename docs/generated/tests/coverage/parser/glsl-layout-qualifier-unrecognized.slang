//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=0b97346b4f21e449b53cfd070b4f2eee57742cdfe1e7938ff5629e665e2795a0
//META: purpose=Verifies an unrecognized identifier inside a `layout(...)` qualifier list is rejected with E31217.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the GLSL `layout(...)` qualifier parser matches each
// inner identifier against a known set; an unknown bare identifier that is
// neither a known keyword nor an `= value` assignment drives the
// unrecognized-qualifier branch. CHECK pins E31217 and the observed message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

layout(bogusqualifier) uniform float x;
//CHECK: E31217
//CHECK: GLSL layout qualifier is unrecognized or requires assignment

[numthreads(1, 1, 1)]
void main() {}
