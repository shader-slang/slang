//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=9163d71b0ed4735c5c28e2877570515476519666c5a6da87256143a8fab12f94
//META: purpose=A [mutating] method cannot be invoked on an immutable receiver; the checker rejects the call with diagnostic 30050.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says modifier validation lives in
// slang-check-modifier.cpp. The [mutating] modifier on a method
// enforces a constraint on the receiver: it must be an lvalue
// (mutable). Verify the diagnostic when called on a const.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    int x;
    [mutating] void bump() { x = x + 1; }
}

void main()
{
    const S s = { 0 };
    s.bump();
//CHECK:  ^ mutating method cannot be called on immutable value
//CHECK:  ^ mutating method 'bump' cannot be called on an immutable value
}
