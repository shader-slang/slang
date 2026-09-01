//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=745e90bcc2440ac2accf9529f203ae4435a595ab30563e368559c64f8e6f978e
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
