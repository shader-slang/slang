//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=917434add55b557b228b443846eefc5d6962c4a14daa202ebe626eecee22d1ca
//META: purpose=The statement parser keeps a declaration parsed inside a function body when it is a variable, a `struct`, a `typedef`, a `typealias`, or a `using`, so all five forms declare usable names.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the accept side of the statement-level declaration filter, the
// counterpart to the `namespace` rejection. Each accepted form is not merely
// parsed but *used* immediately afterwards — the struct is instantiated and
// read, both alias spellings declare a variable, and the `using` is what makes
// the unqualified `helper()` call resolve — so a form that was silently
// dropped instead of kept would fail to compile rather than quietly pass.
// The printed sum is the arithmetic witness that every one of them bound.

//TEST:INTERPRET(filecheck=CHECK):

namespace NS { int helper() { return 7; } }

void main()
{
    struct S { int a; };
    typedef int AliasA;
    typealias AliasB = int;
    using NS;

    S s;
    s.a = 1;
    AliasA m = 2;
    AliasB n = 3;

    //CHECK: sum=13
    printf("sum=%d\n", s.a + m + n + helper());
}
