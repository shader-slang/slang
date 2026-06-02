//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=Interface member functions may be static or non-static; a static method requirement is satisfied by a static method on the conforming struct.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states "The interface member functions may be static or non-static."
// Verifies that a static method requirement in an interface is satisfied
// by a matching static method on the conforming struct.

//TEST:INTERPRET(filecheck=CHECK):

interface IFactory
{
    static int create();
}

struct WidgetFactory : IFactory
{
    static int create() { return 99; }
}

void main()
{
    //CHECK: created=99
    printf("created=%d\n", WidgetFactory.create());
}
