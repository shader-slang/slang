//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=A StringType value keeps its text through lowering, so a `String` literal reaches the emitted target source verbatim.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the String type carries real text rather than an opaque
// handle. A module-scope String literal is used for its length so it
// cannot be dead-stripped, and the host-cpp backend is the observation
// point because slangi's printf has no %s. CHECK matches the literal
// text in the emitted C++ source.

//TEST:SIMPLE(filecheck=CHECK):-target host-cpp -entry computeMain -stage compute

static const String message = "slang-marker-text";

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: slang-marker-text
    outBuf[0] = message.getLength();
}
