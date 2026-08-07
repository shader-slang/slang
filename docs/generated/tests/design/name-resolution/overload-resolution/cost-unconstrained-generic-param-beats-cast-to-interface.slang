//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=d77239a753c95159f4db29b226999df363503e43e2ce42ec3d20143496696e2b
//META: purpose=kConversionCost_UnconstraintGenericParam (20) is cheaper than kConversionCost_CastToInterface (50), so the cost step selects the generic candidate before the specificity step could prefer the non-generic one.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the cost comparison runs ahead of the specificity comparison.
// Binding a conforming struct to an unconstrained T costs 20 while boxing it
// into the interface costs 50, so the generic candidate wins even though
// specificity would otherwise prefer the non-generic interface overload.
// CHECK asserts the generic overload's tag.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape { int kind(); }

struct Square : IShape { int kind() { return 4; } }

int pick(IShape s) { return 1; }
int pick<T>(T v)   { return 2; }

void main()
{
    Square sq;
    int tag = pick(sq);
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
