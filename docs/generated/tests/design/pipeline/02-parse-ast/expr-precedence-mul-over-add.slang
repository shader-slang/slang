//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=533753bf58b03097d708a72394179634eea26760dccf152ebe2bcd53f0cfb0c0
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
