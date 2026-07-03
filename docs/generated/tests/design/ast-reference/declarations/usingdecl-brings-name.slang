//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
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
