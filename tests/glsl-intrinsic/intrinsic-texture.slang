//TEST:SIMPLE(filecheck=HLSL): -allow-glsl -stage compute  -entry computeMain -target hlsl -DHLSL
//TEST:SIMPLE(filecheck=HLSL): -allow-glsl -stage fragment -entry fragMain -target hlsl -DHLSL

// In GLSL, the transformation to Slang and back shouldn't change GLSL target
//TEST:SIMPLE(filecheck=GLSL_NOGRADS):  -allow-glsl -stage compute -entry computeMain -target glsl -DGLSL -DNO_GRADS
//TEST:SIMPLE(filecheck=GLSL_GRADS):    -allow-glsl -stage compute -entry computeMain -target glsl -DGLSL -capability GL_NV_compute_shader_derivatives
//TEST:SIMPLE(filecheck=GLSL_NOGRADS):  -allow-glsl -stage vertex -entry vertexMain -target glsl -DGLSL -DVERTEX_STAGE -DNO_GRADS
//TEST:SIMPLE(filecheck=GLSL_GRADS):    -allow-glsl -stage fragment -entry fragMain -target glsl -DGLSL

// Sampling gradients available. Implicit LOD sampling translates to OpImageSampleImplicitLod and similar
//TEST:SIMPLE(filecheck=SPIR_GRADS):  -allow-glsl -stage fragment -entry fragMain -target spirv -DSPIRV
//TEST:SIMPLE(filecheck=SPIR_GRADS):  -allow-glsl -stage compute -entry computeMain -target spirv -DSPIRV -capability SPV_KHR_compute_shader_derivatives

// sampling gradients not available. Implicit LOD sampling translates to OpImageSampleExplicitLod with Lod=0 and similar
//TEST:SIMPLE(filecheck=SPIR_NOGRADS):  -allow-glsl -stage vertex -entry vertexMain -target spirv -DSPIRV -DVERTEX_STAGE -DNO_GRADS

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
    // GLSL_GRADS-LABEL:    textureFuncs_0
    // GLSL_NOGRADS-LABEL:  textureFuncs_0
    typealias gvec4 = vector<T.Element,4>;

    constexpr float coord = 0.5;

    constexpr ivec2 offset2D = ivec2(2);
    constexpr ivec3 offset3D = ivec3(3);
    constexpr ivec2 offsets[4] = { ivec2(1), ivec2(2), ivec2(3), ivec2(4) };

    bool ignoreResultF32 = ignoreResult && T.Element is float;
    bool ignoreResultI32 = ignoreResult && T.Element is int32_t;

    return true
        // 8.9.1. Texture Query Functions

        // GLSL_GRADS:    textureSize({{.*}}sampler1D
        // GLSL_NOGRADS:  textureSize({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && int(4) == textureSize(gsampler1D, int(0))

        // GLSL_GRADS:    textureSize({{.*}}sampler2D
        // GLSL_NOGRADS:  textureSize({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(gsampler2D, int(0))

        // GLSL_GRADS:    textureSize({{.*}}sampler3D
        // GLSL_NOGRADS:  textureSize({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec3(4) == textureSize(gsampler3D, int(0))

        // GLSL_GRADS:    textureSize({{.*}}samplerCube
        // GLSL_NOGRADS:  textureSize({{.*}}samplerCube
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(gsamplerCube, int(0))

        // GLSL_GRADS:    textureSize({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureSize({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && int(4) == textureSize(uniform_sampler1DShadow, int(0))

        // GLSL_GRADS:    textureSize({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureSize({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec2(4) == textureSize(uniform_sampler2DShadow, int(0)) || ignoreResult)

        // GLSL_GRADS:    textureSize({{.*}}samplerCubeShadow
        // GLSL_NOGRADS:  textureSize({{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec2(4) == textureSize(uniform_samplerCubeShadow, int(0)) || ignoreResult)

        // GLSL_GRADS:    textureSize({{.*}}samplerCubeArray
        // GLSL_NOGRADS:  textureSize({{.*}}samplerCubeArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec3(4,4,2) == textureSize(gsamplerCubeArray, int(0)) || ignoreResultI32)

        // GLSL_GRADS:    textureSize({{.*}}samplerCubeArrayShadow
        // GLSL_NOGRADS:  textureSize({{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec3(4,4,2) == textureSize(uniform_samplerCubeArrayShadow, int(0))

        // GLSL_GRADS:    textureSize({{.*}}sampler2DRect
        // GLSL_NOGRADS:  textureSize({{.*}}sampler2DRect
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(gsampler2DRect)

        // GLSL_GRADS:    textureSize({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureSize({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(uniform_sampler2DRectShadow)

        // GLSL_GRADS:    textureSize({{.*}}sampler1DArray
        // GLSL_NOGRADS:  textureSize({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec2(4,2) == textureSize(gsampler1DArray, int(0)) || ignoreResultI32 )

        // GLSL_GRADS:    textureSize({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  textureSize({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec2(4,2) == textureSize(uniform_sampler1DArrayShadow, int(0)) || ignoreResult)

        // GLSL_GRADS:    textureSize({{.*}}sampler2DArray
        // GLSL_NOGRADS:  textureSize({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && (ivec3(4,4,2) == textureSize(gsampler2DArray, int(0)) || ignoreResultI32)

        // GLSL_GRADS:    textureSize({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  textureSize({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec3(4,4,2) == textureSize(uniform_sampler2DArrayShadow, int(0))

        // GLSL_GRADS:    imageSize({{.*}}samplerBuffer
        // GLSL_NOGRADS:  imageSize({{.*}}samplerBuffer
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerBuffer
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerBuffer
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[LOAD]]
        && (int(4) == textureSize(gsamplerBuffer) || ignoreResult)

        // GLSL_GRADS:    textureSize({{.*}}sampler2DMS
        // GLSL_NOGRADS:  textureSize({{.*}}sampler2DMS
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec2(4) == textureSize(gsampler2DMS)

        // GLSL_GRADS:    textureSize({{.*}}sampler2DMSArray
        // GLSL_NOGRADS:  textureSize({{.*}}sampler2DMSArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySize{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySize{{.*}}[[IMAGE]]
        && ivec3(4,4,2) == textureSize(gsampler2DMSArray)

