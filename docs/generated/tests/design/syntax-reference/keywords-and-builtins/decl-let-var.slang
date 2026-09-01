//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
//META: purpose=`let` introduces an immutable binding and `var` introduces a mutable binding; both are decl keywords.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declare both a `let` and a `var` of the same value, mutate the
// `var` once, and observe the result. The mutability difference is
// covered by a separate diagnostic test.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    let a = 7;
    var b = 5;
    b = b + 1;
    // CHECK: a=7 b=6
    printf("a=%d b=%d\n", a, b);
}
