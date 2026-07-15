//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=8bbf4cb8daa21d5c3d6e035a494748b9ecec4dbd1090f3e9e250c4555daedc9f
//META: purpose=A DeclRefIntVal is an unsubstituted generic value parameter; after the generic is instantiated, the IntVal collapses to its concrete value and the array carries the substituted size.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states `DeclRefIntVal` names "an unsubstituted generic
// value parameter" which "collapses to a constant after
// substitution". A generic struct parameterised by `let N : int`
// uses N as an array bound; at instantiation, N becomes a
// concrete integer and the array length is observable.

//TEST:INTERPRET(filecheck=CHECK):

struct Buf<let N : int>
{
    int data[N];
}

void main()
{
    Buf<3> b;
    b.data[0] = 10;
    b.data[1] = 20;
    b.data[2] = 30;
    // CHECK: b=10,20,30
    printf("b=%d,%d,%d\n", b.data[0], b.data[1], b.data[2]);
}
