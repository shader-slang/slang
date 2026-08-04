//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=f9c7becda59ea2cafbc68bded1f9fb4c90f4bada02e2725c3fe5655439f0c486
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
