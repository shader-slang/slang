//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A type whose associated type does not satisfy a `where T.AssocType == int` equality constraint is rejected at the call site.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Type equality constraint for associated type).
// FloatProperty's PropertyType is float, not int. Calling addTwoInts with FloatProperty
// must be rejected at compile time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFace
{
    associatedtype PropertyType;
    property prop : PropertyType { get; }
}

struct FloatProperty : IFace
{
    typealias PropertyType = float;
    float _val;
    property prop : float { get { return _val; } }
}

int addTwoInts<T : IFace>(T a, T b) where T.PropertyType == int
{
    return 0;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    FloatProperty fp1;
    FloatProperty fp2;
    int r = addTwoInts(fp1, fp2);
//CHECK: E39999
}
