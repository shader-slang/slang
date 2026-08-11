//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=dfd7b5878428e66d4c6b22b8ce6a609a70c3ed9c74c32c1f56d5d54a8a3e67d5
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
