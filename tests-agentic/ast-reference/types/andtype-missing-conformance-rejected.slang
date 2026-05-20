//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#andtype
//META: doc_section_digest=7025dcc263ba359294366e6446e2fc71a77eb827f3573dd04518f3b52362df10
//META: purpose=An AndType `IA & IB` requires conformance to both interfaces; a type that conforms to only one fails the conjunction and the call is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The flip side of AndType conjunction: a type that conforms to
// only IA but not IB is NOT accepted where `IA & IB` is required,
// and the checker rejects the call.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IA { int a(); }
interface IB { int b(); }

struct OnlyA : IA { int a() { return 1; } }

int combined<T : IA & IB>(T v) { return v.a() * 10 + v.b(); }

void main()
{
    OnlyA o;
    int r = combined(o);
//CHECK:            ^ could not specialize generic for arguments of type (OnlyA)
}
