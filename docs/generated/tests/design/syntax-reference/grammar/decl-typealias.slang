//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=b092f350bdbde298b8860c3da710d6672d2e5f235d75620d903bddf8a6158c3b
//META: purpose=`TypeAliasDecl` parses as `'typealias' IDENT '=' Type ';'`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the TypeAliasDecl production: `typealias Name = Type;`. The
// alias is then used as a type in a variable declaration.

//TEST:INTERPRET(filecheck=CHECK):

typealias MyInt = int;

void main()
{
    MyInt x = 42;
    // CHECK: x=42
    printf("x=%d\n", x);
}
