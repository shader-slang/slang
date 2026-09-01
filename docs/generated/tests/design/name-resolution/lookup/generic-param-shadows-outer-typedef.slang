//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#generic-parameters
//META: doc_section_digest=e6a604ecd95d745398ce230ff444eb5e4ef8a7eb569f1a45fd57675c245e1700
//META: purpose=A generic parameter is a direct member of the generic's own scope, which the inner decl's parent chain passes through first, so it shadows a same-named decl in the enclosing scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the generic scope sits between the inner decl and the
// file scope. A file-scope `typedef float T` competes with the generic
// parameter `T`; only the parameter admits the IInteger constraint and
// the int instantiation. CHECK asserts 7 round-tripping through
// asInt(), which requires `T` inside the function to be the parameter.

//TEST:INTERPRET(filecheck=CHECK):

typedef float T;

interface IInteger { int asInt(); }
extension int : IInteger { int asInt() { return this; } }

int handle<T : IInteger>(T x) { return x.asInt(); }

void main()
{
    // CHECK: r=7
    printf("r=%d\n", handle<int>(7));
}
