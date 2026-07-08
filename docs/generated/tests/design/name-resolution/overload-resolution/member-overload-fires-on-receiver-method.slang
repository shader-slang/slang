//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=61c0250465db1de2183c1e86044b9101be702b2abc84656a2bce95e208e63cb5
//META: purpose=Member-access overload set assembled by AddDeclRefOverloadCandidates resolves to the receiver-typed method whose parameter list matches the call.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two same-name methods on `S` differ by signature; calling with one
// int picks the unary overload.

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
