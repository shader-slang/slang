//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
//META: purpose=`typealias Name = T;` is a decl keyword binding `Name` to type `T`; the alias resolves to its underlying type for overload resolution.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `typealias MyInt = int;` then call a `kind(int)` overload through
// the alias. The overload that matches confirms `MyInt` resolved to
// `int`.

//TEST:INTERPRET(filecheck=CHECK):

typealias MyInt = int;

int kind(int v) { return 1; }
int kind(uint v) { return 2; }

void main()
{
    MyInt x = 42;
    // CHECK: kind=1
    printf("kind=%d\n", kind(x));
}
