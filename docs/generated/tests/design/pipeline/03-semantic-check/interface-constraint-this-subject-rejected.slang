//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=57d9f151be60063d08c61244f7768487d8eaebc83717628353b6150f280dd55a
//META: purpose=A bare-`This` subject on an interface `__constraint` is rejected during checking because it would express inheritance rather than a checked predicate.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out the checker applies to `__constraint` subjects.
// Writing `This` as the subject would silently mean "this interface inherits
// IBar", which the language spells with an inheritance clause instead, so the
// constraint form has to be refused. CHECK pins E30427 on the `This` subject.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IBar
{
    int bar();
}

interface IThis
{
    __constraint This : IBar;
//CHECK: E30427
}

void main()
{
}
