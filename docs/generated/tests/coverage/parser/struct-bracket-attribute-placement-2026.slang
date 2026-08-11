//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=0b97346b4f21e449b53cfd070b4f2eee57742cdfe1e7938ff5629e665e2795a0
//META: purpose=Verifies a bracketed attribute list placed after the `struct` keyword is rejected with E31205 under the 2026 language version.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: ParseStruct sees a `[` after `struct` and, under
// language version 2026, the post-keyword bracketed-attribute placement is
// an error (it was deprecated in 2025 and removed in 2026). The `-std 2026`
// flag selects that version. CHECK pins E31205 and the observed message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026 -target hlsl -entry main -stage compute

struct [unroll] S { int x; }
//CHECK: E31205
//CHECK: invalid placement of bracketed attributes list.

[numthreads(1, 1, 1)]
void main() {}
