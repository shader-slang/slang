//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A UsingDecl brings names from another scope into the current one; an unqualified use of the imported name resolves.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of UsingDecl. `using namespace N`
// is a UsingDecl that makes `N`'s members reachable without the
// `N::` qualifier from the surrounding scope.

//TEST:INTERPRET(filecheck=CHECK):

namespace N { int compute() { return 42; } }

using namespace N;

void main()
{
    //CHECK: 42
    printf("%d\n", compute());
}
