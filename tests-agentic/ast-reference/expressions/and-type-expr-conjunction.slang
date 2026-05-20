//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr
//META: doc_section_digest=4254abda0523877f7fb67acc9da1ad7de3884b6d869e71f92f12575587230f39
//META: purpose=An AndTypeExpr `IA & IB` denotes a conjunction-of-conformances type; a value conforming to both can be passed where the conjunction is required.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// AndTypeExpr is listed in the type-expression family with the
// `T & U` spelling for "conjunction of conformances". A struct that
// conforms to both interfaces can be passed where `IA & IB` is
// expected; both interface methods are dispatchable on the value.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int a(); }
interface IB { int b(); }

struct S : IA, IB
{
    int a() { return 1; }
    int b() { return 2; }
}

int combined<T : IA & IB>(T v) { return v.a() * 10 + v.b(); }

void main()
{
    S s;
    // CHECK: c=12
    printf("c=%d\n", combined(s));
}