#if !defined(NO_GRADS)
        // GLSL_GRADS:    textureQueryLod({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler1D, float(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler2D, vec2(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler3D, vec3(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}samplerCube
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsamplerCube, vec3(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler1DArray, float(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsampler2DArray, vec2(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}samplerCubeArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(gsamplerCubeArray, vec3(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_sampler1DShadow, float(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_sampler2DShadow, vec2(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_samplerCubeShadow, vec3(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_sampler1DArrayShadow, float(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_sampler2DArrayShadow, vec2(coord)) * vec2(1,0))

        // GLSL_GRADS:    textureQueryLod({{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    OpImageQueryLod{{.*}}[[LOAD]]
        && vec2(0) == (textureQueryLod(uniform_samplerCubeArrayShadow, vec3(coord)) * vec2(1,0))
#endif

        // GLSL_GRADS:    textureQueryLevels({{.*}}sampler1D
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler1D)

        // GLSL_GRADS:    textureQueryLevels({{.*}}sampler2D
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler2D)

        // GLSL_GRADS:    textureQueryLevels({{.*}}sampler3D
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler3D)

        // GLSL_GRADS:    textureQueryLevels({{.*}}samplerCube
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}samplerCube
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsamplerCube)

        // GLSL_GRADS:    textureQueryLevels({{.*}}sampler1DArray
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler1DArray)

        // GLSL_GRADS:    textureQueryLevels({{.*}}sampler2DArray
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsampler2DArray)

        // GLSL_GRADS:    textureQueryLevels({{.*}}samplerCubeArray
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}samplerCubeArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(gsamplerCubeArray)

        // GLSL_GRADS:    textureQueryLevels({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_sampler1DShadow)

        // GLSL_GRADS:    textureQueryLevels({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_sampler2DShadow)

        // GLSL_GRADS:    textureQueryLevels({{.*}}samplerCubeShadow
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_samplerCubeShadow)

        // GLSL_GRADS:    textureQueryLevels({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_sampler1DArrayShadow)

        // GLSL_GRADS:    textureQueryLevels({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_sampler2DArrayShadow)

        // GLSL_GRADS:    textureQueryLevels({{.*}}samplerCubeArrayShadow
        // GLSL_NOGRADS:  textureQueryLevels({{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQueryLevels{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQueryLevels{{.*}}[[IMAGE]]
        && int(3) == textureQueryLevels(uniform_samplerCubeArrayShadow)

        // GLSL_GRADS:    textureSamples({{.*}}sampler2DMS
        // GLSL_NOGRADS:  textureSamples({{.*}}sampler2DMS
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySamples{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySamples{{.*}}[[IMAGE]]
        && int(1) == textureSamples(gsampler2DMS)

        // GLSL_GRADS:    textureSamples({{.*}}sampler2DMSArray
        // GLSL_NOGRADS:  textureSamples({{.*}}sampler2DMSArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageQuerySamples{{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageQuerySamples{{.*}}[[IMAGE]]
        && int(1) == textureSamples(gsampler2DMSArray)

        // 8.9.2. Texel Lookup Functions
        // GLSL_GRADS:    texture({{.*}}sampler1D
        // GLSL_NOGRADS:  texture({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler1D, float(coord))

        // GLSL_GRADS:    texture({{.*}}sampler1D
        // GLSL_NOGRADS:  texture({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler1D, float(coord), float(0))

        // GLSL_GRADS:    texture({{.*}}sampler2D
        // GLSL_NOGRADS:  texture({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler2D, vec2(coord))

        // GLSL_GRADS:    texture({{.*}}sampler2D
        // GLSL_NOGRADS:  texture({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler2D, vec2(coord), float(0))

        // GLSL_GRADS:    texture({{.*}}sampler3D
        // GLSL_NOGRADS:  texture({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler3D, vec3(coord))

        // GLSL_GRADS:    texture({{.*}}sampler3D
        // GLSL_NOGRADS:  texture({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler3D, vec3(coord), float(0))

        // GLSL_GRADS:    texture({{.*}}samplerCube
        // GLSL_NOGRADS:  texture({{.*}}samplerCube
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsamplerCube, vec3(coord))

        // GLSL_GRADS:    texture({{.*}}samplerCube
        // GLSL_NOGRADS:  texture({{.*}}samplerCube
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsamplerCube, vec3(coord), float(0))

        // GLSL_GRADS:    texture({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  texture({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == texture(uniform_sampler1DShadow, vec3(coord))

        // GLSL_GRADS:    texture({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  texture({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == texture(uniform_sampler1DShadow, vec3(coord), float(0))

        // GLSL_GRADS:    texture({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  texture({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == texture(uniform_sampler2DShadow, vec3(coord)) || ignoreResultF32)

        // GLSL_GRADS:    texture({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  texture({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == texture(uniform_sampler2DShadow, vec3(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    texture({{.*}}samplerCubeShadow
        // GLSL_NOGRADS:  texture({{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == texture(uniform_samplerCubeShadow, vec4(coord)) || ignoreResultF32)

        // GLSL_GRADS:    texture({{.*}}samplerCubeShadow
        // GLSL_NOGRADS:  texture({{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == texture(uniform_samplerCubeShadow, vec4(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    texture({{.*}}samplerCubeArrayShadow
        // GLSL_NOGRADS:  texture({{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == texture(uniform_samplerCubeArrayShadow, vec4(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    texture({{.*}}samplerCubeArrayShadow
        // GLSL_NOGRADS:  texture({{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == texture(uniform_samplerCubeArrayShadow, vec4(coord), float(0), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    texture({{.*}}sampler2DArray
        // GLSL_NOGRADS:  texture({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler2DArray, vec3(coord))

        // GLSL_GRADS:    texture({{.*}}sampler2DArray
        // GLSL_NOGRADS:  texture({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler2DArray, vec3(coord), float(0))

        // GLSL_GRADS:    texture({{.*}}samplerCubeArray
        // GLSL_NOGRADS:  texture({{.*}}samplerCubeArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsamplerCubeArray, vec4(coord))

        // GLSL_GRADS:    texture({{.*}}samplerCubeArray
        // GLSL_NOGRADS:  texture({{.*}}samplerCubeArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsamplerCubeArray, vec4(coord), float(0))

        // GLSL_GRADS:    texture({{.*}}sampler1DArray
        // GLSL_NOGRADS:  texture({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler1DArray, vec2(coord))

        // GLSL_GRADS:    texture({{.*}}sampler1DArray
        // GLSL_NOGRADS:  texture({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler1DArray, vec2(coord), float(0))

        // GLSL_GRADS:    texture({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  texture({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == texture(uniform_sampler1DArrayShadow, vec3(coord))

        // GLSL_GRADS:    texture({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  texture({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == texture(uniform_sampler1DArrayShadow, vec3(coord), float(0))

        // GLSL_GRADS:    texture({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  texture({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == texture(uniform_sampler2DArrayShadow, vec4(coord))

        // GLSL_GRADS:    texture({{.*}}sampler2DRect
        // GLSL_NOGRADS:  texture({{.*}}sampler2DRect
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageSampleImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texture(gsampler2DRect, vec2(coord))

        // GLSL_GRADS:    texture({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  texture({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == texture(uniform_sampler2DRectShadow, vec3(coord))

        // GLSL_GRADS:    texture({{.*}}samplerCubeArrayShadow
        // GLSL_NOGRADS:  texture({{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod{{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == texture(uniform_samplerCubeArrayShadow, vec4(coord), float(0))

        // GLSL_GRADS:    textureProj({{.*}}sampler1D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler1D, vec2(coord))

        // GLSL_GRADS:    textureProj({{.*}}sampler1D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler1D, vec2(coord), float(0))

        // GLSL_GRADS:    textureProj({{.*}}sampler1D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler1D, vec4(coord))

        // GLSL_GRADS:    textureProj({{.*}}sampler1D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler1D, vec4(coord), float(0))

        // GLSL_GRADS:    textureProj({{.*}}sampler2D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler2D, vec3(coord))

        // GLSL_GRADS:    textureProj({{.*}}sampler2D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler2D, vec3(coord), float(0))

        // GLSL_GRADS:    textureProj({{.*}}sampler2D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler2D, vec4(coord))

        // GLSL_GRADS:    textureProj({{.*}}sampler2D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler2D, vec4(coord), float(0))

        // GLSL_GRADS:    textureProj({{.*}}sampler3D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler3D, vec4(coord))

        // GLSL_GRADS:    textureProj({{.*}}sampler3D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler3D, vec4(coord), float(0))

        // GLSL_GRADS:    textureProj({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureProj(uniform_sampler1DShadow, vec4(coord))

        // GLSL_GRADS:    textureProj({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureProj(uniform_sampler1DShadow, vec4(coord), float(0))

        // GLSL_GRADS:    textureProj({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureProj(uniform_sampler2DShadow, vec4(coord)) || ignoreResultF32)

        // GLSL_GRADS:    textureProj({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureProj(uniform_sampler2DShadow, vec4(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS-COUNT-2:    textureProj({{.*}}sampler2DRect
        // GLSL_NOGRADS-COUNT-2:  textureProjLod({{.*}}sampler2DRect
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProj(gsampler2DRect, vec3(coord))
        && gvec4(T.Element(0)) == textureProj(gsampler2DRect, vec4(coord))

        // GLSL_GRADS:    textureProj({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureProj(uniform_sampler2DRectShadow, vec4(coord))

        // GLSL_GRADS:    textureLod({{.*}}sampler1D
        // GLSL_NOGRADS:  textureLod({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler1D, float(coord), float(0))

        // GLSL_GRADS:    textureLod({{.*}}sampler2D
        // GLSL_NOGRADS:  textureLod({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler2D, vec2(coord), float(0))

        // GLSL_GRADS:    textureLod({{.*}}sampler3D
        // GLSL_NOGRADS:  textureLod({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler3D, vec3(coord), float(0))

        // GLSL_GRADS:    textureLod({{.*}}samplerCube
        // GLSL_NOGRADS:  textureLod({{.*}}samplerCube
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsamplerCube, vec3(coord), float(0))

        // GLSL_GRADS:    textureLod({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureLod({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureLod(uniform_sampler2DShadow, vec3(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureLod({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  textureLod({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureLod(uniform_sampler2DArrayShadow, vec4(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureLod({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureLod({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureLod(uniform_sampler1DShadow, vec3(coord), float(0))

        // GLSL_GRADS:    textureLod({{.*}}sampler1DArray
        // GLSL_NOGRADS:  textureLod({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler1DArray, vec2(coord), float(0))

        // GLSL_GRADS:    textureLod({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  textureLod({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureLod(uniform_sampler1DArrayShadow, vec3(coord), float(0))

        // GLSL_GRADS:    textureLod({{.*}}sampler2DArray
        // GLSL_NOGRADS:  textureLod({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsampler2DArray, vec3(coord), float(0))

        // GLSL_GRADS:    textureLod({{.*}}samplerCubeArray
        // GLSL_NOGRADS:  textureLod({{.*}}samplerCubeArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureLod(gsamplerCubeArray, vec4(coord), float(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler1D
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler1D, float(coord), int(1))

        // GLSL_GRADS:    textureLod({{.*}}samplerCubeShadow
        // GLSL_NOGRADS:  textureLod({{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureLod(uniform_samplerCubeShadow, vec4(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureLod({{.*}}samplerCubeArrayShadow
        // GLSL_NOGRADS:  textureLod({{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureLod(uniform_samplerCubeArrayShadow, vec4(coord), float(0), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureOffset({{.*}}sampler1D
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler1D, float(coord), int(0), float(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler2D
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2D, vec2(coord), offset2D)

        // GLSL_GRADS:    textureOffset({{.*}}sampler2D
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2D, vec2(coord), offset2D, float(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler3D
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler3D, vec3(coord), offset3D)

        // GLSL_GRADS:    textureOffset({{.*}}sampler3D
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler3D, vec3(coord), offset3D, float(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureOffset(uniform_sampler2DShadow, vec3(coord), offset2D) || ignoreResultF32)

        // GLSL_GRADS:    textureOffset({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureOffset(uniform_sampler2DShadow, vec3(coord), offset2D, float(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureOffset({{.*}}sampler2DRect
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler2DRect
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2DRect, vec2(coord), offset2D)

        // GLSL_GRADS:    textureOffset({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler2DRectShadow, vec3(coord), offset2D)

        // GLSL_GRADS:    textureOffset({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler1DShadow, vec3(coord), int(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler1DShadow, vec3(coord), int(0), float(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler1DArray
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler1DArray, vec2(coord), int(1))

        // GLSL_GRADS:    textureOffset({{.*}}sampler1DArray
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler1DArray, vec2(coord), int(0), float(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler2DArray
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2DArray, vec3(coord), offset2D)

        // GLSL_GRADS:    textureOffset({{.*}}sampler2DArray
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageSampleImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureOffset(gsampler2DArray, vec3(coord), offset2D, float(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler1DArrayShadow, vec3(coord), int(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler1DArrayShadow, vec3(coord), int(0), float(0))

        // GLSL_GRADS:    textureOffset({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  textureOffset({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureOffset(uniform_sampler2DArrayShadow, vec4(coord), offset2D)

        // GLSL_GRADS:    texelFetch({{.*}}sampler1D
        // GLSL_NOGRADS:  texelFetch({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler1D, int(coord), int(0))

        // GLSL_GRADS:    texelFetch({{.*}}sampler2D
        // GLSL_NOGRADS:  texelFetch({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler2D, ivec2(coord), int(0))

        // GLSL_GRADS:    texelFetch({{.*}}sampler3D
        // GLSL_NOGRADS:  texelFetch({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler3D, ivec3(coord), int(0))

        // GLSL_GRADS:    texelFetch({{.*}}sampler2DRect
        // GLSL_NOGRADS:  texelFetch({{.*}}sampler2DRect
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]
        && gvec4(T.Element(0)) == texelFetch(gsampler2DRect, ivec2(coord))

        // GLSL_GRADS:    texelFetch({{.*}}sampler1DArray
        // GLSL_NOGRADS:  texelFetch({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler1DArray, ivec2(coord), int(0))

        // GLSL_GRADS:    texelFetch({{.*}}sampler2DArray
        // GLSL_NOGRADS:  texelFetch({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler2DArray, ivec3(coord), int(0))

        // GLSL_GRADS:    imageLoad({{.*}}samplerBuffer
        // GLSL_NOGRADS:  imageLoad({{.*}}samplerBuffer
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerBuffer
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerBuffer
        // SPIR_GRADS:    OpImageRead {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageRead {{.*}}[[LOAD]]
        && (gvec4(T.Element(0)) == texelFetch(gsamplerBuffer, int(coord)) || ignoreResultF32)

        // GLSL_GRADS:    texelFetch({{.*}}sampler2DMS
        // GLSL_NOGRADS:  texelFetch({{.*}}sampler2DMS
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMS
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // S-PIR: OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler2DMS, ivec2(coord), int(0))

        // GLSL_GRADS:    texelFetch({{.*}}sampler2DMSArray
        // GLSL_NOGRADS:  texelFetch({{.*}}sampler2DMSArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DMSArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // S-PIR: OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod %
        && gvec4(T.Element(0)) == texelFetch(gsampler2DMSArray, ivec3(coord), int(0))

        // GLSL_GRADS:    texelFetchOffset({{.*}}sampler1D
        // GLSL_NOGRADS:  texelFetchOffset({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler1D, int(coord), int(0), int(0))

        // GLSL_GRADS:    texelFetchOffset({{.*}}sampler2D
        // GLSL_NOGRADS:  texelFetchOffset({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler2D, ivec2(coord), int(0), offset2D)

        // GLSL_GRADS:    texelFetchOffset({{.*}}sampler3D
        // GLSL_NOGRADS:  texelFetchOffset({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler3D, ivec3(coord), int(0), offset3D)

        // GLSL_GRADS:    texelFetchOffset({{.*}}sampler2DRect
        // GLSL_NOGRADS:  texelFetchOffset({{.*}}sampler2DRect
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler2DRect, ivec2(coord), offset2D)

        // GLSL_GRADS:    texelFetchOffset({{.*}}sampler1DArray
        // GLSL_NOGRADS:  texelFetchOffset({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler1DArray, ivec2(coord), int(0), int(0))

        // GLSL_GRADS:    texelFetchOffset({{.*}}sampler2DArray
        // GLSL_NOGRADS:  texelFetchOffset({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_NOGRADS:  [[IMAGE:%[1-9][0-9]*]] = OpImage{{.*}}[[LOAD]]
        // SPIR_GRADS:    OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageFetch {{.*}}[[IMAGE]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == texelFetchOffset(gsampler2DArray, ivec3(coord), int(0), offset2D)

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler1D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler1D, vec2(coord), int(0))

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler1D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler1D, vec2(coord), int(0), float(0))

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler1D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler1D, vec4(coord), int(0))

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler1D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler1D, vec4(coord), int(0), float(0))

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler2D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2D, vec3(coord), offset2D)

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler2D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2D, vec3(coord), offset2D, float(0))

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler2D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2D, vec4(coord), offset2D)

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler2D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2D, vec4(coord), offset2D, float(0))

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler3D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler3D, vec4(coord), offset3D)

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler3D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler3D, vec4(coord), offset3D, float(0))

        // GLSL_GRADS-COUNT-2:    textureProjOffset({{.*}}sampler2DRect
        // GLSL_NOGRADS-COUNT-2:  textureProjLodOffset({{.*}}sampler2DRect
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2DRect, vec3(coord), offset2D)

        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageSampleProjImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjOffset(gsampler2DRect, vec4(coord), offset2D)

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureProjOffset(uniform_sampler2DRectShadow, vec4(coord), offset2D)

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureProjOffset(uniform_sampler1DShadow, vec4(coord), int(0))

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureProjOffset(uniform_sampler1DShadow, vec4(coord), int(0), float(0))

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureProjOffset(uniform_sampler2DShadow, vec4(coord), offset2D) || ignoreResultF32)

        // GLSL_GRADS:    textureProjOffset({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefImplicitLod {{.*}}[[LOAD]]{{.*}} Bias|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureProjOffset(uniform_sampler2DShadow, vec4(coord), offset2D, float(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureLodOffset({{.*}}sampler1D
        // GLSL_NOGRADS:  textureLodOffset({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler1D, float(coord), float(0), 0)

        // GLSL_GRADS:    textureLodOffset({{.*}}sampler2D
        // GLSL_NOGRADS:  textureLodOffset({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler2D, vec2(coord), float(0), offset2D)

        // GLSL_GRADS:    textureLodOffset({{.*}}sampler3D
        // GLSL_NOGRADS:  textureLodOffset({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler3D, vec3(coord), float(0), offset3D)

        // GLSL_GRADS:    textureLodOffset({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureLodOffset({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureLodOffset(uniform_sampler1DShadow, vec3(coord), float(0), 0)

        // GLSL_GRADS:    textureLodOffset({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureLodOffset({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureLodOffset(uniform_sampler2DShadow, vec3(coord), float(0), offset2D) || ignoreResultF32)

        // GLSL_GRADS:    textureLodOffset({{.*}}sampler1DArray
        // GLSL_NOGRADS:  textureLodOffset({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler1DArray, vec2(coord), float(0), 0)

        // GLSL_GRADS:    textureLodOffset({{.*}}sampler2DArray
        // GLSL_NOGRADS:  textureLodOffset({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureLodOffset(gsampler2DArray, vec3(coord), float(0), offset2D)

        // GLSL_GRADS:    textureLodOffset({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  textureLodOffset({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureLodOffset(uniform_sampler1DArrayShadow, vec3(coord), float(0), int(0))

        // GLSL_GRADS-COUNT-2:    textureProjLod({{.*}}sampler1D
        // GLSL_NOGRADS-COUNT-2:  textureProjLod({{.*}}sampler1D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProjLod(gsampler1D, vec2(coord), float(0))
        && gvec4(T.Element(0)) == textureProjLod(gsampler1D, vec4(coord), float(0))

        // GLSL_GRADS-COUNT-2:    textureProjLod({{.*}}sampler2D
        // GLSL_NOGRADS-COUNT-2:  textureProjLod({{.*}}sampler2D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProjLod(gsampler2D, vec3(coord), float(0))
        && gvec4(T.Element(0)) == textureProjLod(gsampler2D, vec4(coord), float(0))

        // GLSL_GRADS:    textureProjLod({{.*}}sampler3D
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && gvec4(T.Element(0)) == textureProjLod(gsampler3D, vec4(coord), float(0))

        // GLSL_GRADS:    textureProjLod({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && float(0) == textureProjLod(uniform_sampler1DShadow, vec4(coord), float(0))

        // GLSL_GRADS:    textureProjLod({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureProjLod({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod %
        && (float(0) == textureProjLod(uniform_sampler2DShadow, vec4(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS-COUNT-2:    textureProjLodOffset({{.*}}sampler1D
        // GLSL_NOGRADS-COUNT-2:  textureProjLodOffset({{.*}}sampler1D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler1D, vec2(coord), float(0), 0)
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler1D, vec4(coord), float(0), 0)

        // GLSL_GRADS-COUNT-2:    textureProjLodOffset({{.*}}sampler2D
        // GLSL_NOGRADS-COUNT-2:  textureProjLodOffset({{.*}}sampler2D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler2D, vec3(coord), float(0), offset2D)
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler2D, vec4(coord), float(0), offset2D)

        // GLSL_GRADS:    textureProjLodOffset({{.*}}sampler3D
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && gvec4(T.Element(0)) == textureProjLodOffset(gsampler3D, vec4(coord), float(0), offset3D)

        // GLSL_GRADS:    textureProjLodOffset({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && float(0) == textureProjLodOffset(uniform_sampler1DShadow, vec4(coord), float(0), int(0))

        // GLSL_GRADS:    textureProjLodOffset({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureProjLodOffset({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Lod|ConstOffset %
        && (float(0) == textureProjLodOffset(uniform_sampler2DShadow, vec4(coord), float(0), offset2D) || ignoreResultF32)

        // GLSL_GRADS:    textureGrad({{.*}}sampler1D
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler1D, float(coord), float(0), float(0))

        // GLSL_GRADS:    textureGrad({{.*}}sampler2D
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler2D, vec2(coord), vec2(0), vec2(0))

        // GLSL_GRADS:    textureGrad({{.*}}sampler3D
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler3D, vec3(coord), vec3(0), vec3(0))

        // GLSL_GRADS:    textureGrad({{.*}}samplerCube
        // GLSL_NOGRADS:  textureGrad({{.*}}samplerCube
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsamplerCube, vec3(coord), vec3(0), vec3(0))

        // GLSL_GRADS:    textureGrad({{.*}}sampler2DRect
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler2DRect
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler2DRect, vec2(coord), vec2(0), vec2(0))

        // GLSL_GRADS:    textureGrad({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureGrad(uniform_sampler2DRectShadow, vec3(coord), vec2(0), vec2(0))

        // GLSL_GRADS:    textureGrad({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureGrad(uniform_sampler1DShadow, vec3(coord), float(0), float(0))

        // GLSL_GRADS:    textureGrad({{.*}}sampler1DArray
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler1DArray, vec2(coord), float(0), float(0))

        // GLSL_GRADS:    textureGrad({{.*}}sampler2DArray
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsampler2DArray, vec3(coord), vec2(0), vec2(0))

        // GLSL_GRADS:    textureGrad({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureGrad(uniform_sampler1DArrayShadow, vec3(coord), float(0), float(0))

        // GLSL_GRADS:    textureGrad({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && (float(0) == textureGrad(uniform_sampler2DShadow, vec3(coord), vec2(0), vec2(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureGrad({{.*}}samplerCubeShadow
        // GLSL_NOGRADS:  textureGrad({{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && (float(0) == textureGrad(uniform_samplerCubeShadow, vec4(coord), vec3(0), vec3(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureGrad({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  textureGrad({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureGrad(uniform_sampler2DArrayShadow, vec4(coord), vec2(0), vec2(0))

        // GLSL_GRADS:    textureGrad({{.*}}samplerCubeArray
        // GLSL_NOGRADS:  textureGrad({{.*}}samplerCubeArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureGrad(gsamplerCubeArray, vec4(coord), vec3(0), vec3(0))

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler1D
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler1D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler1D, float(coord), float(0), float(0), 0)

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler2D
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler2D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler2D, vec2(coord), vec2(0), vec2(0), offset2D)

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler3D
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler3D, vec3(coord), vec3(0), vec3(0), offset3D)

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler2DRect
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler2DRect
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler2DRect, vec2(coord), vec2(0), vec2(0), offset2D)

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureGradOffset(uniform_sampler2DRectShadow, vec3(coord), vec2(0), vec2(0), offset2D)

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureGradOffset(uniform_sampler1DShadow, vec3(coord), float(0), float(0), int(0))

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && (float(0) == textureGradOffset(uniform_sampler2DShadow, vec3(coord), vec2(0), vec2(0), offset2D) || ignoreResultF32)

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler2DArray
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler2DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler2DArray, vec3(coord), vec2(0), vec2(0), offset2D)

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler1DArray
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler1DArray
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArray
        // SPIR_GRADS:    OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureGradOffset(gsampler1DArray, vec2(coord), float(0), float(0), int(0))

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler1DArrayShadow
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureGradOffset(uniform_sampler1DArrayShadow, vec3(coord), float(0), float(0), int(0))

        // GLSL_GRADS:    textureGradOffset({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  textureGradOffset({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureGradOffset(uniform_sampler2DArrayShadow, vec4(coord), vec2(0), vec2(0), offset2D)

        // GLSL_GRADS-COUNT-2:    textureProjGrad({{.*}}sampler1D
        // GLSL_NOGRADS-COUNT-2:  textureProjGrad({{.*}}sampler1D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureProjGrad(gsampler1D, vec2(coord), float(0), float(0))
        && gvec4(T.Element(0)) == textureProjGrad(gsampler1D, vec4(coord), float(0), float(0))

        // GLSL_GRADS-COUNT-2:    textureProjGrad({{.*}}sampler2D
        // GLSL_NOGRADS-COUNT-2:  textureProjGrad({{.*}}sampler2D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureProjGrad(gsampler2D, vec3(coord), vec2(0), vec2(0))
        && gvec4(T.Element(0)) == textureProjGrad(gsampler2D, vec4(coord), vec2(0), vec2(0))

        // GLSL_GRADS:    textureProjGrad({{.*}}sampler3D
        // GLSL_NOGRADS:  textureProjGrad({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureProjGrad(gsampler3D, vec4(coord), vec3(0), vec3(0))

        // GLSL_GRADS-COUNT-2:    textureProjGrad({{.*}}sampler2DRect
        // GLSL_NOGRADS-COUNT-2:  textureProjGrad({{.*}}sampler2DRect
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && gvec4(T.Element(0)) == textureProjGrad(gsampler2DRect, vec3(coord), vec2(0), vec2(0))
        && gvec4(T.Element(0)) == textureProjGrad(gsampler2DRect, vec4(coord), vec2(0), vec2(0))

        // GLSL_GRADS:    textureProjGrad({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureProjGrad({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureProjGrad(uniform_sampler2DRectShadow, vec4(coord), vec2(0), vec2(0))

        // GLSL_GRADS:    textureProjGrad({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureProjGrad({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && float(0) == textureProjGrad(uniform_sampler1DShadow, vec4(coord), float(0), float(0))

        // GLSL_GRADS:    textureProjGrad({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureProjGrad({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad %
        && (float(0) == textureProjGrad(uniform_sampler2DShadow, vec4(coord), vec2(0), vec2(0)) || ignoreResultF32)

        // GLSL_GRADS-COUNT-2:    textureProjGradOffset({{.*}}sampler1D
        // GLSL_NOGRADS-COUNT-2:  textureProjGradOffset({{.*}}sampler1D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler1D, vec2(coord), float(0), float(0), 0)
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler1D, vec4(coord), float(0), float(0), 0)

        // GLSL_GRADS-COUNT-2:    textureProjGradOffset({{.*}}sampler2D
        // GLSL_NOGRADS-COUNT-2:  textureProjGradOffset({{.*}}sampler2D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler2D, vec3(coord), vec2(0), vec2(0), offset2D)
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler2D, vec4(coord), vec2(0), vec2(0), offset2D)

        // GLSL_GRADS:    textureProjGradOffset({{.*}}sampler3D
        // GLSL_NOGRADS:  textureProjGradOffset({{.*}}sampler3D
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler3D
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler3D, vec4(coord), vec3(0), vec3(0), offset3D)

        // GLSL_GRADS-COUNT-2:    textureProjGradOffset({{.*}}sampler2DRect
        // GLSL_NOGRADS-COUNT-2:  textureProjGradOffset({{.*}}sampler2DRect
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler2DRect, vec3(coord), vec2(0), vec2(0), offset2D)
        && gvec4(T.Element(0)) == textureProjGradOffset(gsampler2DRect, vec4(coord), vec2(0), vec2(0), offset2D)

        // GLSL_GRADS:    textureProjGradOffset({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureProjGradOffset({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureProjGradOffset(uniform_sampler2DRectShadow, vec4(coord), vec2(0), vec2(0), offset2D)

        // GLSL_GRADS:    textureProjGradOffset({{.*}}sampler1DShadow
        // GLSL_NOGRADS:  textureProjGradOffset({{.*}}sampler1DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler1DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && float(0) == textureProjGradOffset(uniform_sampler1DShadow, vec4(coord), float(0), float(0), int(0))

        // GLSL_GRADS:    textureProjGradOffset({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureProjGradOffset({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        // SPIR_NOGRADS:  OpImageSampleProjDrefExplicitLod {{.*}}[[LOAD]]{{.*}} Grad|ConstOffset %
        && (float(0) == textureProjGradOffset(uniform_sampler2DShadow, vec4(coord), vec2(0), vec2(0), offset2D) || ignoreResultF32)

        // 8.9.4. Texture Gather Functions

        // GLSL_GRADS-COUNT-2:    textureGather({{.*}}sampler2D
        // GLSL_NOGRADS-COUNT-2:  textureGather({{.*}}sampler2D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsampler2D, vec2(coord))
        && gvec4(T.Element(0)) == textureGather(gsampler2D, vec2(coord), int(0))

        // GLSL_GRADS-COUNT-2:    textureGather({{.*}}sampler2DArray
        // GLSL_NOGRADS-COUNT-2:  textureGather({{.*}}sampler2DArray
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsampler2DArray, vec3(coord))
        && gvec4(T.Element(0)) == textureGather(gsampler2DArray, vec3(coord), int(0))

        // GLSL_GRADS-COUNT-2:    textureGather({{.*}}samplerCube
        // GLSL_NOGRADS-COUNT-2:  textureGather({{.*}}samplerCube
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCube
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsamplerCube, vec3(coord))
        && gvec4(T.Element(0)) == textureGather(gsamplerCube, vec3(coord), int(0))

        // GLSL_GRADS-COUNT-2:    textureGather({{.*}}samplerCubeArray
        // GLSL_NOGRADS-COUNT-2:  textureGather({{.*}}samplerCubeArray
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArray
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsamplerCubeArray, vec4(coord))
        && gvec4(T.Element(0)) == textureGather(gsamplerCubeArray, vec4(coord), int(0))

        // GLSL_GRADS-COUNT-2:    textureGather({{.*}}sampler2DRect
        // GLSL_NOGRADS-COUNT-2:  textureGather({{.*}}sampler2DRect
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]
        && gvec4(T.Element(0)) == textureGather(gsampler2DRect, vec2(coord))
        && gvec4(T.Element(0)) == textureGather(gsampler2DRect, vec2(coord), int(0))

        // GLSL_GRADS:    textureGather({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureGather({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && (vec4(0) == textureGather(uniform_sampler2DShadow, vec2(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureGather({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  textureGather({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && vec4(0) == textureGather(uniform_sampler2DArrayShadow, vec3(coord), float(0))

        // GLSL_GRADS:    textureGather({{.*}}samplerCubeShadow
        // GLSL_NOGRADS:  textureGather({{.*}}samplerCubeShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && (vec4(0) == textureGather(uniform_samplerCubeShadow, vec3(coord), float(0)) || ignoreResultF32)

        // GLSL_GRADS:    textureGather({{.*}}samplerCubeArrayShadow
        // GLSL_NOGRADS:  textureGather({{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}samplerCubeArrayShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && vec4(0) == textureGather(uniform_samplerCubeArrayShadow, vec4(coord), float(0))

        // GLSL_GRADS:    textureGather({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureGather({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]
        && vec4(0) == textureGather(uniform_sampler2DRectShadow, vec2(coord), float(0))

        // GLSL_GRADS-COUNT-2:    textureGatherOffset({{.*}}sampler2D
        // GLSL_NOGRADS-COUNT-2:  textureGatherOffset({{.*}}sampler2D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2D, vec2(coord), offset2D)
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2D, vec2(coord), offset2D, int(0))

        // GLSL_GRADS-COUNT-2:    textureGatherOffset({{.*}}sampler2DArray
        // GLSL_NOGRADS-COUNT-2:  textureGatherOffset({{.*}}sampler2DArray
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2DArray, vec3(coord), offset2D)
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2DArray, vec3(coord), offset2D, int(0))

        // GLSL_GRADS:    textureGatherOffset({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureGatherOffset({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && (vec4(0) == textureGatherOffset(uniform_sampler2DShadow, vec2(coord), float(0), offset2D) || ignoreResultF32)

        // GLSL_GRADS:    textureGatherOffset({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  textureGatherOffset({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && vec4(0) == textureGatherOffset(uniform_sampler2DArrayShadow, vec3(coord), float(0), offset2D)

        // GLSL_GRADS-COUNT-2:    textureGatherOffset({{.*}}sampler2DRect
        // GLSL_NOGRADS-COUNT-2:  textureGatherOffset({{.*}}sampler2DRect
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2DRect, vec2(coord), offset2D)
        && gvec4(T.Element(0)) == textureGatherOffset(gsampler2DRect, vec2(coord), offset2D, int(0))

        // GLSL_GRADS:    textureGatherOffset({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureGatherOffset({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffset %
        && vec4(0) == textureGatherOffset(uniform_sampler2DRectShadow, vec2(coord), float(0), offset2D)

        // GLSL_GRADS-COUNT-2:    textureGatherOffsets({{.*}}sampler2D
        // GLSL_NOGRADS-COUNT-2:  textureGatherOffsets({{.*}}sampler2D
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2D
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2D, vec2(coord), offsets)
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2D, vec2(coord), offsets, int(0))

        // GLSL_GRADS-COUNT-2:    textureGatherOffsets({{.*}}sampler2DArray
        // GLSL_NOGRADS-COUNT-2:  textureGatherOffsets({{.*}}sampler2DArray
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArray
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2DArray, vec3(coord), offsets)
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2DArray, vec3(coord), offsets, int(0))

        // GLSL_GRADS:    textureGatherOffsets({{.*}}sampler2DShadow
        // GLSL_NOGRADS:  textureGatherOffsets({{.*}}sampler2DShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && (vec4(0) == textureGatherOffsets(uniform_sampler2DShadow, vec2(coord), float(0), offsets) || ignoreResultF32)

        // GLSL_GRADS:    textureGatherOffsets({{.*}}sampler2DArrayShadow
        // GLSL_NOGRADS:  textureGatherOffsets({{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DArrayShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && vec4(0) == textureGatherOffsets(uniform_sampler2DArrayShadow, vec3(coord), float(0), offsets)

        // GLSL_GRADS-COUNT-2:    textureGatherOffsets({{.*}}sampler2DRect
        // GLSL_NOGRADS-COUNT-2:  textureGatherOffsets({{.*}}sampler2DRect
        // SPIR_GRADS-COUNT-2:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_NOGRADS-COUNT-2:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRect
        // SPIR_GRADS:    OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR_NOGRADS:  OpImageGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2DRect, vec2(coord), offsets)
        && gvec4(T.Element(0)) == textureGatherOffsets(gsampler2DRect, vec2(coord), offsets, int(0))

        // GLSL_GRADS:    textureGatherOffsets({{.*}}sampler2DRectShadow
        // GLSL_NOGRADS:  textureGatherOffsets({{.*}}sampler2DRectShadow
        // SPIR_GRADS:    [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_NOGRADS:  [[LOAD:%[1-9][0-9]*]] = OpLoad{{.*}}sampler2DRectShadow
        // SPIR_GRADS:    OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
        // SPIR_NOGRADS:  OpImageDrefGather {{.*}}[[LOAD]]{{.*}} ConstOffsets %
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
    // GLSL_GRADS:    void main(
    // GLSL_NOGRADS:  void main(
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
