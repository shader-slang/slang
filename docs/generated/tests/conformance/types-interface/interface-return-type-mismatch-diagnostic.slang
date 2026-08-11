//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A member function whose return type is not convertible to the interface requirement's return type is rejected with E38106 (return type mismatch).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an implementation whose return type is not convertible to
// the interface requirement's return type is rejected with E38106.
// BadConvert returns float4 where float is required; CHECK pins the
// caret-aligned "return type mismatch" message and E38106.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

interface IConverter
{
    float convert(int x);
}

struct BadConvert : IConverter
{
    // float4 is not convertible to float
    float4 convert(int x) { return float4(x, 0, 0, 0); }
//CHECK:   ^^^^^^^ return type mismatch
//CHECK:   ^^^^^^^ member 'convert' return type 'vector<float,4>' does not match interface requirement return type 'float'.
}

void main() {}

// CHECK: E38106
