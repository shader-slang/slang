//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=Entry-point output parameters and return values belong to the Output address space; SPIRV emits vertex shader outputs with `Output` storage class.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc table row for "Output" states: entry point output parameters and entry point
// return values are in the Output address space, instance scope = thread.
// "Entry point outputs from a graphics shader stage are inputs to the next stage."
// SPIRV: vertex shader output parameters emit with `Output` storage class.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry vsMain -stage vertex

struct VSOutput {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

VSOutput vsMain(float3 pos : POSITION, float2 uv : TEXCOORD0)
{
    VSOutput o;
    o.pos = float4(pos, 1.0);
    o.uv  = uv;
    return o;
}

// Vertex shader output (SV_Position and TEXCOORD0 output) use Output storage class.
// CHECK: OpVariable {{.*}} Output
