//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#modifiers
//META: doc_section_digest=70ceb760c32425b92d6d86403d9499a01d3afd02e92cf1d5ef7c2c183d0ee9ff
//META: purpose=`ModifierKeyword` includes `inout`, which attaches to a `Param`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an `inout` modifier on a parameter parses and produces
// a pass-by-reference binding observable at the call site.

//TEST:INTERPRET(filecheck=CHECK):

void inc(inout int x) { x = x + 1; }

void main()
{
    int v = 10;
    inc(v);
    // CHECK: v=11
    printf("v=%d\n", v);
}
