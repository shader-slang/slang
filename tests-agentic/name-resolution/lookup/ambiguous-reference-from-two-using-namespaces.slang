//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=b22ae0db87266fb01a1be1ff4487019df27480bf09ff6476d416556c6388f8d4
//META: purpose=When a non-overloadable lookup resolves to two same-named value decls reachable via two sibling-injected namespaces, the checker emits ambiguous-reference (code 39999).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "When a non-overloadable result resolves to multiple decls (e.g.
// two types with the same name reachable via different sibling
// scopes), the checker emits diagnostic `ambiguous-reference`."
// The companion 03-semantic-check test exercises this on types;
// here we cover the *value* (constant) ambiguity surface for the
// same diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

namespace A { static const int VAL = 1; }
namespace B { static const int VAL = 2; }

using namespace A;
using namespace B;

void main()
{
    int z = VAL;
//CHECK:    ^^^ ambiguous reference to 'VAL'
}
