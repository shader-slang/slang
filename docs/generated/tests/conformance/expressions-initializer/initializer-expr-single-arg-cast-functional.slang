//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:11+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-initializer.md#initializer-expressions
//META: doc_section_digest=4baa222d5b95c67d63377172d8a9ca06fa2e19f939661ef5099353e8c61b3470
//META: purpose=A single-argument initializer expression int(x) is treated as a cast expression and produces the same value as (int)x.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// C3: "An initializer expression with only a single argument is treated as a
// cast expression." The doc's example: int(1.0f) and (int) 1.0f are equivalent.
// Use function parameters to defeat the constant folder.

//TEST:INTERPRET(filecheck=CHECK):

float passFloat(float f) { return f; }

void main()
{
    float f1 = passFloat(1.7f);
    float f2 = passFloat(1.7f);
    int a = int(f1);       // initializer-expression form (single arg → cast)
    int b = (int)f2;       // C-style cast form
    // Both must truncate to 1
    //CHECK: 1 1
    printf("%d %d\n", a, b);
}
