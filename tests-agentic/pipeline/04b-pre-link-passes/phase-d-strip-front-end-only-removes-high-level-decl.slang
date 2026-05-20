//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/pipeline/04b-pre-link-passes.md#stripfrontendonlyinstructions
//META: doc_section_digest=14f6a7b07242b6298cbc13f7d2d10a7c3a385bb0f04856ad82f45cbb7629d49f
//META: purpose=Phase D's stripFrontEndOnlyInstructions (D10) removes IRHighLevelDeclDecoration before the LOWER-TO-IR dump.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The notable-passes "stripFrontEndOnlyInstructions" section
// documents that `IRHighLevelDeclDecoration` is among the
// front-end-only decorations removed before backend processing.
// Verify by writing a module with normal user decls and asserting
// the decoration's printed form does not appear in the IR dump.

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
