//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=A StringType String-typed literal preserves its text through to emitted target code.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the StringType claim that a `String` literal carries its
// text through lowering: a module-scope String value is emitted into
// the CPU C++ target. slangi printf has no %s, so we FileCheck the
// emitted C++ source for the literal text. CHECK matches the string
// content in the emitted code.

//TEST:SIMPLE(filecheck=CHECK):-target host-cpp -entry computeMain -stage compute

static const String message = "slang-marker-text";

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: slang-marker-text
    outBuf[0] = message.getLength();
}
