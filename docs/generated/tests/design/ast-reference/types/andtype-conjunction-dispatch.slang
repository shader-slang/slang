//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#andtype
//META: doc_section_digest=276be3a0ef406e1f03533a72d5ee898889d527b8cf55e4a39ec109f47c8fac42
//META: purpose=An AndType `IA & IB` tracks a conjunction of conformance requirements; a type conforming to both is accepted and both interfaces' methods dispatch in the generic body.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a conjunction constraint admits both interfaces at once
// without a named composite interface being declared. The generic body
// calls one method from each interface and combines them; CHECK asserts
// the combined result, which is unreachable unless both conformances
// were carried by the single constraint.

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
