//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=38bbbd7083f0dd5470308b28e015e307c9b17c55d97e2e766bc45629df65ad7a
//META: purpose=`enum` is a decl keyword (dispatched via direct identifier lookahead, not the syntax table) introducing an enumeration with explicit case values.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `enum` parsing is observable via the C++ emit because the
// interpreter does not implement VM bytecode for all enum operations
// in every configuration. The `enum` declaration must parse for the
// program to compile to C++ at all.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

enum Color { Red = 7, Green = 8, Blue = 9 };

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Color c = Color.Green;
    int v = int(c);
    //CHECK: enum
    printf("c=%d\n", v);
}
