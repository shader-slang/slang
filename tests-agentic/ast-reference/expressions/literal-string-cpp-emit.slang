//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=A StringLiteralExpr carries the parsed string value; the same string text appears in emitted C++ output.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// slangi printf does not support %s, so for StringLiteralExpr value
// observation we compile to C++ and FileCheck the emitted source.
// The literal "agentic-string-marker" must appear verbatim.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    printf("agentic-string-marker");
    // CHECK: agentic-string-marker
}
