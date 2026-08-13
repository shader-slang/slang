//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=1fb3516e49ddbb5d4259751275e6b594d9d69aa0b1266c092e549ec5de2b449a
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
