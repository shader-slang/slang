//TEST:SIMPLE(filecheck=GLSL): -target glsl
//TEST:SIMPLE(filecheck=SPIRV): -target spirv
// NOTE: Validates target-specific specialization ID/local size emission in
// generated GLSL/SPIR-V text output.

// SPIRV-DAG: OpExecutionModeId %{{.*}} LocalSizeId %[[SX:[0-9A-Za-z_]+]] %[[SY:[0-9A-Za-z_]+]] %[[SZ:[0-9A-Za-z_]+]]
// SPIRV-DAG: OpDecorate %[[SX]] SpecId 3
// SPIRV-DAG: OpDecorate %[[SY]] SpecId 4
// SPIRV-DAG: %[[SX]] = OpSpecConstant %int 8
// SPIRV-DAG: %[[SY]] = OpSpecConstant %int 4
// SPIRV-DAG: %[[SZ]] = OpConstant %int 1

// GLSL: layout(constant_id = 3)
// GLSL-NEXT: int groupSizeX_0 = 8;
//
// GLSL: layout(constant_id = 4)
// GLSL-NEXT: int groupSizeY_0 = 4;
//
// GLSL: layout(local_size_x_id = 3, local_size_y_id = 4, local_size_z = 1) in;

enum class GroupSize : int
{
    X = 8,
    Y = 4,
    Z = 1,
}

[vk::constant_id(3)]
const GroupSize groupSizeX = GroupSize::X;

[vk::constant_id(4)]
const GroupSize groupSizeY = GroupSize::Y;

RWStructuredBuffer<int> outputBuffer;

[numthreads(groupSizeX, groupSizeY, 1)]
void computeMain()
{
    int3 size = WorkgroupSize();
    outputBuffer[0] = size.x;
    outputBuffer[1] = size.y;
    outputBuffer[2] = size.z;
}
