//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An interface `static const` requirement whose type is not `int` or `bool` is rejected with E30306.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states "The type of a `static` `const` member must be either
// `int` or `bool`." Verifies that using `float` fires E30306.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

interface IHasBadStatic
{
    static const float badMember;
//CHECK:               ^^^^^^^^^ invalid static const requirement type
//CHECK:               ^^^^^^^^^ 'static const' requirement can only have int or bool type.
}

void main() {}

// CHECK: E30306
