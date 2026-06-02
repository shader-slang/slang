//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:11+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-initializer.md#initializer-list-expression
//META: doc_section_digest=68be737ab4b8adb6374f77514e63e2dd70ca264c5b4d8f7ff4cffa68f75e41dc
//META: purpose=An initializer list expression used as the initial-value of a parameter declaration (default argument) initializes the parameter when called without that argument.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// C5 (positive path): the doc says init lists are permitted as initial-value
// expressions in parameter declarations (not just variable declarations).
// Verify a struct parameter with a default {} initializer produces the
// expected field values when the function is called without that argument.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec2 { float x; float y; };

void process(Vec2 v = { 0.5f, 1.5f })
{
    printf("%f %f\n", v.x, v.y);
}

void main()
{
    // Called without argument — default init list should apply.
    //CHECK: 0.500000 1.500000
    process();
}
