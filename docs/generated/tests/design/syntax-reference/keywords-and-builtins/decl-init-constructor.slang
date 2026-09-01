//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
//META: purpose=`__init` is the decl keyword for a constructor; it is invoked when the type name is used as a call expression.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declare a struct with an `__init(int)` constructor and call it via
// the type-name-as-callable form `V(7)`. The doc notes that `__init`
// is the underscore-prefixed compiler-internal spelling of `init`.

//TEST:INTERPRET(filecheck=CHECK):

struct V
{
    int x;
    __init(int v) { x = v; }
}

void main()
{
    V v = V(7);
    // CHECK: x=7
    printf("x=%d\n", v.x);
}
