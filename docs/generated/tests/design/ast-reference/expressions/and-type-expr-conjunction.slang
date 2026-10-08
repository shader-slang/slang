//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-packbranchtypeexpr
//META: doc_section_digest=c81f550a9810da4877739b3075a0cd088585204924f21fbdc02d017dbf1948e5
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
