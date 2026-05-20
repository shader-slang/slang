//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=d6e74209f0cfcbd939127caa9bf2791387db9155c2d0b2f0a561349e2e185456
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
