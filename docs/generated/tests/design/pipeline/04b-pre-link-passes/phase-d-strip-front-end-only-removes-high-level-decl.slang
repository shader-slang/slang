//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#stripfrontendonlyinstructions
//META: doc_section_digest=6d9c3e171509715bc06e848e926d607a7687b1fec0c44f072b66721b314e4dc1
//META: purpose=Phase D's stripFrontEndOnlyInstructions (D10) always removes the front-end-only IRHighLevelDeclDecoration before the module is handed on for code generation.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unconditional half of the stripping list. The mandatory
// passes use the high-level-decl decoration to point diagnostics back at
// AST-level code, but it must not reach code generation. The module
// declares an ordinary struct and entry point, and the CHECK-NOT asserts
// the decoration's printed form is absent from the post-Phase-D snapshot.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct Helper { int value; };

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Helper h = { 42 };
    buf[0] = h.value;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-NOT: HighLevelDecl
