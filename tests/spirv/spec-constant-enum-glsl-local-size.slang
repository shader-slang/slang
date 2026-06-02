//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry main -allow-glsl
// SPIR-V-only: validates LocalSizeId/SpecId emission for GLSL local_size_* enum specialization constants.

#version 450

enum class GroupSize : int
{
    X = 8,
    Y = 4,
};

[vk::constant_id(3)]
const GroupSize groupSizeX = GroupSize::X;

[vk::constant_id(4)]
const GroupSize groupSizeY = GroupSize::Y;

// SPIRV-DAG: OpExecutionModeId %main LocalSizeId %[[SX:[0-9A-Za-z_]+]] %[[SY:[0-9A-Za-z_]+]] %[[SZ:[0-9A-Za-z_]+]]
// SPIRV-DAG: OpDecorate %[[SX]] SpecId 3
// SPIRV-DAG: OpDecorate %[[SY]] SpecId 4
// SPIRV-DAG: %[[SX]] = OpSpecConstant %int 8
// SPIRV-DAG: %[[SY]] = OpSpecConstant %int 4
// SPIRV-DAG: %[[SZ]] = OpConstant %int 1
layout(local_size_x = groupSizeX, local_size_y = groupSizeY, local_size_z = 1) in;

void main()
{
}
