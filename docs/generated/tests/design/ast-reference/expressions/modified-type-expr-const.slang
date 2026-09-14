//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-packbranchtypeexpr
//META: doc_section_digest=c81f550a9810da4877739b3075a0cd088585204924f21fbdc02d017dbf1948e5
//META: purpose=A ModifiedTypeExpr `const T` carries a modifier prefix on the type; reads of the const-qualified value return the initialized value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a modifier prefix on a type spelling is accepted and the
// declared value still reads back. `const int` is declared and initialised,
// and the CHECK asserts the initialiser's value survived the modified type
// spelling.

//TEST:INTERPRET(filecheck=CHECK):

static const int kK = 42;

void main()
{
    // CHECK: k=42
    printf("k=%d\n", kK);
}
