//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=4e5c787f52095c6dbdd20c8c0f4df226f6e2e9bbef0e89d6a888ac6bb5940443
//META: purpose=`if` and `else` are statement keywords recognized by direct identifier comparison in the statement parser; the conditional branches according to the boolean argument.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `if (cond) { ... } else { ... }` is accepted by the
// parser and that the `else` clause is reached when the condition is
// false. Both arms write the same variable, so a parser that silently
// dropped the `else` clause would still leave the variable at its
// initial value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 0;
    if (false) { x = 7; } else { x = 9; }
    // CHECK: x=9
    printf("x=%d\n", x);
}
