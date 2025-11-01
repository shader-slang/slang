//TEST:SIMPLE(filecheck=HLSL): -allow-glsl -stage compute  -entry computeMain -target hlsl -DHLSL
//TEST:SIMPLE(filecheck=HLSL): -allow-glsl -stage fragment -entry fragMain -target hlsl -DHLSL

// In GLSL, the transformation to Slang and back shouldn't change GLSL target
//TEST:SIMPLE(filecheck=GLSL-NOGRADS):  -allow-glsl -stage compute -entry computeMain -target glsl -DGLSL -DNO_GRADS
//TEST:SIMPLE(filecheck=GLSL-GRADS):    -allow-glsl -stage compute -entry computeMain -target glsl -DGLSL -capability GL_NV_compute_shader_derivatives
//TEST:SIMPLE(filecheck=GLSL-NOGRADS):  -allow-glsl -stage vertex -entry vertexMain -target glsl -DGLSL -DVERTEX_STAGE -DNO_GRADS
//TEST:SIMPLE(filecheck=GLSL-GRADS):    -allow-glsl -stage fragment -entry fragMain -target glsl -DGLSL

// Sampling gradients available. Implicit LOD sampling translates to OpImageSampleImplicitLod and similar
//TEST:SIMPLE(filecheck=SPIR-GRADS):  -allow-glsl -stage fragment -entry fragMain -target spirv -DSPIRV
//TEST:SIMPLE(filecheck=SPIR-GRADS):  -allow-glsl -stage compute -entry computeMain -target spirv -DSPIRV -capability SPV_KHR_compute_shader_derivatives

// sampling gradients not available. Implicit LOD sampling translates to OpImageSampleExplicitLod with Lod=0 and similar
//TEST:SIMPLE(filecheck=SPIR-NOGRADS):  -allow-glsl -stage vertex -entry vertexMain -target spirv -DSPIRV -DVERTEX_STAGE -DNO_GRADS

//TEST:SIMPLE(filecheck=CUDA): -allow-glsl -stage compute  -entry computeMain -target cuda -DCUDA
//TEST:SIMPLE(filecheck=CUDA): -allow-glsl -stage fragment -entry fragMain -target cuda -DCUDA

//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -output-using-type -xslang -DSPIRV

// TODO: This test revealed a few problems for a path from GLSL to HLSL
//T-EST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -compute -entry computeMain -allow-glsl -output-using-type -profile cs_6_6 -xslang -DHLSL


//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    int result;
} outputBuffer;

// float

//TEST_INPUT: TextureSampler1D(size=4, content = zero):name uniform_sampler1DShadow
uniform sampler1DShadow uniform_sampler1DShadow;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_sampler2DShadow
uniform sampler2DShadow uniform_sampler2DShadow;

//TEST_INPUT: TextureSamplerCube(size=4, content = zero):name uniform_samplerCubeShadow
uniform samplerCubeShadow uniform_samplerCubeShadow;


//TEST_INPUT: TextureSampler1D(size=4, content = zero, arrayLength=2):name uniform_sampler1DArrayShadow
uniform sampler1DArrayShadow uniform_sampler1DArrayShadow;

//TEST_INPUT: TextureSampler2D(size=4, content = zero, arrayLength=2):name uniform_sampler2DArrayShadow
uniform sampler2DArrayShadow uniform_sampler2DArrayShadow;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_sampler2DRectShadow
uniform sampler2DRectShadow uniform_sampler2DRectShadow;

//TEST_INPUT: TextureSamplerCube(size=4, content = zero, arrayLength=2):name uniform_samplerCubeArrayShadow
uniform samplerCubeArrayShadow uniform_samplerCubeArrayShadow;


//TEST_INPUT: TextureSampler1D(size=4, content = zero):name uniform_sampler1D
uniform sampler1D uniform_sampler1D;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_sampler2D
uniform sampler2D uniform_sampler2D;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_sampler2DRect
uniform sampler2DRect uniform_sampler2DRect;

//TEST_INPUT: TextureSampler3D(size=4, content = zero):name uniform_sampler3D
uniform sampler3D uniform_sampler3D;

//TEST_INPUT: TextureSamplerCube(size=4, content = zero):name uniform_samplerCube
uniform samplerCube uniform_samplerCube;

//TEST_INPUT: TextureSampler1D(size=4, content = zero, arrayLength=2):name uniform_sampler1DArray
uniform sampler1DArray uniform_sampler1DArray;

//TEST_INPUT: TextureSampler2D(size=4, content = zero, arrayLength=2):name uniform_sampler2DArray
uniform sampler2DArray uniform_sampler2DArray;

//TEST_INPUT: TextureSamplerCube(size=4, content = zero, arrayLength=2):name uniform_samplerCubeArray
uniform samplerCubeArray uniform_samplerCubeArray;

//TEST_INPUT: TextureSampler1D(size=4, content = zero):name uniform_samplerBuffer
uniform samplerBuffer uniform_samplerBuffer;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_sampler2DMS
uniform sampler2DMS uniform_sampler2DMS;

//TEST_INPUT: TextureSampler2D(size=4, content = zero, arrayLength=2):name uniform_sampler2DMSArray
uniform sampler2DMSArray uniform_sampler2DMSArray;

// int

//TEST_INPUT: TextureSampler1D(size=4, content = zero):name uniform_isampler1D
uniform isampler1D uniform_isampler1D;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_isampler2D
uniform isampler2D uniform_isampler2D;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_isampler2DRect
uniform isampler2DRect uniform_isampler2DRect;

//TEST_INPUT: TextureSampler3D(size=4, content = zero):name uniform_isampler3D
uniform isampler3D uniform_isampler3D;

//TEST_INPUT: TextureSamplerCube(size=4, content = zero):name uniform_isamplerCube
uniform isamplerCube uniform_isamplerCube;

//TEST_INPUT: TextureSampler1D(size=4, content = zero):name uniform_isampler1DArray
uniform isampler1DArray uniform_isampler1DArray;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_isampler2DArray
uniform isampler2DArray uniform_isampler2DArray;

//TEST_INPUT: TextureSamplerCube(size=4, content = zero):name uniform_isamplerCubeArray
uniform isamplerCubeArray uniform_isamplerCubeArray;

//TEST_INPUT: TextureSampler1D(size=4, content = zero):name uniform_isamplerBuffer
uniform isamplerBuffer uniform_isamplerBuffer;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_isampler2DMS
uniform isampler2DMS uniform_isampler2DMS;

//TEST_INPUT: TextureSampler2D(size=4, content = zero, arrayLength = 2):name uniform_isampler2DMSArray
uniform isampler2DMSArray uniform_isampler2DMSArray;

// uint

//TEST_INPUT: TextureSampler1D(size=4, content = zero):name uniform_usampler1D
uniform usampler1D uniform_usampler1D;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_usampler2D
uniform usampler2D uniform_usampler2D;

//TEST_INPUT: TextureSampler2D(size=4, content = zero):name uniform_usampler2DRect
uniform usampler2DRect uniform_usampler2DRect;

//TEST_INPUT: TextureSampler3D(size=4, content = zero):name uniform_usampler3D
uniform usampler3D uniform_usampler3D;

