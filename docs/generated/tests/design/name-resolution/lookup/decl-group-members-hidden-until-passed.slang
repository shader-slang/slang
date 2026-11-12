//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#block-local-shadowing
//META: doc_section_digest=24b2e37706dffd5a2f6286a13b0271a96dc30366c33fe244deca52d7be73e0e4
//META: purpose=In a multi-declarator statement each member is hidden until the checker passes it: a later member can see an earlier one, but an earlier initializer cannot see a later one and resolves to the outer name.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the DeclGroup handling of the hidden flag. In `int c = b,
// b = 1;` the first initializer names `b` before the group's own `b`
// has been passed, so lookup skips it and binds to the outer `b` (7).
// In `int a = 1, d = a + 1;` the earlier member is already visible to
// the later initializer. CHECK asserts both observations.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int b = 7;
    {
        int c = b, b = 1;
        int a = 1, d = a + 1;
        // CHECK: r=7 1 2
        printf("r=%d %d %d\n", c, b, d);
    }
}
