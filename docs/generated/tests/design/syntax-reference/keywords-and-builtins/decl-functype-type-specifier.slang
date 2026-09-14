//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
//META: purpose=`functype` is accepted by the type-specifier parser through plain identifier lookahead rather than a syntax-table registration, and it parses the following parenthesised parameter list plus `->` result into a function type.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `functype(int) -> int` denotes a real type distinct from
// the scalar types, using the bundle's overload-probe idiom: one
// `kind` overload takes the function type, the other takes `int`.
// Passing the name of a matching function selects the function-type
// arm (1) and passing a literal selects the `int` arm (2). Both arms
// resolving distinctly requires `functype` to have been parsed as a
// type specifier and its parameter list / `->` result to have been
// folded into a function type; a `functype` that fell through to
// ordinary identifier lookup would leave the first overload
// undeclarable.

//TEST:INTERPRET(filecheck=CHECK):

int addOne(int x) { return x + 1; }

int kind(functype(int) -> int f) { return 1; }
int kind(int v) { return 2; }

void main()
{
    // CHECK: fn=1 scalar=2
    printf("fn=%d scalar=%d\n", kind(addOne), kind(3));
}
