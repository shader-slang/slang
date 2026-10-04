//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#val-nodebase
//META: doc_section_digest=17754b4ad55bb3bbeb3b934dd617e4a3120a865a281a81848bf3fc5ab03948d7
//META: purpose=The Witness family are Vals, so a generic constrained by an interface carries a conformance value that dispatches each instantiation to its own implementation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an interface conformance is a compile-time value the
// generic can use. `getCode<T : ICode>` calls `code()` on T without
// knowing the concrete type; the only thing that can route the call is
// the conformance value supplied for each instantiation. CHECK asserts
// the two instantiations return their own codes.

//TEST:INTERPRET(filecheck=CHECK):

interface ICode { int code(); }

struct Red  : ICode { int code() { return 1; } };
struct Blue : ICode { int code() { return 2; } };

int getCode<T : ICode>(T v) { return v.code(); }

void main()
{
    Red  r;
    Blue b;
    //CHECK: red=1
    printf("red=%d\n", getCode(r));
    //CHECK: blue=2
    printf("blue=%d\n", getCode(b));
}
