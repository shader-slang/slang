//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#andtype
//META: doc_section_digest=7025dcc263ba359294366e6446e2fc71a77eb827f3573dd04518f3b52362df10
//META: purpose=An AndType `IA & IB` requires conformance to both interfaces; a type that conforms to both has both interface methods dispatched on it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `AndType` represents a conjunction of interface conformances.
// A struct that satisfies both interfaces is acceptable where
// `T : IA & IB` is required, and methods from BOTH interfaces are
// dispatched on the same value.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int a(); }
interface IB { int b(); }

struct Both : IA, IB
{
    int a() { return 3; }
    int b() { return 4; }
}

int combined<T : IA & IB>(T v) { return v.a() * 10 + v.b(); }

void main()
{
    Both v;
    // CHECK: c=34
    printf("c=%d\n", combined(v));
}
