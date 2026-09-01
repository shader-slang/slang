//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#explicit-vkbinding-on-entry-point-parameters
//META: doc_section_digest=d594d25adb697c7a448d8fd4e5af2ada74a9ee315fdd20515276d88cbf22323f
//META: purpose=One entry point can mix an explicitly vk::binding-bound parameter with an implicitly bound one; the explicit reservation is kept and the implicit parameter is allocated around it.
//META: intent=boundary
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-parameter completion path: the explicit reservation must
// survive completion while the neighbouring implicit parameter still gets a
// real allocated binding from the global pool. CHECK asserts the explicit
// parameter keeps 9/2 and the implicit one lands at binding 1 of set 0,
// i.e. after the module-scope outBuf rather than at a field-relative offset.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main(
    [[vk::binding(9, 2)]] uniform StructuredBuffer<int> explicitBuf,
    uniform StructuredBuffer<int> implicitBuf)
{
    outBuf[0] = explicitBuf[0] + implicitBuf[0];
}

// SPV-DAG: OpDecorate %entryPointParams_explicitBuf{{[A-Za-z0-9_]*}} Binding 9
// SPV-DAG: OpDecorate %entryPointParams_explicitBuf{{[A-Za-z0-9_]*}} DescriptorSet 2
// SPV-DAG: OpDecorate %entryPointParams_implicitBuf{{[A-Za-z0-9_]*}} Binding 1
// SPV-DAG: OpDecorate %entryPointParams_implicitBuf{{[A-Za-z0-9_]*}} DescriptorSet 0
