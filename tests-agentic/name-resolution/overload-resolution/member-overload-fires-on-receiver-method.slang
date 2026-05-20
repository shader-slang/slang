//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=25b7fa875eaa3f334cbe42c411b97e9677fd0d8ee67a01fca43e03581635f93c
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
