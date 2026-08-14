//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
//META: purpose=Boundary: `bit_cast<float>(uintVal)` lowers to `bitCast(%a)` with `Float` result type — reinterpret across the signed/unsigned + int/float boundary.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The existing smoke test covers `bit_cast<float>(int)`. This
// boundary probe reinterprets a `uint` (unsigned) as a `float`,
// crossing both the signedness and the int/float type boundary
// in one step.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    fbuf[0] = bit_cast<float>(a);
}

// IR-LABEL: func %main
// IR: bitCast(%a)
