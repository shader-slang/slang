//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=5cf1a0be2720fdb7640ba4534ce97ec9428cc65b37576f2b89297596bf873eb6
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
