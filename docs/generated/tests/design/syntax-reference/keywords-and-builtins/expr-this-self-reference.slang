//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=6c39a8a833fc084f0b1f7e30400e6efc997d8e919471186717dfce4ca0bf8e11
//META: purpose=`this` is an expression keyword denoting the current object inside a member function.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Read `this.value` from inside a member function and observe the
// owning instance's data. If `this` were not recognized as the
// self-reference keyword the program would fail to type-check.

//TEST:INTERPRET(filecheck=CHECK):

struct Foo
{
    int value;
    int getValue() { return this.value; }
};

void main()
{
    Foo f;
    f.value = 33;
    // CHECK: v=33
    printf("v=%d\n", f.getValue());
}
