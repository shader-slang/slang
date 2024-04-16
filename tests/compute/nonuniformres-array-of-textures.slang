//TEST:SIMPLE(filecheck=CHECK1):-target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK2):-target spirv -entry main -stage compute -emit-spirv-directly

Texture2D<float> textures[2];
RWTexture2D<float2> outputTexture;
[numthreads(1, 1, 1)]
void main(uint2 pixelIndex : SV_DispatchThreadID)
{

    // CHECK1: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1: OpDecorate %[[VAR2:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1: OpDecorate %[[VAR3:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1: OpDecorate %[[VAR4:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1: OpDecorate %[[VAR5:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1: OpDecorate %[[VAR6:[a-zA-Z0-9_]+]] NonUniform

    // CHECK1: %[[VAR1]] = OpCopyObject %{{.*}}
    // CHECK1: %[[VAR2]] = OpAccessChain %_ptr_UniformConstant_{{.*}} %{{.*}} %[[VAR1]]
    // CHECK1: %[[VAR3]] = OpLoad %{{.*}} %[[VAR2]]
    // CHECK1: %[[VAR4]] = OpCopyObject %{{.*}}
    // CHECK1: %[[VAR5]] = OpAccessChain %_ptr_UniformConstant_{{.*}} %{{.*}} %[[VAR4]]
    // CHECK1: %[[VAR6]] = OpLoad %{{.*}} %[[VAR5]]

    // CHECK2: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK2: OpDecorate %[[VAR2:[a-zA-Z0-9_]+]] NonUniform
    // CHECK2: OpDecorate %[[VAR3:[a-zA-Z0-9_]+]] NonUniform
    // CHECK2: OpDecorate %[[VAR4:[a-zA-Z0-9_]+]] NonUniform
    // CHECK2: %[[VAR1]] = OpAccessChain %_ptr_UniformConstant_{{.*}} %textures{{.*}}
    // CHECK2: %[[VAR2]] = OpLoad %{{.*}} %[[VAR1]]
    // CHECK2: %[[VAR3]] = OpAccessChain %_ptr_UniformConstant_{{.*}} %textures{{.*}}
    // CHECK2: %[[VAR4]] = OpLoad %{{.*}} %[[VAR3]]
    float2 tmp0 = textures[NonUniformResourceIndex(pixelIndex.x)].Load(int3(0, 0, 0));
    outputTexture[0] = tmp0;

    float2 tmp1 = textures[NonUniformResourceIndex(uint(float(pixelIndex.x)))].Load(int3(0, 0, 0));
    outputTexture[1] = tmp1;
}

