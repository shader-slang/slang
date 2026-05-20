//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#andtype
//META: doc_section_digest=7025dcc263ba359294366e6446e2fc71a77eb827f3573dd04518f3b52362df10
//META: purpose=AndType extends to more than two conjuncts; an `IA & IB & IC` constraint dispatches methods from all three interfaces on a conforming value.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe of AndType beyond the two-conjunct smoke: a three-
// way conjunction `IA & IB & IC` accepts a struct conforming to
// all three interfaces and dispatches every interface method.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int a(); }
interface IB { int b(); }
interface IC { int c(); }

struct Triple : IA, IB, IC
{
    int a() { return 100; }
    int b() { return 20; }
    int c() { return 3; }
}

int combine<T : IA & IB & IC>(T v) { return v.a() + v.b() + v.c(); }

void main()
{
    Triple t;
    // CHECK: r=123
    printf("r=%d\n", combine(t));
}
