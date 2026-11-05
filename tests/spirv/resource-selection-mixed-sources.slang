//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry branchIndex -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry branchIndex -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry branchIndex -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry differentArrays -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry differentArrays -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry differentArrays -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry nestedArrays -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry selectedArray -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry selectedArray -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry selectedArray -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry selectedRow -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry heapOrArray -capability spvDescriptorHeapEXT -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry heapOrArray -capability spvDescriptorHeapEXT -validate-ir -validate-ir-detailed

// We select resources reached through different declarations and through different numbers
// of array subscripts. Descriptor indices must remain ordinary SSA values even when an index
// is computed only on one incoming branch or the selected value is itself an array.
// The direct SPIR-V emitter retains nested descriptor array types, which Vulkan rejects even
// without a resource selection. We exercise multidimensional arrays and selected rows through
// GLSL output and use one-dimensional descriptor arrays for the SPIR-V validation checks.
// SPIRV: OpImageSampleExplicitLod
// GLSL: textureLod

Texture2D<float4> singleTexture;
Texture2D<float4> firstTextures[4];
Texture2D<float4> secondTextures[4];
Texture2D<float4> textureRows[2][4];
Texture2D<float4> otherRows[2][4];
SamplerState samplers[4];
cbuffer Parameters { uint index; uint otherIndex; };

// The selected descriptor index is computed inside the branch, while the sample coordinates
// are computed after the merge. We require both values to remain available at the sample.
float4 branchIndex(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> selected = singleTexture;
    if (uv.x > 0.5)
    {
        uint branchIndex = (index + uint(uv.y)) % 4;
        selected = firstTextures[branchIndex];
    }
    float2 coordinate = uv * 0.5 + 0.25;
    return selected.SampleLevel(samplers[0], coordinate, 0);
}

float4 differentArrays(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> selected = select(uv.x > 0.5,
        firstTextures[index % 4], secondTextures[otherIndex % 4]);
    return selected.SampleLevel(samplers[0], uv, 0);
}

float4 nestedArrays(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> selected = select(uv.x > 0.5,
        textureRows[index % 2][otherIndex % 4],
        otherRows[otherIndex % 2][index % 4]);
    return selected.SampleLevel(samplers[0], uv, 0);
}

float4 selectedArray(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> textures[4] = uv.x > 0.5 ? firstTextures : secondTextures;
    return textures[index % 4].SampleLevel(samplers[0], uv, 0);
}

float4 selectedRow(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> row[4] = uv.x > 0.5 ? textureRows[0] : otherRows[1];
    return row[index % 4].SampleLevel(samplers[0], uv, 0);
}

float4 heapOrArray(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState selected = select(uv.x > 0.5,
        (SamplerState)SamplerDescriptorHeap[index], samplers[otherIndex % 4]);
    return singleTexture.SampleLevel(selected, uv, 0);
}
