//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=a90b198ea2328d6f79a99f07405f80975c42e7ee4f6f90cb3a7ef552183ec66f
//META: purpose=Verifies a type-only declaration with no declarator (e.g. `int;`) is rejected because it declares nothing.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: a declaration that names a type but no declarator and is
// not a tag-defining struct/enum produces no declared entity; the parser
// drives the declares-nothing branch. CHECK pins the code and message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

int;
/*CHECK: E39999
^^^ declaration does not declare anything
*/

[numthreads(1, 1, 1)]
void main() {}
