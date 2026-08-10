//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=9fa528801fe0f265a456b36eb8381148d65db73060d29e10a33ff9ce384104ac
//META: purpose=`WhereTerm` accepts the type-coercion constraint `Type '(' Type ')' 'implicit'?` (toType '(' fromType ')').
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the TypeCoercionConstraintDecl surface in both spellings:
// `where float(T)` and `where float(T) implicit`. Each generic is
// specialized with an int argument, which the builtin int-to-float
// coercion satisfies; the summed result proves both terms parsed and
// the constrained specializations were admitted. (The constraint
// gates specialization only: converting `x` to float inside the body
// is rejected by the checker, so the bodies return constants.)

//TEST:INTERPRET(filecheck=CHECK):

float widen<T>(T x) where float(T)
{
    return 40.0;
}

float widenImplicit<T>(T x) where float(T) implicit
{
    return 2.0;
}

void main()
{
    // CHECK: r=42
    printf("r=%d\n", int(widen(40) + widenImplicit(2)));
}
