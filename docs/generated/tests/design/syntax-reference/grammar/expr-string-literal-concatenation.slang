//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#literal-forms-vs-token-kinds
//META: doc_section_digest=f5c669bb9740dfa642523af3da2be0ac7b679950ab4f326668ed87a90288d1e6
//META: purpose=`Literal ::= STRING_LIT+` — adjacent string-literal tokens fuse into a single string literal.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `+` repetition on STRING_LIT. Three adjacent literal
// tokens are written where one format string is expected; if they were
// three separate atoms the call would not type-check as a single
// format argument, and the printed text would not be the concatenation.
// Observing `abcd` on one line proves the tokens fused into one literal
// whose trailing newline came from the third token.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: abcd
    printf("ab" "cd" "\n");
}
