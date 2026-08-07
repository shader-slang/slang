//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=bf6116c2e5b561cd38063b60dd513143618c8f41f5c801a0b352865012956ca2
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
