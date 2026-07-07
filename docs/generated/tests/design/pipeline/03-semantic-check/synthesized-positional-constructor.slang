//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=51cb62c0b32d5853269f2948b06d5fe885de6c3a839dfdb3e56f3ab895330c38
//META: purpose=The checker synthesizes implicit construction code (e.g. an initializer-list constructor) so a struct can be built from a brace-enclosed value list without an explicit `__init`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Synthesizing implicit code section says construction methods
// are among the members added at check time. Verify the
// user-visible consequence: a struct with two int fields is
// initializable from `{3, 4}`.

//TEST:INTERPRET(filecheck=CHECK):

struct Point { int x; int y; }

void main()
{
    Point p = { 3, 4 };
    // CHECK: x=3 y=4
    printf("x=%d y=%d\n", p.x, p.y);
}
