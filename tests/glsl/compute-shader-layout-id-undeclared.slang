// A `local_size_{xyz}_id` that names an id with no `constant_id` declaration gets an
// implicit specialization constant with that id and a default value of 1. A declared
// `constant_id` is used instead wherever it appears in the file, before or after the
// `layout(...) in;` statement.

//TEST:SIMPLE(filecheck=TWO):-target spirv -stage compute -entry main -DTWO
//TEST:SIMPLE(filecheck=TWO_GLSL):-target glsl -stage compute -entry main -DTWO
//TEST:SIMPLE(filecheck=THREE):-target spirv -stage compute -entry main -DTHREE
//TEST:SIMPLE(filecheck=MIXED):-target spirv -stage compute -entry main -DMIXED
//TEST:SIMPLE(filecheck=REPEAT):-target spirv -stage compute -entry main -DREPEAT
//TEST:SIMPLE(filecheck=AFTER):-target spirv -stage compute -entry main -DAFTER
//TEST:SIMPLE(filecheck=AFTER_ATTR):-target spirv -stage compute -entry main -DAFTER_ATTR
//TEST:SIMPLE(filecheck=SEPARATE):-target spirv -stage compute -entry main -DSEPARATE
//TEST:SIMPLE(filecheck=LARGE):-target spirv -stage compute -entry main -DLARGE

#version 450

#if defined(TWO)

// TWO: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %[[Y:[0-9A-Za-z_]+]] %int_2
// TWO-DAG: OpDecorate %[[X]] SpecId 5
// TWO-DAG: OpDecorate %[[Y]] SpecId 6
// TWO-DAG: %[[X]] = OpSpecConstant %int 1
// TWO-DAG: %[[Y]] = OpSpecConstant %int 1

// TWO_GLSL-DAG: layout(constant_id = 5)
// TWO_GLSL-DAG: layout(constant_id = 6)
// TWO_GLSL-DAG: const int {{[0-9A-Za-z_]+}} = 1;
// TWO_GLSL-DAG: const int {{[0-9A-Za-z_]+}} = 1;
// TWO_GLSL-DAG: layout(local_size_x_id = 5, local_size_y_id = 6, local_size_z = 2) in;

layout(local_size_x_id = 5, local_size_y_id = 6, local_size_z = 2) in;

#elif defined(THREE)

// THREE: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %[[Y:[0-9A-Za-z_]+]] %[[Z:[0-9A-Za-z_]+]]
// THREE-DAG: OpDecorate %[[X]] SpecId 5
// THREE-DAG: OpDecorate %[[Y]] SpecId 6
// THREE-DAG: OpDecorate %[[Z]] SpecId 7
// THREE-DAG: %[[X]] = OpSpecConstant %int 1
// THREE-DAG: %[[Y]] = OpSpecConstant %int 1
// THREE-DAG: %[[Z]] = OpSpecConstant %int 1

layout(local_size_x_id = 5, local_size_y_id = 6, local_size_z_id = 7) in;

#elif defined(MIXED)

// MIXED: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %declaredBefore %declaredAfter
// MIXED-DAG: OpDecorate %[[X]] SpecId 5
// MIXED-DAG: OpDecorate %declaredBefore SpecId 6
// MIXED-DAG: OpDecorate %declaredAfter SpecId 7
// MIXED-DAG: %[[X]] = OpSpecConstant %int 1
// MIXED-DAG: %declaredBefore = OpSpecConstant %int 3
// MIXED-DAG: %declaredAfter = OpSpecConstant %int 4

layout(constant_id = 6) const int declaredBefore = 3;
layout(local_size_x_id = 5, local_size_y_id = 6, local_size_z_id = 7) in;
layout(constant_id = 7) const int declaredAfter = 4;

#elif defined(REPEAT)

// REPEAT: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %[[X]] %int_1
// REPEAT-DAG: OpDecorate %[[X]] SpecId 5
// REPEAT-DAG: %[[X]] = OpSpecConstant %int 1

layout(local_size_x_id = 5, local_size_y_id = 5) in;

#elif defined(AFTER)

// AFTER: OpExecutionModeId %main LocalSizeId %declaredAfter %int_1 %int_1
// AFTER-DAG: OpDecorate %declaredAfter SpecId 5
// AFTER-DAG: %declaredAfter = OpSpecConstant %int 4

layout(local_size_x_id = 5) in;
layout(constant_id = 5) const int declaredAfter = 4;

#elif defined(AFTER_ATTR)

// AFTER_ATTR: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %declaredAfter %int_1
// AFTER_ATTR-DAG: OpDecorate %[[X]] SpecId 5
// AFTER_ATTR-DAG: OpDecorate %declaredAfter SpecId 6
// AFTER_ATTR-DAG: %[[X]] = OpSpecConstant %int 1
// AFTER_ATTR-DAG: %declaredAfter = OpSpecConstant %int 7

layout(local_size_x_id = 5, local_size_y_id = 6) in;
[vk::constant_id(6)]
const int declaredAfter = 7;

#elif defined(SEPARATE)

// SEPARATE: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %[[X]] %int_1
// SEPARATE-DAG: OpDecorate %[[X]] SpecId 5
// SEPARATE-DAG: %[[X]] = OpSpecConstant %int 1

layout(local_size_x_id = 5) in;
layout(local_size_y_id = 5) in;

#elif defined(LARGE)

// LARGE: OpExecutionModeId %main LocalSizeId %declaredLarge %[[Y:[0-9A-Za-z_]+]] %int_1
// LARGE-DAG: OpDecorate %declaredLarge SpecId 3000000000
// LARGE-DAG: OpDecorate %[[Y]] SpecId 6
// LARGE-DAG: %declaredLarge = OpSpecConstant %int 7

layout(local_size_x_id = 3000000000, local_size_y_id = 6) in;
layout(constant_id = 3000000000) const int declaredLarge = 7;

#endif

void main()
{
}
