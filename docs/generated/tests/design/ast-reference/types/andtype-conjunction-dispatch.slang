//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#andtype
//META: doc_section_digest=276be3a0ef406e1f03533a72d5ee898889d527b8cf55e4a39ec109f47c8fac42
//META: purpose=An AndType `IA & IB` requires conformance to both interfaces; a type conforming to both can be passed and both interface methods dispatch.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies AndType's claim that `IA & IB` tracks conjunctive
// conformance: a type conforming to both is accepted, and both
// interface methods are callable in the generic body. CHECK asserts
// the combined result, reachable only if both conformances were
// admitted.

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
    Both o;
    // CHECK: r=34
    printf("r=%d\n", combined(o));
}
