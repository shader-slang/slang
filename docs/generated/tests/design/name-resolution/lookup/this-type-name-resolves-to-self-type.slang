//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=8db1d44af39131b43cc8258b2bec4dad2a4381ae619907da2e2b6a9de2021028
//META: purpose=The name This inside a non-interface aggregate type resolves to the enclosing type's own decl-ref rather than being looked up as an ordinary member.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the first case of the member-lookup decl handler: This in
// something that is not an interface short-circuits to the decl-ref
// being searched. The static factory declares its local and its return
// type as `This`, which must denote Box; CHECK asserts the field read
// back through a `Box`-typed variable, which only type-checks if This
// resolved to Box itself.

//TEST:INTERPRET(filecheck=CHECK):

struct Box
{
    int v;
    static This make(int n) { This b; b.v = n; return b; }
}

void main()
{
    Box b = Box::make(9);
    // CHECK: r=9
    printf("r=%d\n", b.v);
}
