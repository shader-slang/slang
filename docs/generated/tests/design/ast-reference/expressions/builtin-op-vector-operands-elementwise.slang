//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=81b1b9ef9946750fa4993fc9e38d2ffd230f962a9583bad2803017f58f335d29
//META: purpose=The builtin-operator fast path covers vector operands: `+` and `*` on int3 values produce elementwise results.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that builtin binary operators accept vector operands, one of
// the operand shapes the checker's fast path recognizes. Two int3 values
// are added and multiplied; CHECK asserts every component so a
// mis-resolved operation kind or a scalar-only path would be caught.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int3 a = int3(1, 2, 3);
    int3 b = int3(10, 20, 30);
    int3 s = a + b;
    int3 p = a * b;
    // CHECK: s=11,22,33
    printf("s=%d,%d,%d\n", s.x, s.y, s.z);
    // CHECK: p=10,40,90
    printf("p=%d,%d,%d\n", p.x, p.y, p.z);
}
