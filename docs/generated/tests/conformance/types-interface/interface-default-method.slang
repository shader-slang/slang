//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An interface member function with an implementation body provides a default; the conforming struct inherits it without overriding.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states "Any number of member functions ... with the
// implementation body, which are added to the inheriting type".
// Verifies that a concrete struct inherits a default method from
// the interface without needing to override it.

//TEST:INTERPRET(filecheck=CHECK):

interface IWithDefault
{
    int base();
    int doubled() { return base() * 2; }
}

struct MyStruct : IWithDefault
{
    int _v;
    int base() { return _v; }
}

void main()
{
    MyStruct s;
    s._v = 7;
    //CHECK: doubled=14
    printf("doubled=%d\n", s.doubled());
}
