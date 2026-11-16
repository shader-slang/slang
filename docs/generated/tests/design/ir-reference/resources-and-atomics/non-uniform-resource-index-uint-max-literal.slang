//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#nonuniformresourceindex
//META: doc_section_digest=da7804ffd61b86af2c970fd765eb12daa92bd5d94d9373a5ac2b08752465a66f
//META: purpose=`NonUniformResourceIndex(0xFFFFFFFFu)` (UINT_MAX literal) still surfaces as `nonUniformResourceIndex(%i)` with the same uint type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: largest representable uint as the operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint nu = NonUniformResourceIndex(0xFFFFFFFFu);
    rwbuf[nu & 0xff] = tid.x;
}

// IR-LABEL: func %main
// IR: nonUniformResourceIndex({{.*}})
