//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#andtype
//META: doc_section_digest=276be3a0ef406e1f03533a72d5ee898889d527b8cf55e4a39ec109f47c8fac42
//META: purpose=An AndType `IA & IB` requires both conformances; a type conforming to only IA is rejected where `IA & IB` is required.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative flip side of the conjunction constraint: satisfying one half
// is not enough. The argument conforms to IA only, so the constraint
// must fail on the other half. CHECK pins the diagnostic naming the
// missing interface, which shows both halves were checked rather than
// the conjunction collapsing to its first member.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IA { int a(); }
interface IB { int b(); }

struct OnlyA : IA { int a() { return 1; } }

int combined<T : IA & IB>(T v) { return v.a() * 10 + v.b(); }

void main()
{
    OnlyA o;
    int r = combined(o);
//CHECK:            ^ type argument 'OnlyA' does not conform to the required interface 'IB'
}
