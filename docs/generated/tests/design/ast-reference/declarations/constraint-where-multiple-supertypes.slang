//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=3c3b338a49679f1b453ba7815d8deac7d31be7956bc0fe69e33cdab3863157ae
//META: purpose=A `where T : IA, IB` clause with a comma-separated supertype list produces one constraint per supertype, so members of both interfaces are usable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the comma-separated supertype list in a `where` clause is a
// list of independent constraints, not one constraint on a compound type. The
// body calls a member of each interface; the CHECK asserts the sum, which
// requires both constraints to be in effect.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int a(); }
interface IB { int b(); }

struct S : IA, IB
{
    int a() { return 1; }
    int b() { return 20; }
}

int sumBoth<T>(T t) where T : IA, IB { return t.a() + t.b(); }

void main()
{
    S s;
    //CHECK: 21
    printf("%d\n", sumBoth(s));
}
