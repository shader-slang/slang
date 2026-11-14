//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#interface-requirements-vs-default-implementations
//META: doc_section_digest=74784468a186f2fede08fe868c02fc5f99f7dcb5531bbffd440daa487e75a556
//META: purpose=Lookup from inside a default implementation goes through its explicit This parameter and skips the interface's own requirement decls, so a requirement it calls dispatches to the conforming type's implementation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the scope-cursor jump the doc describes for default
// implementations. The default `wrapper()` body calls `base()` with no
// receiver written; because lookup is redirected at the explicit This
// parameter and then skips past the interface itself, the call binds to
// the conformer's `base()`. CHECK asserts 4 * 10, so the conformer's
// value reached the default body.

//TEST:INTERPRET(filecheck=CHECK):

interface I
{
    int base();
    int wrapper() { return base() * 10; }
}

struct S : I { int base() { return 4; } }

int call<T : I>(T t) { return t.wrapper(); }

void main()
{
    S s;
    // CHECK: r=40
    printf("r=%d\n", call(s));
}
