//TEST:SIMPLE(filecheck=WARNING): -target spirv-asm -stage fragment -entry implicitSample -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WARNING-GLSL): -target glsl -stage fragment -entry implicitSample -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WARNING): -target spirv-asm -stage fragment -entry implicitLod -capability texture_querylod -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WARNING-GLSL): -target glsl -stage fragment -entry implicitLod -capability texture_querylod -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SILENT): -target spirv-asm -stage fragment -entry implicitSample -Wno-56020 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=EXPLICIT): -target spirv-asm -stage fragment -entry explicitSample -O0 -warnings-as-errors 56020 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=EXPLICIT-GLSL): -target glsl -stage fragment -entry explicitSample -warnings-as-errors 56020 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SAME-SPV): -target spirv-asm -stage fragment -entry sameArray -warnings-as-errors 56020 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SAME-GLSL): -target glsl -stage fragment -entry sameArray -warnings-as-errors 56020 -validate-ir -validate-ir-detailed

// Moving a resource operation into conditional arms can make implicit derivatives undefined.
// We require one warning per original operation, even when both its texture and sampler are
// selected. Explicit gradients and LOD need no derivative warning. Selecting an index within
// one array needs no new control flow and therefore must not produce either warning about
// conditional execution.
// The GLSL library expresses these operations with textual intrinsics, so the conservative
// classification produces the warning about unknown control-flow requirements instead.
// WARNING-NOT: warning[E56021]
// WARNING: warning[E56020]: resource selection introduces conditional execution
// WARNING-NOT: warning[E56020]
// WARNING-NOT: warning[E56021]
// WARNING-GLSL-NOT: warning[E56020]
// WARNING-GLSL: warning[E56021]: resource selection introduces conditional execution
// WARNING-GLSL-NOT: warning[E56021]
// WARNING-GLSL-NOT: warning[E56020]
// SILENT-NOT: warning[E56020]
// SILENT-NOT: warning[E56021]
// EXPLICIT-NOT: warning[E56020]
// EXPLICIT-NOT: warning[E56021]
// EXPLICIT: %[[GX:[^ ]+]] = OpFunctionCall %v2float %ddx{{( |$)}}
// EXPLICIT: %[[GY:[^ ]+]] = OpFunctionCall %v2float %ddy{{( |$)}}
// EXPLICIT-NOT: OpFunctionCall %v2float %{{(ddx|ddy)}}
// EXPLICIT: OpBranchConditional
// EXPLICIT-NOT: OpFunctionCall %v2float %{{(ddx|ddy)}}
// EXPLICIT: OpImageSampleExplicitLod {{[^ ]+}} {{[^ ]+}} {{[^ ]+}} Grad %[[GX]] %[[GY]]{{( |$)}}
// EXPLICIT-NOT: OpFunctionCall %v2float %{{(ddx|ddy)}}
// EXPLICIT: OpImageSampleExplicitLod {{[^ ]+}} {{[^ ]+}} {{[^ ]+}} Grad %[[GX]] %[[GY]]{{( |$)}}
// EXPLICIT-NOT: OpFunctionCall %v2float %{{(ddx|ddy)}}
// EXPLICIT-GLSL-NOT: warning[E56020]
// EXPLICIT-GLSL: vec2 [[GX:[^ ]+]] = dFdx(
// EXPLICIT-GLSL: vec2 [[GY:[^ ]+]] = dFdy(
// EXPLICIT-GLSL-NOT: {{(dFdx|dFdy)}}(
// EXPLICIT-GLSL: if(
// EXPLICIT-GLSL-NOT: {{(dFdx|dFdy)}}(
// EXPLICIT-GLSL: textureGrad({{.*}}, ([[GX]]), ([[GY]]))
// EXPLICIT-GLSL-NOT: {{(dFdx|dFdy)}}(
// EXPLICIT-GLSL: textureGrad({{.*}}, ([[GX]]), ([[GY]]))
// EXPLICIT-GLSL-NOT: {{(dFdx|dFdy)}}(
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-SPV: %[[UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-SPV: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-SPV: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool %[[X]] %float_0_5{{( |$)}}
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-SPV: %[[INDEX:[^ ]+]] = OpSelect %int %[[CONDITION]] %int_0 %int_1{{( |$)}}
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-SPV: %[[POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplerArray %[[INDEX]]{{( |$)}}
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-SPV: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[POINTER]]{{( |$)}}
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-SPV: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-SPV: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE]] %[[SAMPLER]]{{( |$)}}
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-SPV: OpImageSampleImplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
// SAME-SPV-NOT: {{warning\[E5602[01]\]|Op(Branch|Switch)}}
// SAME-GLSL-NOT: {{warning\[E5602[01]\]|if\(|switch\(}}
// SAME-GLSL-DAG: uniform texture2D [[IMAGE:textureA[^; ]*]];
// SAME-GLSL-DAG: uniform sampler {{ *}}[[SAMPLERS:samplerArray[A-Za-z0-9_]*]][2];
// SAME-GLSL-DAG: in vec2 [[UV:uv[^; ]*]];
// SAME-GLSL-NOT: {{warning\[E5602[01]\]|if\(|switch\(}}
// SAME-GLSL: texture(sampler2D([[IMAGE]],[[SAMPLERS]][nonuniformEXT(([[UV]].x) > 0.5 ? 0 : 1)]),
// SAME-GLSL-NOT: {{warning\[E5602[01]\]|if\(|switch\(}}

Texture2D<float4> textureA;
Texture2D<float4> textureB;
SamplerState samplerA;
SamplerState samplerB;
SamplerState samplerArray[2];

float4 implicitSample(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    SamplerState sampler = select(uv.y > 0.5, samplerA, samplerB);
    return texture.Sample(sampler, uv);
}

float4 implicitLod(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    return texture.CalculateLevelOfDetail(samplerA, uv);
}

// We calculate gradients before sampling so each cloned `SampleGrad` operation can use the
// same already-computed gradients without introducing a conditional derivative calculation.
float4 explicitSample(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    float2 gradientX = ddx(uv);
    float2 gradientY = ddy(uv);
    return texture.SampleLevel(samplerA, uv, 0)
        + texture.SampleGrad(samplerA, uv, gradientX, gradientY)
        + texture.Load(int3(0));
}

float4 sameArray(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState sampler = select(uv.x > 0.5, samplerArray[0], samplerArray[1]);
    return textureA.Sample(sampler, uv);
}
