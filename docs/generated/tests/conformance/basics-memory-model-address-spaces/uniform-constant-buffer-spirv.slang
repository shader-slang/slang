//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`ConstantBuffer<T>` belongs to the Uniform address space; SPIRV emits the variable with `Uniform` storage class.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `ConstantBuffer<T>` belongs to the Uniform address space in SPIRV.
// The shader reads `scale` and `offset` from the buffer and writes the sum to an output so
// the resource is not DCE'd; CHECK asserts the SPIRV assembly contains an
// `OpVariable ... Uniform`, confirming the Uniform address-space-to-storage-class mapping.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

struct Params {
    float scale;
    int   offset;
};

ConstantBuffer<Params> cb;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    output[0] = cb.scale + float(cb.offset);
}

// CHECK: OpVariable {{.*}} Uniform
// Metal maps ConstantBuffer to constant address space pointer.
// METAL: constant* cb_
// WGSL maps ConstantBuffer to var<uniform>.
// WGSL: var<uniform> cb_
