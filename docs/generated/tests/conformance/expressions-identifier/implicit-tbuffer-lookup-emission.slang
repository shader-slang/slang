//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#implicit-lookup
//META: doc_section_digest=81444a04e3158afb0f979b5eaa4e66672a28128589f884b354ac44b03dbff7ad
//META: purpose=A bare identifier resolves to a field declared inside a global-scope tbuffer, via implicit tbuffer lookup.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a bare identifier inside a compute shader resolves to a field of
// a global-scope `tbuffer` via implicit tbuffer lookup. The bare name `multiplier`
// must resolve to `TParams.multiplier` without qualification. CHECK asserts the
// emitted HLSL contains `tbuffer` and `multiplier`, confirming the lookup and emission.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

tbuffer TParams
{
    int multiplier;
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    // 'multiplier' is a bare identifier resolving via implicit tbuffer lookup.
    outBuf[0] = multiplier * 2;
}

// CHECK: tbuffer
// CHECK: multiplier
