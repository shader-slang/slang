//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:11+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-initializer.md#initializer-expressions
//META: doc_section_digest=4baa222d5b95c67d63377172d8a9ca06fa2e19f939661ef5099353e8c61b3470
//META: purpose=A call whose base expression is a type (not a value) is an initializer expression that creates an instance of that type using the given arguments.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's canonical example is float2(1.0f, 2.0f): a type-as-callee call that
// produces a float2 initialized from two float arguments.
// Use function parameters to defeat the constant folder.

//TEST:INTERPRET(filecheck=CHECK):

float passX(float x) { return x; }
float passY(float y) { return y; }

void main()
{
    float2 v = float2(passX(1.0f), passY(2.0f));
    //CHECK: 1.000000 2.000000
    printf("%f %f\n", v.x, v.y);
}
