//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
//META: purpose=A name inside a NamespaceDecl scope is reachable via explicit `Namespace::name` qualification.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Positive companion to namespace-member-needs-qualification: the
// same `inner::answer` is reachable with explicit qualification.

//TEST:INTERPRET(filecheck=CHECK):

namespace inner {
    int answer() { return 7; }
}

void main()
{
    int q = inner::answer();
    // CHECK: q=7
    printf("q=%d\n", q);
}
