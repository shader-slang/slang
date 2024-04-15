//DISABLED_TEST:SIMPLE(filecheck=CHECK1):-target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK2):-target spirv -entry main -stage compute -emit-spirv-directly

RWTexture2D<uint> texArray[2];

[numthreads(1,1,1)]
void main( uint2 dispatchThreadID : SV_DispatchThreadID, uint2 groupThreadID : SV_GroupThreadID )
{
    // CHECK1-DAG: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1-DAG: OpDecorate %[[VAR2:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1-DAG: OpDecorate %[[VAR3:[a-zA-Z0-9_]+]] NonUniform
    // CHECK1: %[[VAR1]] = OpCopyObject %{{.*}}
    // CHECK1: %[[VAR2]] = OpAccessChain %_ptr_UniformConstant_{{.*}} %{{.*}} %[[VAR1]]
    // CHECK1: %[[VAR3]] = OpImageTexelPointer %_ptr_Image_uint %[[VAR2]] %{{.*}}

    // CHECK2-DAG: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK2: %[[VAR1]] = OpAccessChain %_ptr_UniformConstant_{{.*}} %texArray %{{.*}}
    // CHECK2: %{{.*}} = OpImageTexelPointer %_ptr_Image_uint %[[VAR1]] %{{.*}}
    InterlockedAdd(texArray[NonUniformResourceIndex(dispatchThreadID.x)][uint2(0)], 2);
}
