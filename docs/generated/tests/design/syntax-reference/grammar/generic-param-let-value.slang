//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=9f7b958fb06af2133f1ee3e2412be63c482681598203ef131595bab232b76842
//META: purpose=`GenericParam` accepts the value-parameter form `'let' IDENT (':' Type)?` (GenericValueParamDecl).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `'let' IDENT ':' Type` arm of GenericParam, which the doc
// distinguishes from the type-parameter arm. The parameter is used as a
// multiplier in the body, so the printed 42 is only reachable if `N` was
// bound to the *value* 7 rather than to a type named 7.

//TEST:INTERPRET(filecheck=CHECK):

int scaleBy<let N : int>(int x)
{
    return x * N;
}

void main()
{
    // CHECK: r=42
    printf("r=%d\n", scaleBy<7>(6));
}
