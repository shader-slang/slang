//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`Texture2D<T>` belongs to the Image address space; SPIRV emits the variable with `UniformConstant` storage class.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `Texture2D<T>` belongs to the Image address space. The shader samples the
// texture and writes the result to an output buffer so DCE cannot remove the resource;
// CHECK asserts the SPIRV assembly declares `%myTexture` as `OpVariable ... UniformConstant`,
// confirming the Image address space maps to the UniformConstant storage class.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

Texture2D<float4>  myTexture;
SamplerState       mySampler;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float4> output)
{
    output[0] = myTexture.SampleLevel(mySampler, float2(0.5, 0.5), 0.0);
}

// CHECK: %myTexture = OpVariable {{.*}} UniformConstant
