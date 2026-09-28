//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=51cb62c0b32d5853269f2948b06d5fe885de6c3a839dfdb3e56f3ab895330c38
//META: purpose=An initializer list with more elements than the synthesized constructor accepts is rejected with E30523 naming both counts.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the boundary of the synthesized construction signature: the
// synthesized constructor has exactly one parameter per field, so a third
// initializer for a two-field struct has nowhere to bind. CHECK pins E30523,
// whose message carries the expected and actual counts.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    int a;
    int b;
}

void main()
{
    S s = { 1, 2, 3 };
//CHECK: E30523
}
