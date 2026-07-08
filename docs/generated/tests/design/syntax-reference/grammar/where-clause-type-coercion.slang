//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=f6e12c33ddd19d9369ee802b5f8dd1f7ec0e74a603076a1958b10edb008910ca
//META: purpose=`WhereTerm` accepts the type-coercion constraint `Type '(' Type ')' 'implicit'?` (toType '(' fromType ')').
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the TypeCoercionConstraintDecl surface in both spellings:
// `where float(T)` and `where float(T) implicit`. Each generic converts
// its argument to float under the constraint; the summed result proves
// both terms parsed and admit the int-to-float coercion.

//TEST:INTERPRET(filecheck=CHECK):

float widen<T>(T x) where float(T)
{
    return float(x);
}

float widenImplicit<T>(T x) where float(T) implicit
{
    return float(x);
}

void main()
{
    // CHECK: r=42
    printf("r=%d\n", int(widen(40) + widenImplicit(2)));
}
