//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=41260c8f8bf7f0e8598932b33e0e91618368529d9e8fe3e7b7881fe0dd2a078f
//META: purpose=`GenericParam` accepts the traditional type-first value form `Type IDENT`, so `<int N>` declares a value parameter.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the type-first arm of GenericParam. The doc says an identifier
// followed by `:`, `,`, `>` or `=` is a *type* parameter and anything else is
// parsed as a type-first *value* parameter, so `<int N>` must bind a value.
// Using N as a multiplier and printing 42 proves the value reading was taken.

//TEST:INTERPRET(filecheck=CHECK):

int scaleBy<int N>(int x)
{
    return x * N;
}

void main()
{
    // CHECK: r=42
    printf("r=%d\n", scaleBy<7>(6));
}
