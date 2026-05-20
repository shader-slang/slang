//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=c2e407a8f2f30473161ca1872b47d857bf827b2378eb5ccd399e2c4f1cecb0b6
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
