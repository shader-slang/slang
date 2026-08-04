//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:40Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=f0fb19c83c1fe744cfc27441df1ce1770502bc22a362ebfb8484f2adaed2edb9
//META: purpose=A `using` declaration brings a namespace (a `NamespaceDeclBase`) into scope so its members resolve unqualified; it is a scope-import, not a per-decl re-export.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the accepted shape of a `using` declaration: its argument
// resolves to a namespace, and afterwards the namespace's members are
// reachable without qualification. CHECK asserts the unqualified call
// resolves to the namespace member and returns its value, which is only
// possible if `using Util;` imported the namespace scope.

//TEST:INTERPRET(filecheck=CHECK):

namespace Util
{
    int triple(int x) { return x * 3; }
}

using Util;

void main()
{
    // CHECK: triple=21
    printf("triple=%d\n", triple(7));
}
