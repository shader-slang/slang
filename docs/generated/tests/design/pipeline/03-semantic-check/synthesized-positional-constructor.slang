//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=6d010633d724c7a7528759b725f5f229e533cca76c7c250f8ff4f590aa0f3a2b
//META: purpose=The checker synthesizes a positional constructor so a struct can be built from a brace-enclosed value list without an explicit `__init`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that construction members are added at check time rather than being
// required in the source. `Point` declares no `__init`, so a brace-enclosed
// initializer can only bind through a signature the checker synthesized.
// CHECK asserts both fields received the listed values in order.

//TEST:INTERPRET(filecheck=CHECK):

struct Point
{
    int x;
    int y;
}

void main()
{
    Point p = { 3, 4 };
    // CHECK: x=3 y=4
    printf("x=%d y=%d\n", p.x, p.y);
}
