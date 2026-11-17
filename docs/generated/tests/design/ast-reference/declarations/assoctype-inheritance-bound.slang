//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#assoctypedecl
//META: doc_section_digest=9c9f01adf6122cb0dc3d48b249f6234107092edc3e147d2d88e10294fda7a2b7
//META: purpose=An inheritance-style bound on an associated type (`associatedtype A : IBar`) constrains every conformer's choice of that type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `: IBar` bound spelling on an associated type. A generic
// constrained on the outer interface calls a member of `IBar` on the value
// the associated-type-typed method returns; that call is only well-typed if
// the bound was recorded, and the CHECK asserts its result.

//TEST:INTERPRET(filecheck=CHECK):

interface IBar { int bar(); }

struct Impl : IBar { int bar() { return 9; } }

interface IMakes
{
    associatedtype A : IBar;
    A make();
}

struct S : IMakes
{
    typealias A = Impl;
    A make() { A a; return a; }
}

int useMake<T : IMakes>(T t) { return t.make().bar(); }

void main()
{
    S s;
    //CHECK: 9
    printf("%d\n", useMake(s));
}
