//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=31abb67bf5286f69461146710f091d89f6d90d100b6c4de40ee80f2ebf88ef5a
//META: purpose=An interface-typed value wrapped in no_diff is still opened as an existential before member access, so a requirement is callable on it.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unwrapModifiedType step in front of existential
// opening. `c` is declared `no_diff IV`, so its type is a ModifiedType
// around the interface type; the member-access base is nevertheless
// opened, and `c.val()` resolves to the requirement and runs the
// conformer's body. CHECK asserts 5.

//TEST:INTERPRET(filecheck=CHECK):

interface IV { int val(); }

struct S : IV { int val() { return 5; } }

void main()
{
    no_diff IV c = S();
    // CHECK: r=5
    printf("r=%d\n", c.val());
}