//TEST_INPUT: TextureSamplerCube(size=4, content = zero):name uniform_usamplerCube
uniform usamplerCube uniform_usamplerCube;

//TEST_INPUT: TextureSampler1D(size=4, content = zero, arrayLength = 2):name uniform_usampler1DArray
uniform usampler1DArray uniform_usampler1DArray;

//TEST_INPUT: TextureSampler2D(size=4, content = zero, arrayLength = 2):name uniform_usampler2DArray
uniform usampler2DArray uniform_usampler2DArray;

//TEST_INPUT: TextureSamplerCube(size=4, content = zero, arrayLength = 2):name uniform_usamplerCubeArray
uniform usamplerCubeArray uniform_usamplerCubeArray;

//TEST_INPUT: Texture1D(size=4, content = zero):name uniform_usamplerBuffer
uniform usamplerBuffer uniform_usamplerBuffer;

//TEST_INPUT: Texture2D(size=4, content = zero):name uniform_usampler2DMS
uniform usampler2DMS uniform_usampler2DMS;

//TEST_INPUT: Texture2D(size=4, content = zero, arrayLength = 2):name uniform_usampler2DMSArray
uniform usampler2DMSArray uniform_usampler2DMSArray;

__generic<T : ITexelElement>
bool textureFuncs( Sampler1D<T> gsampler1D
    , Sampler2D<T> gsampler2D
    , Sampler2DRect<T> gsampler2DRect
    , Sampler3D<T> gsampler3D
    , SamplerCube<T> gsamplerCube
    , Sampler1DArray<T> gsampler1DArray
    , Sampler2DArray<T> gsampler2DArray
    , SamplerCubeArray<T> gsamplerCubeArray
    , SamplerBuffer<T> gsamplerBuffer
    , Sampler2DMS<T> gsampler2DMS
    , Sampler2DMSArray<T> gsampler2DMSArray
    , bool ignoreResult
)
{
    // GLSL-GRADS-LABEL:    textureFuncs_0
    // GLSL-NOGRADS-LABEL:  textureFuncs_0
    typealias gvec4 = vector<T.Element,4>;

    constexpr float coord = 0.5;

    constexpr ivec2 offset2D = ivec2(2);
    constexpr ivec3 offset3D = ivec3(3);
    constexpr ivec2 offsets[4] = { ivec2(1), ivec2(2), ivec2(3), ivec2(4) };

    bool ignoreResultF32 = ignoreResult && T.Element is float;
    bool ignoreResultI32 = ignoreResult && T.Element is int32_t;

    return true
        // 8.9.1. Texture Query Functions

        // GLSL-GRADS:    textureSize({{.*}}sampler1D
        // GLSL-NOGRADS:  textureSize({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && int(4) == textureSize(gsampler1D, int(0))

        // GLSL-GRADS:    textureSize({{.*}}sampler2D
        // GLSL-NOGRADS:  textureSize({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(gsampler2D, int(0))

        // GLSL-GRADS:    textureSize({{.*}}sampler3D
        // GLSL-NOGRADS:  textureSize({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec3(4) == textureSize(gsampler3D, int(0))

        // GLSL-GRADS:    textureSize({{.*}}samplerCube
        // GLSL-NOGRADS:  textureSize({{.*}}samplerCube
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(gsamplerCube, int(0))

        // GLSL-GRADS:    textureSize({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureSize({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && int(4) == textureSize(uniform_sampler1DShadow, int(0))

        // GLSL-GRADS:    textureSize({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureSize({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec2(4) == textureSize(uniform_sampler2DShadow, int(0)) || ignoreResult)

        // GLSL-GRADS:    textureSize({{.*}}samplerCubeShadow
        // GLSL-NOGRADS:  textureSize({{.*}}samplerCubeShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec2(4) == textureSize(uniform_samplerCubeShadow, int(0)) || ignoreResult)

        // GLSL-GRADS:    textureSize({{.*}}samplerCubeArray
        // GLSL-NOGRADS:  textureSize({{.*}}samplerCubeArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec3(4,4,2) == textureSize(gsamplerCubeArray, int(0)) || ignoreResultI32)

        // GLSL-GRADS:    textureSize({{.*}}samplerCubeArrayShadow
        // GLSL-NOGRADS:  textureSize({{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec3(4,4,2) == textureSize(uniform_samplerCubeArrayShadow, int(0))

        // GLSL-GRADS:    textureSize({{.*}}sampler2DRect
        // GLSL-NOGRADS:  textureSize({{.*}}sampler2DRect
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(gsampler2DRect)

        // GLSL-GRADS:    textureSize({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureSize({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(uniform_sampler2DRectShadow)

        // GLSL-GRADS:    textureSize({{.*}}sampler1DArray
        // GLSL-NOGRADS:  textureSize({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec2(4,2) == textureSize(gsampler1DArray, int(0)) || ignoreResultI32 )

        // GLSL-GRADS:    textureSize({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  textureSize({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec2(4,2) == textureSize(uniform_sampler1DArrayShadow, int(0)) || ignoreResult)

        // GLSL-GRADS:    textureSize({{.*}}sampler2DArray
        // GLSL-NOGRADS:  textureSize({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec3(4,4,2) == textureSize(gsampler2DArray, int(0)) || ignoreResultI32)

        // GLSL-GRADS:    textureSize({{.*}}sampler2DArrayShadow
        // GLSL-NOGRADS:  textureSize({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec3(4,4,2) == textureSize(uniform_sampler2DArrayShadow, int(0))

        // GLSL-GRADS:    imageSize({{.*}}samplerBuffer
        // GLSL-NOGRADS:  imageSize({{.*}}samplerBuffer
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerBuffer
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerBuffer
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[LOAD]]
        && (int(4) == textureSize(gsamplerBuffer) || ignoreResult)

        // GLSL-GRADS:    textureSize({{.*}}sampler2DMS
        // GLSL-NOGRADS:  textureSize({{.*}}sampler2DMS
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(gsampler2DMS)

        // GLSL-GRADS:    textureSize({{.*}}sampler2DMSArray
        // GLSL-NOGRADS:  textureSize({{.*}}sampler2DMSArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec3(4,4,2) == textureSize(gsampler2DMSArray)

