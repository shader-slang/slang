//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=533753bf58b03097d708a72394179634eea26760dccf152ebe2bcd53f0cfb0c0
//META: purpose=A `{ ... }` block parses into a Stmt that holds an ordered sequence of sub-statements, executed in source order.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Three assignment statements inside one block. The accumulated value
// is order-dependent (subtraction is not commutative across the
// sequence), so the printed result only matches if the three
// sub-statements were parsed and executed in source order.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x;
    {
        x = 10;
        x = x - 3;
        x = x - 2;
    }
    // CHECK: x=5
    printf("x=%d\n", x);
}
