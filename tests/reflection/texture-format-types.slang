// Tests reflection of explicit texel format descriptor types and legacy inferred formats.

//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry main -target hlsl -no-codegen
//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry main -target spirv -no-codegen
//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry main -target metal -no-codegen

RWTexture2D<RGBA16F> explicitRGBA16F;
RWTexture2D<RG32F> explicitRG32F;
RWTexture2D<RGBA8UI> explicitRGBA8UI;

RWTexture2D<float4> autoFloat4;
RWTexture2D<int4> autoInt4;
RWTexture2D<uint4> autoUInt4;

// REFLECT-LABEL: "name": "explicitRGBA16F"
// REFLECT: "format": "rgba16f"
// REFLECT: "elementCount": 4
// REFLECT: "scalarType": "float32"

// REFLECT-LABEL: "name": "explicitRG32F"
// REFLECT: "format": "rg32f"
// REFLECT: "elementCount": 2
// REFLECT: "scalarType": "float32"

// REFLECT-LABEL: "name": "explicitRGBA8UI"
// REFLECT: "format": "rgba8ui"
// REFLECT: "elementCount": 4
// REFLECT: "scalarType": "uint32"

// REFLECT-LABEL: "name": "autoFloat4"
// REFLECT: "format": "rgba32f"
// REFLECT: "elementCount": 4
// REFLECT: "scalarType": "float32"

// REFLECT-LABEL: "name": "autoInt4"
// REFLECT: "format": "rgba32i"
// REFLECT: "elementCount": 4
// REFLECT: "scalarType": "int32"

// REFLECT-LABEL: "name": "autoUInt4"
// REFLECT: "format": "rgba32ui"
// REFLECT: "elementCount": 4
// REFLECT: "scalarType": "uint32"

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