#if !defined(NO_GRADS)
        // GLSL-GRADS:    textureQueryLod({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler1D, float(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler2D, vec2(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler3D, vec3(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}samplerCube
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsamplerCube, vec3(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler1DArray, float(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler2DArray, vec2(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}samplerCubeArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsamplerCubeArray, vec3(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_sampler1DShadow, float(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_sampler2DShadow, vec2(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}samplerCubeShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_samplerCubeShadow, vec3(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_sampler1DArrayShadow, float(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_sampler2DArrayShadow, vec2(coord)) * vec2(1,0))

        // GLSL-GRADS:    textureQueryLod({{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_samplerCubeArrayShadow, vec3(coord)) * vec2(1,0))
#endif

        // GLSL-GRADS:    textureQueryLevels({{.*}}sampler1D
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler1D)

        // GLSL-GRADS:    textureQueryLevels({{.*}}sampler2D
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler2D)

        // GLSL-GRADS:    textureQueryLevels({{.*}}sampler3D
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler3D)

        // GLSL-GRADS:    textureQueryLevels({{.*}}samplerCube
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}samplerCube
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsamplerCube)

        // GLSL-GRADS:    textureQueryLevels({{.*}}sampler1DArray
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler1DArray)

        // GLSL-GRADS:    textureQueryLevels({{.*}}sampler2DArray
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler2DArray)

        // GLSL-GRADS:    textureQueryLevels({{.*}}samplerCubeArray
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}samplerCubeArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsamplerCubeArray)

        // GLSL-GRADS:    textureQueryLevels({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_sampler1DShadow)

        // GLSL-GRADS:    textureQueryLevels({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_sampler2DShadow)

        // GLSL-GRADS:    textureQueryLevels({{.*}}samplerCubeShadow
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}samplerCubeShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_samplerCubeShadow)

        // GLSL-GRADS:    textureQueryLevels({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_sampler1DArrayShadow)

        // GLSL-GRADS:    textureQueryLevels({{.*}}sampler2DArrayShadow
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_sampler2DArrayShadow)

        // GLSL-GRADS:    textureQueryLevels({{.*}}samplerCubeArrayShadow
        // GLSL-NOGRADS:  textureQueryLevels({{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_samplerCubeArrayShadow)

        // GLSL-GRADS:    textureSamples({{.*}}sampler2DMS
        // GLSL-NOGRADS:  textureSamples({{.*}}sampler2DMS
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySamples{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySamples{{.*}}[[IMAGE]]
        && int(1) == textureSamples(gsampler2DMS)

        // GLSL-GRADS:    textureSamples({{.*}}sampler2DMSArray
        // GLSL-NOGRADS:  textureSamples({{.*}}sampler2DMSArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageQuerySamples{{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageQuerySamples{{.*}}[[IMAGE]]
        && int(1) == textureSamples(gsampler2DMSArray)

        // 8.9.2. Texel Lookup Functions
        // GLSL-GRADS:    texture({{.*}}sampler1D
        // GLSL-NOGRADS:  texture({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == texture(gsampler1D, float(coord))

        // GLSL-GRADS:    texture({{.*}}sampler1D
        // GLSL-NOGRADS:  texture({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}}
        && gvec4(T.Element(0)) == texture(gsampler1D, float(coord), float(0))

        // GLSL-GRADS:    texture({{.*}}sampler2D
        // GLSL-NOGRADS:  texture({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == texture(gsampler2D, vec2(coord))

        // GLSL-GRADS:    texture({{.*}}sampler2D
        // GLSL-NOGRADS:  texture({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}}
        && gvec4(T.Element(0)) == texture(gsampler2D, vec2(coord), float(0))

        // GLSL-GRADS:    texture({{.*}}sampler3D
        // GLSL-NOGRADS:  texture({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == texture(gsampler3D, vec3(coord))

        // GLSL-GRADS:    texture({{.*}}sampler3D
        // GLSL-NOGRADS:  texture({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}}
        && gvec4(T.Element(0)) == texture(gsampler3D, vec3(coord), float(0))

        // GLSL-GRADS:    texture({{.*}}samplerCube
        // GLSL-NOGRADS:  texture({{.*}}samplerCube
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == texture(gsamplerCube, vec3(coord))

        // GLSL-GRADS:    texture({{.*}}samplerCube
        // GLSL-NOGRADS:  texture({{.*}}samplerCube
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}}
        && gvec4(T.Element(0)) == texture(gsamplerCube, vec3(coord), float(0))

        // GLSL-GRADS:    texture({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  texture({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]
        && float(0) == texture(uniform_sampler1DShadow, vec3(coord))

        // GLSL-GRADS:    texture({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  texture({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}}
        && float(0) == texture(uniform_sampler1DShadow, vec3(coord), float(0))

        // GLSL-GRADS:    texture({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  texture({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]
        && (float(0) == texture(uniform_sampler2DShadow, vec3(coord)) || ignoreResultF32)

        // GLSL-GRADS:    texture({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  texture({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}}
        && (float(0) == texture(uniform_sampler2DShadow, vec3(coord), float(0)) || ignoreResultF32)

        // GLSL-GRADS:    texture({{.*}}samplerCubeShadow
        // GLSL-NOGRADS:  texture({{.*}}samplerCubeShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]
        && (float(0) == texture(uniform_samplerCubeShadow, vec4(coord)) || ignoreResultF32)

        // GLSL-GRADS:    texture({{.*}}samplerCubeShadow
        // GLSL-NOGRADS:  texture({{.*}}samplerCubeShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}}
        && (float(0) == texture(uniform_samplerCubeShadow, vec4(coord), float(0)) || ignoreResultF32)

        // GLSL-GRADS:    texture({{.*}}sampler2DArray
        // GLSL-NOGRADS:  texture({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == texture(gsampler2DArray, vec3(coord))

        // GLSL-GRADS:    texture({{.*}}sampler2DArray
        // GLSL-NOGRADS:  texture({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}}
        && gvec4(T.Element(0)) == texture(gsampler2DArray, vec3(coord), float(0))

        // GLSL-GRADS:    texture({{.*}}samplerCubeArray
        // GLSL-NOGRADS:  texture({{.*}}samplerCubeArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == texture(gsamplerCubeArray, vec4(coord))

        // GLSL-GRADS:    texture({{.*}}samplerCubeArray
        // GLSL-NOGRADS:  texture({{.*}}samplerCubeArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}}
        && gvec4(T.Element(0)) == texture(gsamplerCubeArray, vec4(coord), float(0))

        // GLSL-GRADS:    texture({{.*}}sampler1DArray
        // GLSL-NOGRADS:  texture({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == texture(gsampler1DArray, vec2(coord))

        // GLSL-GRADS:    texture({{.*}}sampler1DArray
        // GLSL-NOGRADS:  texture({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}}
        && gvec4(T.Element(0)) == texture(gsampler1DArray, vec2(coord), float(0))

        // GLSL-GRADS:    texture({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  texture({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]
        && float(0) == texture(uniform_sampler1DArrayShadow, vec3(coord))

        // GLSL-GRADS:    texture({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  texture({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}}
        && float(0) == texture(uniform_sampler1DArrayShadow, vec3(coord), float(0))

        // GLSL-GRADS:    texture({{.*}}sampler2DArrayShadow
        // GLSL-NOGRADS:  texture({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]
        && float(0) == texture(uniform_sampler2DArrayShadow, vec4(coord))

        // GLSL-GRADS:    texture({{.*}}sampler2DRect
        // GLSL-NOGRADS:  texture({{.*}}sampler2DRect
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == texture(gsampler2DRect, vec2(coord))

        // GLSL-GRADS:    texture({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  texture({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]
        && float(0) == texture(uniform_sampler2DRectShadow, vec3(coord))

        // GLSL-GRADS:    texture({{.*}}samplerCubeArrayShadow
        // GLSL-NOGRADS:  texture({{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]
        && float(0) == texture(uniform_samplerCubeArrayShadow, vec4(coord), float(0))

        // GLSL-GRADS:    textureProj({{.*}}sampler1D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler1D, vec2(coord))

        // GLSL-GRADS:    textureProj({{.*}}sampler1D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler1D, vec2(coord), float(0))

        // GLSL-GRADS:    textureProj({{.*}}sampler1D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler1D, vec4(coord))

        // GLSL-GRADS:    textureProj({{.*}}sampler1D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler1D, vec4(coord), float(0))

        // GLSL-GRADS:    textureProj({{.*}}sampler2D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler2D, vec3(coord))

        // GLSL-GRADS:    textureProj({{.*}}sampler2D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler2D, vec3(coord), float(0))

        // GLSL-GRADS:    textureProj({{.*}}sampler2D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler2D, vec4(coord))

        // GLSL-GRADS:    textureProj({{.*}}sampler2D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler2D, vec4(coord), float(0))

        // GLSL-GRADS:    textureProj({{.*}}sampler3D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler3D, vec4(coord))

        // GLSL-GRADS:    textureProj({{.*}}sampler3D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler3D, vec4(coord), float(0))

        // GLSL-GRADS:    textureProj({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureProj(uniform_sampler1DShadow, vec4(coord))

        // GLSL-GRADS:    textureProj({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureProj(uniform_sampler1DShadow, vec4(coord), float(0))

        // GLSL-GRADS:    textureProj({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureProj(uniform_sampler2DShadow, vec4(coord)) || ignoreResultF32)

        // GLSL-GRADS:    textureProj({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureProj(uniform_sampler2DShadow, vec4(coord), float(0)) || ignoreResultF32)

        // GLSL-GRADS-COUNT-2:    textureProj({{.*}}sampler2DRect
        // GLSL-NOGRADS-COUNT-2:  textureProjLod({{.*}}sampler2DRect
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureProj(gsampler2DRect, vec3(coord))
        && gvec4(T.Element(0)) == textureProj(gsampler2DRect, vec4(coord))

        // GLSL-GRADS:    textureProj({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureProj(uniform_sampler2DRectShadow, vec4(coord))

        // GLSL-GRADS:    textureLod({{.*}}sampler1D
        // GLSL-NOGRADS:  textureLod({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler1D, float(coord), float(0))

        // GLSL-GRADS:    textureLod({{.*}}sampler2D
        // GLSL-NOGRADS:  textureLod({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler2D, vec2(coord), float(0))

        // GLSL-GRADS:    textureLod({{.*}}sampler3D
        // GLSL-NOGRADS:  textureLod({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler3D, vec3(coord), float(0))

        // GLSL-GRADS:    textureLod({{.*}}samplerCube
        // GLSL-NOGRADS:  textureLod({{.*}}samplerCube
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsamplerCube, vec3(coord), float(0))

        // GLSL-GRADS:    textureLod({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureLod({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureLod(uniform_sampler2DShadow, vec3(coord), float(0)) || ignoreResultF32)

        // GLSL-GRADS:    textureLod({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureLod({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureLod(uniform_sampler1DShadow, vec3(coord), float(0))

        // GLSL-GRADS:    textureLod({{.*}}sampler1DArray
        // GLSL-NOGRADS:  textureLod({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler1DArray, vec2(coord), float(0))

        // GLSL-GRADS:    textureLod({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  textureLod({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureLod(uniform_sampler1DArrayShadow, vec3(coord), float(0))

        // GLSL-GRADS:    textureLod({{.*}}sampler2DArray
        // GLSL-NOGRADS:  textureLod({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler2DArray, vec3(coord), float(0))

        // GLSL-GRADS:    textureLod({{.*}}samplerCubeArray
        // GLSL-NOGRADS:  textureLod({{.*}}samplerCubeArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsamplerCubeArray, vec4(coord), float(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler1D
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler1D, float(coord), int(1))

        // GLSL-GRADS:    textureOffset({{.*}}sampler1D
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler1D, float(coord), int(0), float(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler2D
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2D, vec2(coord), offset2D)

        // GLSL-GRADS:    textureOffset({{.*}}sampler2D
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2D, vec2(coord), offset2D, float(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler3D
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler3D, vec3(coord), offset3D)

        // GLSL-GRADS:    textureOffset({{.*}}sampler3D
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler3D, vec3(coord), offset3D, float(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureOffset(uniform_sampler2DShadow, vec3(coord), offset2D) || ignoreResultF32)

        // GLSL-GRADS:    textureOffset({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureOffset(uniform_sampler2DShadow, vec3(coord), offset2D, float(0)) || ignoreResultF32)

        // GLSL-GRADS:    textureOffset({{.*}}sampler2DRect
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler2DRect
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2DRect, vec2(coord), offset2D)

        // GLSL-GRADS:    textureOffset({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        && float(0) == textureOffset(uniform_sampler2DRectShadow, vec3(coord), offset2D)

        // GLSL-GRADS:    textureOffset({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler1DShadow, vec3(coord), int(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler1DShadow, vec3(coord), int(0), float(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler1DArray
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler1DArray, vec2(coord), int(1))

        // GLSL-GRADS:    textureOffset({{.*}}sampler1DArray
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler1DArray, vec2(coord), int(0), float(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler2DArray
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}}ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2DArray, vec3(coord), offset2D)

        // GLSL-GRADS:    textureOffset({{.*}}sampler2DArray
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2DArray, vec3(coord), offset2D, float(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler1DArrayShadow, vec3(coord), int(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler1DArrayShadow, vec3(coord), int(0), float(0))

        // GLSL-GRADS:    textureOffset({{.*}}sampler2DArrayShadow
        // GLSL-NOGRADS:  textureOffset({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler2DArrayShadow, vec4(coord), offset2D)

        // GLSL-GRADS:    texelFetch({{.*}}sampler1D
        // GLSL-NOGRADS:  texelFetch({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler1D, int(coord), int(0))

        // GLSL-GRADS:    texelFetch({{.*}}sampler2D
        // GLSL-NOGRADS:  texelFetch({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler2D, ivec2(coord), int(0))

        // GLSL-GRADS:    texelFetch({{.*}}sampler3D
        // GLSL-NOGRADS:  texelFetch({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler3D, ivec3(coord), int(0))

        // GLSL-GRADS:    texelFetch({{.*}}sampler2DRect
        // GLSL-NOGRADS:  texelFetch({{.*}}sampler2DRect
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]
        && gvec4(T.Element(0)) == texelFetch(gsampler2DRect, ivec2(coord))

        // GLSL-GRADS:    texelFetch({{.*}}sampler1DArray
        // GLSL-NOGRADS:  texelFetch({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler1DArray, ivec2(coord), int(0))

        // GLSL-GRADS:    texelFetch({{.*}}sampler2DArray
        // GLSL-NOGRADS:  texelFetch({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler2DArray, ivec3(coord), int(0))

        // GLSL-GRADS:    imageLoad({{.*}}samplerBuffer
        // GLSL-NOGRADS:  imageLoad({{.*}}samplerBuffer
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerBuffer
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerBuffer
        // SPIR-GRADS:    OpImageRead {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageRead {{.*}}[[LOAD]]
        && (gvec4(T.Element(0)) == texelFetch(gsamplerBuffer, int(coord)) || ignoreResultF32)

        // GLSL-GRADS:    texelFetch({{.*}}sampler2DMS
        // GLSL-NOGRADS:  texelFetch({{.*}}sampler2DMS
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // S-PIR: OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler2DMS, ivec2(coord), int(0))

        // GLSL-GRADS:    texelFetch({{.*}}sampler2DMSArray
        // GLSL-NOGRADS:  texelFetch({{.*}}sampler2DMSArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // S-PIR: OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler2DMSArray, ivec3(coord), int(0))

        // GLSL-GRADS:    texelFetchOffset({{.*}}sampler1D
        // GLSL-NOGRADS:  texelFetchOffset({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler1D, int(coord), int(0), int(0))

        // GLSL-GRADS:    texelFetchOffset({{.*}}sampler2D
        // GLSL-NOGRADS:  texelFetchOffset({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler2D, ivec2(coord), int(0), offset2D)

        // GLSL-GRADS:    texelFetchOffset({{.*}}sampler3D
        // GLSL-NOGRADS:  texelFetchOffset({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler3D, ivec3(coord), int(0), offset3D)

        // GLSL-GRADS:    texelFetchOffset({{.*}}sampler2DRect
        // GLSL-NOGRADS:  texelFetchOffset({{.*}}sampler2DRect
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler2DRect, ivec2(coord), offset2D)

        // GLSL-GRADS:    texelFetchOffset({{.*}}sampler1DArray
        // GLSL-NOGRADS:  texelFetchOffset({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler1DArray, ivec2(coord), int(0), int(0))

        // GLSL-GRADS:    texelFetchOffset({{.*}}sampler2DArray
        // GLSL-NOGRADS:  texelFetchOffset({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR-GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler2DArray, ivec3(coord), int(0), offset2D)

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler1D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler1D, vec2(coord), int(0))

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler1D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler1D, vec2(coord), int(0), float(0))

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler1D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler1D, vec4(coord), int(0))

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler1D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler1D, vec4(coord), int(0), float(0))

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler2D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2D, vec3(coord), offset2D)

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler2D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2D, vec3(coord), offset2D, float(0))

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler2D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2D, vec4(coord), offset2D)

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler2D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2D, vec4(coord), offset2D, float(0))

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler3D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler3D, vec4(coord), offset3D)

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler3D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler3D, vec4(coord), offset3D, float(0))

        // GLSL-GRADS-COUNT-2:    textureProjOffset({{.*}}sampler2DRect
        // GLSL-NOGRADS-COUNT-2:  textureProjLodOffset({{.*}}sampler2DRect
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2DRect, vec3(coord), offset2D)

        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2DRect, vec4(coord), offset2D)

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureProjOffset(uniform_sampler2DRectShadow, vec4(coord), offset2D)

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureProjOffset(uniform_sampler1DShadow, vec4(coord), int(0))

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureProjOffset(uniform_sampler1DShadow, vec4(coord), int(0), float(0))

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureProjOffset(uniform_sampler2DShadow, vec4(coord), offset2D) || ignoreResultF32)

        // GLSL-GRADS:    textureProjOffset({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureProjOffset(uniform_sampler2DShadow, vec4(coord), offset2D, float(0)) || ignoreResultF32)

        // GLSL-GRADS:    textureLodOffset({{.*}}sampler1D
        // GLSL-NOGRADS:  textureLodOffset({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler1D, float(coord), float(0), 0)

        // GLSL-GRADS:    textureLodOffset({{.*}}sampler2D
        // GLSL-NOGRADS:  textureLodOffset({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler2D, vec2(coord), float(0), offset2D)

        // GLSL-GRADS:    textureLodOffset({{.*}}sampler3D
        // GLSL-NOGRADS:  textureLodOffset({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler3D, vec3(coord), float(0), offset3D)

        // GLSL-GRADS:    textureLodOffset({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureLodOffset({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureLodOffset(uniform_sampler1DShadow, vec3(coord), float(0), 0)

        // GLSL-GRADS:    textureLodOffset({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureLodOffset({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureLodOffset(uniform_sampler2DShadow, vec3(coord), float(0), offset2D) || ignoreResultF32)

        // GLSL-GRADS:    textureLodOffset({{.*}}sampler1DArray
        // GLSL-NOGRADS:  textureLodOffset({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler1DArray, vec2(coord), float(0), 0)

        // GLSL-GRADS:    textureLodOffset({{.*}}sampler2DArray
        // GLSL-NOGRADS:  textureLodOffset({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler2DArray, vec3(coord), float(0), offset2D)

        // GLSL-GRADS:    textureLodOffset({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  textureLodOffset({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureLodOffset(uniform_sampler1DArrayShadow, vec3(coord), float(0), int(0))

        // GLSL-GRADS-COUNT-2:    textureProjLod({{.*}}sampler1D
        // GLSL-NOGRADS-COUNT-2:  textureProjLod({{.*}}sampler1D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProjLod(gsampler1D, vec2(coord), float(0))
        && gvec4(T.Element(0)) == textureProjLod(gsampler1D, vec4(coord), float(0))

        // GLSL-GRADS-COUNT-2:    textureProjLod({{.*}}sampler2D
        // GLSL-NOGRADS-COUNT-2:  textureProjLod({{.*}}sampler2D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProjLod(gsampler2D, vec3(coord), float(0))
        && gvec4(T.Element(0)) == textureProjLod(gsampler2D, vec4(coord), float(0))

        // GLSL-GRADS:    textureProjLod({{.*}}sampler3D
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProjLod(gsampler3D, vec4(coord), float(0))

        // GLSL-GRADS:    textureProjLod({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureProjLod(uniform_sampler1DShadow, vec4(coord), float(0))

        // GLSL-GRADS:    textureProjLod({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureProjLod({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureProjLod(uniform_sampler2DShadow, vec4(coord), float(0)) || ignoreResultF32)

        // GLSL-GRADS-COUNT-2:    textureProjLodOffset({{.*}}sampler1D
        // GLSL-NOGRADS-COUNT-2:  textureProjLodOffset({{.*}}sampler1D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler1D, vec2(coord), float(0), 0)
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler1D, vec4(coord), float(0), 0)

        // GLSL-GRADS-COUNT-2:    textureProjLodOffset({{.*}}sampler2D
        // GLSL-NOGRADS-COUNT-2:  textureProjLodOffset({{.*}}sampler2D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler2D, vec3(coord), float(0), offset2D)
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler2D, vec4(coord), float(0), offset2D)

        // GLSL-GRADS:    textureProjLodOffset({{.*}}sampler3D
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler3D, vec4(coord), float(0), offset3D)

        // GLSL-GRADS:    textureProjLodOffset({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureProjLodOffset(uniform_sampler1DShadow, vec4(coord), float(0), int(0))

        // GLSL-GRADS:    textureProjLodOffset({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureProjLodOffset({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureProjLodOffset(uniform_sampler2DShadow, vec4(coord), float(0), offset2D) || ignoreResultF32)

        // GLSL-GRADS:    textureGrad({{.*}}sampler1D
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler1D, float(coord), float(0), float(0))

        // GLSL-GRADS:    textureGrad({{.*}}sampler2D
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler2D, vec2(coord), vec2(0), vec2(0))

        // GLSL-GRADS:    textureGrad({{.*}}sampler3D
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler3D, vec3(coord), vec3(0), vec3(0))

        // GLSL-GRADS:    textureGrad({{.*}}samplerCube
        // GLSL-NOGRADS:  textureGrad({{.*}}samplerCube
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsamplerCube, vec3(coord), vec3(0), vec3(0))

        // GLSL-GRADS:    textureGrad({{.*}}sampler2DRect
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler2DRect
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler2DRect, vec2(coord), vec2(0), vec2(0))

        // GLSL-GRADS:    textureGrad({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureGrad(uniform_sampler2DRectShadow, vec3(coord), vec2(0), vec2(0))

        // GLSL-GRADS:    textureGrad({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureGrad(uniform_sampler1DShadow, vec3(coord), float(0), float(0))

        // GLSL-GRADS:    textureGrad({{.*}}sampler1DArray
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler1DArray, vec2(coord), float(0), float(0))

        // GLSL-GRADS:    textureGrad({{.*}}sampler2DArray
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler2DArray, vec3(coord), vec2(0), vec2(0))

        // GLSL-GRADS:    textureGrad({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureGrad(uniform_sampler1DArrayShadow, vec3(coord), float(0), float(0))

        // GLSL-GRADS:    textureGrad({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && (float(0) == textureGrad(uniform_sampler2DShadow, vec3(coord), vec2(0), vec2(0)) || ignoreResultF32)

        // GLSL-GRADS:    textureGrad({{.*}}samplerCubeShadow
        // GLSL-NOGRADS:  textureGrad({{.*}}samplerCubeShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && (float(0) == textureGrad(uniform_samplerCubeShadow, vec4(coord), vec3(0), vec3(0)) || ignoreResultF32)

        // GLSL-GRADS:    textureGrad({{.*}}sampler2DArrayShadow
        // GLSL-NOGRADS:  textureGrad({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureGrad(uniform_sampler2DArrayShadow, vec4(coord), vec2(0), vec2(0))

        // GLSL-GRADS:    textureGrad({{.*}}samplerCubeArray
        // GLSL-NOGRADS:  textureGrad({{.*}}samplerCubeArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsamplerCubeArray, vec4(coord), vec3(0), vec3(0))

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler1D
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler1D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler1D, float(coord), float(0), float(0), 0)

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler2D
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler2D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler2D, vec2(coord), vec2(0), vec2(0), offset2D)

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler3D
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler3D, vec3(coord), vec3(0), vec3(0), offset3D)

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler2DRect
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler2DRect
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler2DRect, vec2(coord), vec2(0), vec2(0), offset2D)

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureGradOffset(uniform_sampler2DRectShadow, vec3(coord), vec2(0), vec2(0), offset2D)

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureGradOffset(uniform_sampler1DShadow, vec3(coord), float(0), float(0), int(0))

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && (float(0) == textureGradOffset(uniform_sampler2DShadow, vec3(coord), vec2(0), vec2(0), offset2D) || ignoreResultF32)

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler2DArray
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler2DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler2DArray, vec3(coord), vec2(0), vec2(0), offset2D)

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler1DArray
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler1DArray
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR-GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler1DArray, vec2(coord), float(0), float(0), int(0))

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler1DArrayShadow
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureGradOffset(uniform_sampler1DArrayShadow, vec3(coord), float(0), float(0), int(0))

        // GLSL-GRADS:    textureGradOffset({{.*}}sampler2DArrayShadow
        // GLSL-NOGRADS:  textureGradOffset({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureGradOffset(uniform_sampler2DArrayShadow, vec4(coord), vec2(0), vec2(0), offset2D)

        // GLSL-GRADS-COUNT-2:    textureProjGrad({{.*}}sampler1D
        // GLSL-NOGRADS-COUNT-2:  textureProjGrad({{.*}}sampler1D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureProjGrad(gsampler1D, vec2(coord), float(0), float(0))
        && gvec4(T.Element(0)) == textureProjGrad(gsampler1D, vec4(coord), float(0), float(0))

        // GLSL-GRADS-COUNT-2:    textureProjGrad({{.*}}sampler2D
        // GLSL-NOGRADS-COUNT-2:  textureProjGrad({{.*}}sampler2D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureProjGrad(gsampler2D, vec3(coord), vec2(0), vec2(0))
        && gvec4(T.Element(0)) == textureProjGrad(gsampler2D, vec4(coord), vec2(0), vec2(0))

        // GLSL-GRADS:    textureProjGrad({{.*}}sampler3D
        // GLSL-NOGRADS:  textureProjGrad({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureProjGrad(gsampler3D, vec4(coord), vec3(0), vec3(0))

        // GLSL-GRADS-COUNT-2:    textureProjGrad({{.*}}sampler2DRect
        // GLSL-NOGRADS-COUNT-2:  textureProjGrad({{.*}}sampler2DRect
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureProjGrad(gsampler2DRect, vec3(coord), vec2(0), vec2(0))
        && gvec4(T.Element(0)) == textureProjGrad(gsampler2DRect, vec4(coord), vec2(0), vec2(0))

        // GLSL-GRADS:    textureProjGrad({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureProjGrad({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureProjGrad(uniform_sampler2DRectShadow, vec4(coord), vec2(0), vec2(0))

        // GLSL-GRADS:    textureProjGrad({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureProjGrad({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureProjGrad(uniform_sampler1DShadow, vec4(coord), float(0), float(0))

        // GLSL-GRADS:    textureProjGrad({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureProjGrad({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && (float(0) == textureProjGrad(uniform_sampler2DShadow, vec4(coord), vec2(0), vec2(0)) || ignoreResultF32)

        // GLSL-GRADS-COUNT-2:    textureProjGradOffset({{.*}}sampler1D
        // GLSL-NOGRADS-COUNT-2:  textureProjGradOffset({{.*}}sampler1D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler1D, vec2(coord), float(0), float(0), 0)
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler1D, vec4(coord), float(0), float(0), 0)

        // GLSL-GRADS-COUNT-2:    textureProjGradOffset({{.*}}sampler2D
        // GLSL-NOGRADS-COUNT-2:  textureProjGradOffset({{.*}}sampler2D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler2D, vec3(coord), vec2(0), vec2(0), offset2D)
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler2D, vec4(coord), vec2(0), vec2(0), offset2D)

        // GLSL-GRADS:    textureProjGradOffset({{.*}}sampler3D
        // GLSL-NOGRADS:  textureProjGradOffset({{.*}}sampler3D
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler3D, vec4(coord), vec3(0), vec3(0), offset3D)

        // GLSL-GRADS-COUNT-2:    textureProjGradOffset({{.*}}sampler2DRect
        // GLSL-NOGRADS-COUNT-2:  textureProjGradOffset({{.*}}sampler2DRect
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler2DRect, vec3(coord), vec2(0), vec2(0), offset2D)
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler2DRect, vec4(coord), vec2(0), vec2(0), offset2D)

        // GLSL-GRADS:    textureProjGradOffset({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureProjGradOffset({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureProjGradOffset(uniform_sampler2DRectShadow, vec4(coord), vec2(0), vec2(0), offset2D)

        // GLSL-GRADS:    textureProjGradOffset({{.*}}sampler1DShadow
        // GLSL-NOGRADS:  textureProjGradOffset({{.*}}sampler1DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureProjGradOffset(uniform_sampler1DShadow, vec4(coord), float(0), float(0), int(0))

        // GLSL-GRADS:    textureProjGradOffset({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureProjGradOffset({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR-NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && (float(0) == textureProjGradOffset(uniform_sampler2DShadow, vec4(coord), vec2(0), vec2(0), offset2D) || ignoreResultF32)

        // 8.9.4. Texture Gather Functions

        // GLSL-GRADS-COUNT-2:    textureGather({{.*}}sampler2D
        // GLSL-NOGRADS-COUNT-2:  textureGather({{.*}}sampler2D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsampler2D, vec2(coord))
        && gvec4(T.Element(0)) == textureGather(gsampler2D, vec2(coord), int(0))

        // GLSL-GRADS-COUNT-2:    textureGather({{.*}}sampler2DArray
        // GLSL-NOGRADS-COUNT-2:  textureGather({{.*}}sampler2DArray
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsampler2DArray, vec3(coord))
        && gvec4(T.Element(0)) == textureGather(gsampler2DArray, vec3(coord), int(0))

        // GLSL-GRADS-COUNT-2:    textureGather({{.*}}samplerCube
        // GLSL-NOGRADS-COUNT-2:  textureGather({{.*}}samplerCube
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsamplerCube, vec3(coord))
        && gvec4(T.Element(0)) == textureGather(gsamplerCube, vec3(coord), int(0))

        // GLSL-GRADS-COUNT-2:    textureGather({{.*}}samplerCubeArray
        // GLSL-NOGRADS-COUNT-2:  textureGather({{.*}}samplerCubeArray
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsamplerCubeArray, vec4(coord))
        && gvec4(T.Element(0)) == textureGather(gsamplerCubeArray, vec4(coord), int(0))

        // GLSL-GRADS-COUNT-2:    textureGather({{.*}}sampler2DRect
        // GLSL-NOGRADS-COUNT-2:  textureGather({{.*}}sampler2DRect
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsampler2DRect, vec2(coord))
        && gvec4(T.Element(0)) == textureGather(gsampler2DRect, vec2(coord), int(0))

        // GLSL-GRADS:    textureGather({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureGather({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && (vec4(0) == textureGather(uniform_sampler2DShadow, vec2(coord), float(0)) || ignoreResultF32)

        // GLSL-GRADS:    textureGather({{.*}}sampler2DArrayShadow
        // GLSL-NOGRADS:  textureGather({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && vec4(0) == textureGather(uniform_sampler2DArrayShadow, vec3(coord), float(0))

        // GLSL-GRADS:    textureGather({{.*}}samplerCubeShadow
        // GLSL-NOGRADS:  textureGather({{.*}}samplerCubeShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && (vec4(0) == textureGather(uniform_samplerCubeShadow, vec3(coord), float(0)) || ignoreResultF32)

        // GLSL-GRADS:    textureGather({{.*}}samplerCubeArrayShadow
        // GLSL-NOGRADS:  textureGather({{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && vec4(0) == textureGather(uniform_samplerCubeArrayShadow, vec4(coord), float(0))

        // GLSL-GRADS:    textureGather({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureGather({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && vec4(0) == textureGather(uniform_sampler2DRectShadow, vec2(coord), float(0))

        // GLSL-GRADS-COUNT-2:    textureGatherOffset({{.*}}sampler2D
        // GLSL-NOGRADS-COUNT-2:  textureGatherOffset({{.*}}sampler2D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2D, vec2(coord), offset2D)
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2D, vec2(coord), offset2D, int(0))

        // GLSL-GRADS-COUNT-2:    textureGatherOffset({{.*}}sampler2DArray
        // GLSL-NOGRADS-COUNT-2:  textureGatherOffset({{.*}}sampler2DArray
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2DArray, vec3(coord), offset2D)
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2DArray, vec3(coord), offset2D, int(0))

        // GLSL-GRADS:    textureGatherOffset({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureGatherOffset({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && (vec4(0) == textureGatherOffset(uniform_sampler2DShadow, vec2(coord), float(0), offset2D) || ignoreResultF32)

        // GLSL-GRADS:    textureGatherOffset({{.*}}sampler2DArrayShadow
        // GLSL-NOGRADS:  textureGatherOffset({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && vec4(0) == textureGatherOffset(uniform_sampler2DArrayShadow, vec3(coord), float(0), offset2D)

        // GLSL-GRADS-COUNT-2:    textureGatherOffset({{.*}}sampler2DRect
        // GLSL-NOGRADS-COUNT-2:  textureGatherOffset({{.*}}sampler2DRect
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2DRect, vec2(coord), offset2D)
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2DRect, vec2(coord), offset2D, int(0))

        // GLSL-GRADS:    textureGatherOffset({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureGatherOffset({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && vec4(0) == textureGatherOffset(uniform_sampler2DRectShadow, vec2(coord), float(0), offset2D)

        // GLSL-GRADS-COUNT-2:    textureGatherOffsets({{.*}}sampler2D
        // GLSL-NOGRADS-COUNT-2:  textureGatherOffsets({{.*}}sampler2D
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2D, vec2(coord), offsets)
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2D, vec2(coord), offsets, int(0))

        // GLSL-GRADS-COUNT-2:    textureGatherOffsets({{.*}}sampler2DArray
        // GLSL-NOGRADS-COUNT-2:  textureGatherOffsets({{.*}}sampler2DArray
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2DArray, vec3(coord), offsets)
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2DArray, vec3(coord), offsets, int(0))

        // GLSL-GRADS:    textureGatherOffsets({{.*}}sampler2DShadow
        // GLSL-NOGRADS:  textureGatherOffsets({{.*}}sampler2DShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && (vec4(0) == textureGatherOffsets(uniform_sampler2DShadow, vec2(coord), float(0), offsets) || ignoreResultF32)

        // GLSL-GRADS:    textureGatherOffsets({{.*}}sampler2DArrayShadow
        // GLSL-NOGRADS:  textureGatherOffsets({{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && vec4(0) == textureGatherOffsets(uniform_sampler2DArrayShadow, vec3(coord), float(0), offsets)

        // GLSL-GRADS-COUNT-2:    textureGatherOffsets({{.*}}sampler2DRect
        // GLSL-NOGRADS-COUNT-2:  textureGatherOffsets({{.*}}sampler2DRect
        // SPIR-GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR-GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR-NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2DRect, vec2(coord), offsets)
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2DRect, vec2(coord), offsets, int(0))

        // GLSL-GRADS:    textureGatherOffsets({{.*}}sampler2DRectShadow
        // GLSL-NOGRADS:  textureGatherOffsets({{.*}}sampler2DRectShadow
        // SPIR-GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR-GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR-NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && vec4(0) == textureGatherOffsets(uniform_sampler2DRectShadow, vec2(coord), float(0), offsets)

        // 8.9.5. Compatibility Profile Texture Functions
        // Compatibility functions work for GLSL but not for SPIR-V.
        // We cannot test the SPIR-V asm here for these functions.
        // We also cannot test GLSL source for them because we don't directly
        // call them. If we do, the spirv-via-glsl path cannot work.

        && vec4(0) == texture1D(uniform_sampler1D, float(coord))
        && vec4(0) == texture1D(uniform_sampler1D, float(coord), float(0))
        && vec4(0) == texture1DProj(uniform_sampler1D, vec2(coord))
        && vec4(0) == texture1DProj(uniform_sampler1D, vec2(coord), float(0))
        && vec4(0) == texture1DProj(uniform_sampler1D, vec4(coord))
        && vec4(0) == texture1DProj(uniform_sampler1D, vec4(coord), float(0))
        && vec4(0) == texture1DLod(uniform_sampler1D, float(coord), float(0))
        && vec4(0) == texture1DProjLod(uniform_sampler1D, vec2(coord), float(0))
        && vec4(0) == texture1DProjLod(uniform_sampler1D, vec4(coord), float(0))
        && vec4(0) == texture2D(uniform_sampler2D, vec2(coord))
        && vec4(0) == texture2D(uniform_sampler2D, vec2(coord), float(0))
        && vec4(0) == texture2DProj(uniform_sampler2D, vec3(coord))
        && vec4(0) == texture2DProj(uniform_sampler2D, vec3(coord), float(0))
        && vec4(0) == texture2DProj(uniform_sampler2D, vec4(coord))
        && vec4(0) == texture2DProj(uniform_sampler2D, vec4(coord), float(0))
        && vec4(0) == texture2DLod(uniform_sampler2D, vec2(coord), float(0))
        && vec4(0) == texture2DProjLod(uniform_sampler2D, vec3(coord), float(0))
        && vec4(0) == texture2DProjLod(uniform_sampler2D, vec4(coord), float(0))
        && vec4(0) == texture3D(uniform_sampler3D, vec3(coord))
        && vec4(0) == texture3D(uniform_sampler3D, vec3(coord), float(0))
        && vec4(0) == texture3DProj(uniform_sampler3D, vec4(coord))
        && vec4(0) == texture3DProj(uniform_sampler3D, vec4(coord), float(0))
        && vec4(0) == texture3DLod(uniform_sampler3D, vec3(coord), float(0))
        && vec4(0) == texture3DProjLod(uniform_sampler3D, vec4(coord), float(0))
        && vec4(0) == textureCube(uniform_samplerCube, vec3(coord))
        && vec4(0) == textureCube(uniform_samplerCube, vec3(coord), float(0))
        && vec4(0) == textureCubeLod(uniform_samplerCube, vec3(coord), float(0))
        && vec4(0) == shadow1D(uniform_sampler1DShadow, vec3(coord))
        && vec4(0) == shadow1D(uniform_sampler1DShadow, vec3(coord), float(0))
        && (vec4(0) == shadow2D(uniform_sampler2DShadow, vec3(coord)) || ignoreResultF32)
        && (vec4(0) == shadow2D(uniform_sampler2DShadow, vec3(coord), float(0)) || ignoreResultF32)
        && vec4(0) == shadow1DProj(uniform_sampler1DShadow, vec4(coord))
        && vec4(0) == shadow1DProj(uniform_sampler1DShadow, vec4(coord), float(0))
        && (vec4(0) == shadow2DProj(uniform_sampler2DShadow, vec4(coord)) || ignoreResultF32)
        && (vec4(0) == shadow2DProj(uniform_sampler2DShadow, vec4(coord), float(0)) || ignoreResultF32)
        && vec4(0) == shadow1DLod(uniform_sampler1DShadow, vec3(coord), float(0))
        && (vec4(0) == shadow2DLod(uniform_sampler2DShadow, vec3(coord), float(0)) || ignoreResultF32)
        && vec4(0) == shadow1DProjLod(uniform_sampler1DShadow, vec4(coord), float(0))
        && (vec4(0) == shadow2DProjLod(uniform_sampler2DShadow, vec4(coord), float(0)) || ignoreResultF32)

        ;
}

