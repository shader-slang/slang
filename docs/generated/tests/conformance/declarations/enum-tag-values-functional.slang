//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#enumeration-types
//META: doc_section_digest=4b116a159e7885ebdd4cd361d9d70a300c658321193492133928c90bc17abd3e
//META: purpose=The first enum case without an explicit tag value gets value 0; subsequent cases without an explicit value get previous+1.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the default tag-value assignment rule: first case = 0, each subsequent case = previous + 1.
// `Status.Active`, `Status.Inactive`, and `Status.Pending` must print 0, 1, 2 respectively;
// the three CHECK lines confirm the sequential assignment.

//TEST:INTERPRET(filecheck=CHECK):

enum Status { Active, Inactive, Pending }

void main()
{
    // CHECK: 0
    // CHECK: 1
    // CHECK: 2
    printf("%d\n", int(Status.Active));
    printf("%d\n", int(Status.Inactive));
    printf("%d\n", int(Status.Pending));
}
