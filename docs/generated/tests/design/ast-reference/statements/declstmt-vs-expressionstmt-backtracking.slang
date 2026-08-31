//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#declstmt-expressionstmt-and-emptystmt
//META: doc_section_digest=2ff8fbd6c14c1930e6df45b007c07bb9e931f77d21a4e28207d5df091a83100d
//META: purpose=A statement that starts with an identifier is re-parsed as a declaration when an identifier follows the type and as an expression statement otherwise, so `S s;` and `s.v = 3;` in the same block are classified differently.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two identifier-initial statement forms both work in one
// block: `Color c;` must become a local declaration (so the name is
// usable afterwards) while `c.v = 3;` must become an expression
// statement evaluated for its assignment. The CHECK asserts the value
// computed from both, which is unreachable if either form were
// classified as the other.

//TEST:INTERPRET(filecheck=CHECK):

struct Color
{
    int v;
}

void main()
{
    Color c;
    c.v = 3;
    int n = 0;
    n = c.v + 4;
    // CHECK: n=7
    printf("n=%d\n", n);
}
