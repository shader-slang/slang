//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=51cb62c0b32d5853269f2948b06d5fe885de6c3a839dfdb3e56f3ab895330c38
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
