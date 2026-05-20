//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=c71eed362b7090c94c5447396e3ebd7ff5d3d15480a793a8f05cdaa4015e935b
//META: purpose=A candidate that survives all probe-phase steps (arity, fixity, types, directions, constraints, visibility) is tagged Status::Applicable; with one candidate set the resolver picks it directly.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Single-overload positive: `bump` requires an inout int argument,
// the call has the right arity (1), passes type/direction checks,
// is visible, and reaches Applicable. The chosen candidate is
// invoked and we observe the mutation.

//TEST:INTERPRET(filecheck=CHECK):

void bump(inout int x) { x = x + 100; }

void main()
{
    int v = 7;
    bump(v);
    // CHECK: v=107
    printf("v=%d\n", v);
}
