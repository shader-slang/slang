//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An associated type requirement may be satisfied by a nested struct declaration with the same name inside the conforming struct.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states "An associated named type may be provided by:
// Declaring a nested structure with the same name."
// Verifies that a nested struct satisfies an associatedtype requirement.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasNestedType
{
    associatedtype Item;
    Item makeItem();
}

struct ContainerWithNested : IHasNestedType
{
    struct Item
    {
        int code;
    }

    Item makeItem()
    {
        Item it;
        it.code = 7;
        return it;
    }
}

void main()
{
    ContainerWithNested c;
    ContainerWithNested.Item item = c.makeItem();
    //CHECK: code=7
    printf("code=%d\n", item.code);
}
