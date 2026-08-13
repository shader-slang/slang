//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=A UsingDecl brings names from another scope into the current one so they resolve without qualification.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `using namespace N;` makes `N`'s members reachable
// unqualified. The call in `main` uses the bare name; the CHECK asserts the
// namespaced function's result, so the using declaration must have injected
// the name into the enclosing scope.

//TEST:INTERPRET(filecheck=CHECK):

namespace N { int compute() { return 42; } }

using namespace N;

void main()
{
    //CHECK: 42
    printf("%d\n", compute());
}
