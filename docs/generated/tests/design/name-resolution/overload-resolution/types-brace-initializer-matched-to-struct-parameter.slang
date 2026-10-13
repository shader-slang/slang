//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=8893ade9a7efa04929eeabd0929b6ce5ed75204189b492f7d284d2f366d938f2
//META: purpose=TryCheckOverloadCandidateTypes obtains the struct parameter information and calls matchOverloadArgument, which matches and coerces a brace initializer argument instead of rejecting it outright.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `TryCheckOverloadCandidateTypes` obtains the type and passing mode of parameter `p`, then calls
// `matchOverloadArgument` for `{1, 2}`. That operation matches and coerces the initializer list to
// `P` instead of treating it as an unmatched argument. CHECK confirms that `take` receives both
// initialized field values.

//TEST:INTERPRET(filecheck=CHECK):

struct P { int a; int b; }

int take(P p) { return p.a * 10 + p.b; }

void main()
{
    int v = take({1, 2});
    // CHECK: v=12
    printf("v=%d\n", v);
}
