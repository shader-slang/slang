//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#val-nodebase
//META: doc_section_digest=17754b4ad55bb3bbeb3b934dd617e4a3120a865a281a81848bf3fc5ab03948d7
//META: purpose=A DeclRefBase names a declaration together with optional generic substitutions, so one generic declaration referenced at two different substitutions yields two different specializations.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a reference to a declaration carries its generic
// arguments with it. The single generic `id` is referenced twice, at
// `int` and at `float`; CHECK asserts each call returns a value of the
// substituted type. If a reference could only name the declaration and
// not its substitution, the two call sites could not disagree about T.

//TEST:INTERPRET(filecheck=CHECK):

T id<T>(T v) { return v; }

void main()
{
    int   i = id<int>(7);
    float f = id<float>(2.5f);
    //CHECK: i=7
    printf("i=%d\n", i);
    //CHECK: f=2.500000
    printf("f=%f\n", f);
}
