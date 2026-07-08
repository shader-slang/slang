//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=1730544025501f67efee33835c81f561c62ac7733b5e89832c526106e61b012f
//META: purpose=A `using` declaration splices the named namespace's scope in as a sibling, making the namespace's members reachable without qualification.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies sibling-scope use case 4: after `using Util;` the checker
// wires Util's scope as a sibling, so calling `triple` with no `Util::`
// qualifier resolves. CHECK asserts the call's result, which is only
// computable if the unqualified name reached the namespace member.

//TEST:INTERPRET(filecheck=CHECK):

namespace Util
{
    int triple(int x) { return 3 * x; }
}

using Util;

void main()
{
    int r = triple(5);
    // CHECK: r=15
    printf("r=%d\n", r);
}
