//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#enumeration-types
//META: doc_section_digest=4b116a159e7885ebdd4cd361d9d70a300c658321193492133928c90bc17abd3e
//META: purpose=An explicit tag value in the middle of an enum sets that case's value; subsequent cases without an explicit value continue from there.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an explicit tag value mid-enum resets the counter and subsequent cases increment.
// `Red` gets 0 by default, `Green` is pinned to 3, and `Blue` must be 4;
// CHECK lines assert all three printed values match.

//TEST:INTERPRET(filecheck=CHECK):

enum Color { Red, Green = 3, Blue }

void main()
{
    // CHECK: 0
    // CHECK: 3
    // CHECK: 4
    printf("%d\n", int(Color.Red));
    printf("%d\n", int(Color.Green));
    printf("%d\n", int(Color.Blue));
}
