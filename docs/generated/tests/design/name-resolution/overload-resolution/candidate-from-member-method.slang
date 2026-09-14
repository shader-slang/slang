//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=09e8a26796107f4ea870ece7161025de2c11ac3fe6f559cd6a8ba18ea88a706d
//META: purpose=AddDeclRefOverloadCandidates turns each member of a lookup result into a candidate, so a member call resolves to the same-name method whose parameter list matches.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the member-callable candidate family. Both `act` methods are found
// on the receiver, so both become candidates and the arity step keeps only the
// unary one. CHECK asserts its tag.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    int act(int a)        { return 1; }
    int act(int a, int b) { return 2; }
}

void main()
{
    S s;
    int tag = s.act(5);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
