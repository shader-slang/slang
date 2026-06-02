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

// The doc says: "If the first case declaration… elides an initial-value expression,
// the value `0` is used… If any other case declaration elides an initial-value
// expression, its tag value is one greater than the tag value of the immediately
// preceding case declaration."

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
