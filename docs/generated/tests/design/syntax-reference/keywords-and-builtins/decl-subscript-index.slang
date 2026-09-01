//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
//META: purpose=`__subscript(args) -> T { get { ... } }` is a decl keyword that binds `v[args]` syntax to a user-defined accessor.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Define a `__subscript` returning the i-th element of a 3-field
// struct and verify `v[i]` dispatches into it.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec3
{
    int x;
    int y;
    int z;
    __subscript(int i) -> int { get { return i == 0 ? x : (i == 1 ? y : z); } }
}

void main()
{
    Vec3 v;
    v.x = 10;
    v.y = 20;
    v.z = 30;
    // CHECK: a=10 b=20 c=30
    printf("a=%d b=%d c=%d\n", v[0], v[1], v[2]);
}