#if defined(HLSL)
[require(hlsl, sm_5_1, compute_fragment)]
#elif defined(GLSL) && !defined(VERTEX_STAGE)
[require(glsl, sm_5_1, compute_fragment)]
#elif defined(GLSL) && defined(VERTEX_STAGE)
[require(glsl, sm_5_1, vertex)]
#elif defined(SPIRV) && !defined(VERTEX_STAGE)
[require(spirv, sm_5_1, compute_fragment)]
#elif defined(SPIRV) && defined(VERTEX_STAGE)
[require(spirv, sm_5_1, vertex)]
#elif defined(CUDA)
[require(cuda, sm_5_1, compute_fragment)]
#endif
__extern_cpp void TEST_main()
{
    // GLSL-GRADS:    void main(
    // GLSL-NOGRADS:  void main(
    // HLSL: void TEST_main(
    // CUDA: void TEST_main(
    // CPP: void _TEST_main(
    // BUF: 1

    // Some of the functional testing result is incorrect due to
    // the problem on our testing framework. We will ignore them
    // until we address the problems.
    // The problem is mostly with 2DShadow, CubeShadow and buffer.
    bool ignoreResult = (outputBuffer.result == 0); // always true

    bool result = true
        && textureFuncs(
            uniform_sampler1D
            , uniform_sampler2D
            , uniform_sampler2DRect
            , uniform_sampler3D
            , uniform_samplerCube
            , uniform_sampler1DArray
            , uniform_sampler2DArray
            , uniform_samplerCubeArray
            , uniform_samplerBuffer
            , uniform_sampler2DMS
            , uniform_sampler2DMSArray
            , ignoreResult
            )
#if !defined(HLSL)
        && textureFuncs(
            uniform_isampler1D
            , uniform_isampler2D
            , uniform_isampler2DRect
            , uniform_isampler3D
            , uniform_isamplerCube
            , uniform_isampler1DArray
            , uniform_isampler2DArray
            , uniform_isamplerCubeArray
            , uniform_isamplerBuffer
            , uniform_isampler2DMS
            , uniform_isampler2DMSArray
            , ignoreResult
            )
        && textureFuncs(
            uniform_usampler1D
            , uniform_usampler2D
            , uniform_usampler2DRect
            , uniform_usampler3D
            , uniform_usamplerCube
            , uniform_usampler1DArray
            , uniform_usampler2DArray
            , uniform_usamplerCubeArray
            , uniform_usamplerBuffer
            , uniform_usampler2DMS
            , uniform_usampler2DMSArray
            , ignoreResult
            )
#endif
        ;

    outputBuffer.result = int(result);
}

[shader("compute")]
[DerivativeGroupLinear]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    TEST_main();
}

[shader("fragment")]
void fragMain()
{
    TEST_main();
}

[shader("vertex")]
void vertexMain()
{
    TEST_main();
}
