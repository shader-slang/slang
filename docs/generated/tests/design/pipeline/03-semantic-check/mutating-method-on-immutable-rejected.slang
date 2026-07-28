//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6165fbd3f46bb23f31d4481667f01c7f841053292ade0127deceffcbed0876dc
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
// CHECK: E30050
}
