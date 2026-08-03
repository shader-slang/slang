//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=1730544025501f67efee33835c81f561c62ac7733b5e89832c526106e61b012f
//META: purpose=A `using` of a namespace declared in multiple blocks makes members from every block reachable unqualified, because the splice covers the namespace's owned and sibling scopes.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Combines sibling-scope use cases 3 and 4: the reopened `namespace Pair`
// parses into one container, and `using Pair;` splices that namespace's
// owned/sibling scopes in. Summing one member from each block proves both
// declarations are reachable through the single unqualified splice.

//TEST:INTERPRET(filecheck=CHECK):

namespace Pair
{
    int first() { return 10; }
}

namespace Pair
{
    int second() { return 20; }
}

using Pair;

void main()
{
    int s = first() + second();
    // CHECK: s=30
    printf("s=%d\n", s);
}
