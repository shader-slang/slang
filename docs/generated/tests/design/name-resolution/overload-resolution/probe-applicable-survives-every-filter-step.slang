//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=6e6bd1f54cf8c6c1c7ea9efd59c3c27ea43dc0195aebd524dc7f1c903ca95457
//META: purpose=A candidate that survives arity, fixity, type, direction, constraint and visibility filtering is tagged Applicable and is the call the resolver builds.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the all-steps-pass path of the probe pipeline. `bump` is the only
// candidate: the call has the required arity, is not a prefix/postfix form,
// the argument coerces exactly, the inout direction is satisfied by an l-value,
// there are no constraints, and the decl is visible. CHECK observes the
// mutation, which only happens if that candidate was chosen and invoked.

//TEST:INTERPRET(filecheck=CHECK):

void bump(inout int x) { x = x + 100; }

void main()
{
    int v = 7;
    bump(v);
    // CHECK: v=107
    printf("v=%d\n", v);
}
