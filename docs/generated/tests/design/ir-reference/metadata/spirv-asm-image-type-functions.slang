//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#sampledtype--imagetype--sampledimagetype
//META: doc_section_digest=3802383fe789695ddbf7fcdf3ef23e57f2080fc08c83096add9b83bc97997b54
//META: purpose=The __imageType and __sampledImageType tokens are type functions whose operand is a value in scope, so the SPIR-V type is computed at emit time rather than stored in the IR.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two type functions are preserved as unevaluated operands. The
// inline asm derives the image type and the sampled-image type from the
// combined texture-sampler parameter rather than naming either type; the
// CHECKs require each type function to appear in the result-type position of
// its instruction with the value it was applied to still as its operand, which
// is what makes a generic inline-asm fragment adapt to the types in scope.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage fragment

[vk::binding(0)]
Sampler2D gTex;

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    return spirv_asm
    {
        %img : __imageType(gTex) = OpImage $gTex;
        %si : __sampledImageType(gTex) = OpSampledImage %img $gTex;
        result : $$float4 = OpImageSampleImplicitLod %si $uv
    };
}

// CHECK: SPIRVAsmInst({{[0-9]+}} : UInt, __imageType({{.*}}), %"img",
// CHECK-NEXT: SPIRVAsmInst({{[0-9]+}} : UInt, __sampledImageType({{.*}}), %"si",
