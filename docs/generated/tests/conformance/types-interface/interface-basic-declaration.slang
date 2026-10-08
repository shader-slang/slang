//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An interface declares a set of member requirements; a conforming struct must provide compatible implementations of all required members.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies basic interface conformance: a struct satisfying a simple
// method requirement compiles and the method is callable at runtime.

//TEST:INTERPRET(filecheck=CHECK):

interface IValue
{
    int getValue();
}

struct Concrete : IValue
{
    int v;
    int getValue() { return v; }
}

void main()
{
    Concrete obj;
    obj.v = 42;
    //CHECK: result=42
    printf("result=%d\n", obj.getValue());
}
