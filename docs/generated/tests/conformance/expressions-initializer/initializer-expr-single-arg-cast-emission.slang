//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:11+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-initializer.md#initializer-expressions
//META: doc_section_digest=4baa222d5b95c67d63377172d8a9ca06fa2e19f939661ef5099353e8c61b3470
//META: purpose=A single-argument initializer expression int(f) emits as an int() cast in HLSL, matching the C-style cast form.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// C3 emission: the doc states int(1.0f) and (int) 1.0f are equivalent.
// Verify that HLSL emission uses int(...) cast form for both.
// The function must be reachable to avoid DCE.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

float passFloat(float f) { return f; }

int castViaInitExpr(float f)
{
    // single-argument initializer expression form
    return int(f);
}

int castViaCStyle(float f)
{
    // C-style cast form — must be equivalent
    return (int)f;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(RWStructuredBuffer<int> buf)
{
    buf[0] = castViaInitExpr(passFloat(buf[1]));
    buf[2] = castViaCStyle(passFloat(buf[3]));
}

// Both functions should emit int(...) in HLSL
// CHECK: int(
// CHECK: int(
