//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#modifiers
//META: doc_section_digest=70ceb760c32425b92d6d86403d9499a01d3afd02e92cf1d5ef7c2c183d0ee9ff
//META: purpose=`ModifierList` accepts multiple `ModifierKeyword` tokens (`static`, `const`) in sequence before a `VarDecl`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the parser collects a `ModifierList` of two
// ModifierKeywords (`static const`) attached to the file-scope
// `VarDecl`. The bound value is then read inside `main`.

//TEST:INTERPRET(filecheck=CHECK):

static const int kAnswer = 42;

void main()
{
    // CHECK: kAnswer=42
    printf("kAnswer=%d\n", kAnswer);
}
