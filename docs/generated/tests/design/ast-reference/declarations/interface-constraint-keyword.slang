//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=cb681a5a5428e160c2311cbf091e8eda34ecba068f868b779a464083304bb62f
//META: purpose=An `__constraint A == B;` written inside an interface body is a constraint requirement every conformer must satisfy.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__constraint` inside an interface body constrains conformers
// rather than the interface itself. The derived interface asserts that a
// conformer's associated type equals the conformer; a struct that supplies
// `DataType = S` conforms, and the CHECK asserts a value dispatched through a
// generic constrained on that derived interface.

//TEST:INTERPRET(filecheck=CHECK):

interface IBase { associatedtype DataType; int val(); }

interface ISelf : IBase { __constraint DataType == This; }

struct S : ISelf
{
    typealias DataType = S;
    int val() { return 13; }
}

int useSelf<T : ISelf>(T t) { return t.val(); }

void main()
{
    S s;
    //CHECK: 13
    printf("%d\n", useSelf(s));
}
