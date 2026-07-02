//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#andtype
//META: doc_section_digest=276be3a0ef406e1f03533a72d5ee898889d527b8cf55e4a39ec109f47c8fac42
//META: purpose=An AndType `IA & IB` requires both conformances; a type conforming to only IA is rejected where `IA & IB` is required.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative flip side of AndType conjunction: a type conforming to
// only IA (not IB) is not accepted where `IA & IB` is required. CHECK
// pins the non-conformance diagnostic naming the missing interface.

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
