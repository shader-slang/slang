//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#modifiers
//META: doc_section_digest=f7b0ae9b07f0675cd756d6bc3597c976e818c75e0b93ae38cb59ab21c210ef82
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
