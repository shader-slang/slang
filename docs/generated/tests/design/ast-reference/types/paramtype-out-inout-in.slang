//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=c7f55cb42aaa9e03df513fb2e3ff5bbcf380f2c09432d750a2aa9c55dfc56801
//META: purpose=The parameter-passing-mode types differ observably at the caller: `inout` threads a value through, `out` delivers a callee-assigned value, and plain `in` is a copy whose mutation does not propagate.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the three passing modes wrap the same `int` parameter
// type differently. The same caller-side local is passed to each; CHECK
// asserts 6 after the inout increment, 99 after the out assignment, and
// an unchanged 7 after the in-parameter mutation, so collapsing any two
// modes would change one of the three values.

//TEST:INTERPRET(filecheck=CHECK):

void addOne(inout int x) { x = x + 1; }
void setTo(out int x) { x = 99; }
void readOnly(in int x) { x = x + 1000; }

void main()
{
    int a = 5;
    addOne(a);
    // CHECK: a=6
    printf("a=%d\n", a);

    int b;
    setTo(b);
    // CHECK: b=99
    printf("b=%d\n", b);

    int c = 7;
    readOnly(c);
    // CHECK: c=7
    printf("c=%d\n", c);
}
