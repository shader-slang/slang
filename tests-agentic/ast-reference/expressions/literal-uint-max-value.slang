//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=Verifies that an IntegerLiteralExpr with the `u` suffix at uint MAX (0xFFFFFFFFu) carries the unsigned MAX value through evaluation.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says IntegerLiteralExpr's suffixType selects the base type.
// `u` selects uint; the full-range value 0xFFFFFFFF must survive.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    uint v = 0xFFFFFFFFu;
    // CHECK: v=4294967295
    printf("v=%u\n", v);
}
