//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=90f68d89058e95e927dbc5ffd876802120cff1e5fe6639465168476f05517709
//META: purpose=The parser builds Expr nodes that honor operator precedence: `*` binds tighter than `+`, so `2 + 3 * 4` parses as `2 + (3 * 4)` and evaluates to 14.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// If `+` and `*` were parsed left-to-right with equal precedence the
// answer would be (2+3)*4 = 20. The printed 14 confirms the parser
// placed `*` deeper in the Expr tree than `+`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int r = 2 + 3 * 4;
    // CHECK: r=14
    printf("r=%d\n", r);
}
