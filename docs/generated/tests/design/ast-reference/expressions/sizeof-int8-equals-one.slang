//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Verifies the lower-edge boundary of SizeOfExpr: `sizeof(int8_t)` is the documented value 1.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for SizeOfExpr: the smallest documented
// numeric type is 1 byte wide.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int s = int(sizeof(int8_t));
    // CHECK: s=1
    printf("s=%d\n", s);
}
