// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=HLSL): -O0 -target hlsl -entry main -profile ps_6_6 -validate-ir
//TEST:SIMPLE(filecheck=NOCALL): -O0 -target hlsl -entry main -profile ps_6_6 -validate-ir

// D3D resource specialization inlines `loadTextureAtCentroid` when its texture argument is obtained
// from the descriptor heap. We use no mutable resource global, so the test also exercises modules
// that do not require resource-global replacement.
// After inlining, varying-reference resolution must replace the helper's reference with the
// entry-point varying input.
// NOCALL-NOT: loadTextureAtCentroid
// HLSL: float4 main(float3 [[COLOR:color_[0-9]+]] : COLOR)
// HLSL-DAG: EvaluateAttributeAtCentroid([[COLOR]])
// HLSL-DAG: ResourceDescriptorHeap[NonUniformResourceIndex

uniform uint textureIndex;

// We pass the color by reference so interpolation can use the original fragment varying input.
[noinline]
float4 loadTextureAtCentroid(__constref float3 color, Texture2D<float4> texture)
{
    float3 centroidColor = EvaluateAttributeAtCentroid(color);
    return texture.Load(int3(0, 0, 0)) * float4(centroidColor, 1.0);
}

// The resource read forces late inlining of the helper that also contains the interpolation.
[shader("fragment")]
float4 main(float3 color : COLOR) : SV_Target
{
    Texture2D<float4> selectedTexture =
        ResourceDescriptorHeap[NonUniformResourceIndex(textureIndex)];
    return loadTextureAtCentroid(color, selectedTexture);
}
