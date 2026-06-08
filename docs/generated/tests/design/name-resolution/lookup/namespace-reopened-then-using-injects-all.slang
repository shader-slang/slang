//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#module-and-namespace
//META: doc_section_digest=29b1e50e76ce09df697e30313ad1997122b36545720e82abde315d7de649b1b6
//META: purpose=Multiple namespace blocks of the same name collapse into the same NamespaceDecl; a single `using namespace` then injects every accumulated member into the unqualified lookup chain.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "Multiple `namespace Foo {}` declarations in the
// same module collapse into the same `NamespaceDecl` — the parser
// explicitly reuses the first one it finds." Confirm that one
// `using namespace M` makes BOTH alpha (from the first block) and
// beta (from the re-opened block) unqualified-reachable, proving
// the two blocks share the same merged NamespaceDecl.

//TEST:INTERPRET(filecheck=CHECK):

namespace M
{
    int alpha() { return 1; }
}

namespace M
{
    int beta() { return 2; }
}

using namespace M;

void main()
{
    // CHECK: r=3
    printf("r=%d\n", alpha() + beta());
}
