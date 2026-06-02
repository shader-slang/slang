//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An interface may declare a subscript operator requirement without a body; the conforming struct provides compatible get and set implementations.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states that subscript operator declarations may be declared
// in interfaces with get/set without bodies. The conforming type provides
// implementations. Verified with __subscript get/set.

//TEST:INTERPRET(filecheck=CHECK):

interface IIndexable
{
    __subscript(uint i) -> int { get; set; }
}

struct FixedArray : IIndexable
{
    int data[4];

    __subscript(uint i) -> int
    {
        get { return data[i]; }
        set { data[i] = newValue; }
    }
}

void main()
{
    FixedArray fa;
    fa.data[0] = 0;
    fa.data[1] = 0;
    fa[0] = 10;
    fa[1] = 20;
    //CHECK: elem0=10 elem1=20
    printf("elem0=%d elem1=%d\n", fa[0], fa[1]);
}
