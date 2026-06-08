//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#implicit-lookup
//META: doc_section_digest=81444a04e3158afb0f979b5eaa4e66672a28128589f884b354ac44b03dbff7ad
//META: purpose=A bare identifier inside a function body resolves to a field declared inside a global-scope cbuffer, via implicit cbuffer lookup.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// When a global-scope cbuffer declares a field, a bare identifier in function scope
// resolves to that field. The emitted HLSL must reference the cbuffer-qualified field
// (not treat it as an undefined local), confirming implicit cbuffer lookup occurred.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

cbuffer Params
{
    int scale;
    int offset;
}

RWStructuredBuffer<int> outBuf;

int compute(int x)
{
    // 'scale' and 'offset' are bare identifier expressions that resolve to
    // the cbuffer fields via implicit cbuffer lookup.
    return x * scale + offset;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = compute(3);
}

// The emitted HLSL must reference the cbuffer-qualified field names.
// CHECK: cbuffer
// CHECK: scale
// CHECK: offset
