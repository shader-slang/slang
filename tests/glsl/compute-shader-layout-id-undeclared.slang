// A `local_size_{xyz}_id` that names an id with no `constant_id` declaration gets an
// implicit specialization constant with that id. Its default is the axis's size from a
// `layout(local_size_<axis> = N) in;` statement, or 1 when there is none. A declared
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
//TEST:SIMPLE(filecheck=SIZE_FIRST):-target spirv -stage compute -entry main -DSIZE_FIRST
//TEST:SIMPLE(filecheck=SIZE_LAST):-target spirv -stage compute -entry main -DSIZE_LAST
//TEST:SIMPLE(filecheck=SIZE_SPEC):-target spirv -stage compute -entry main -DSIZE_SPEC

#version 450

#if defined(TWO)

// Two undeclared ids: the second lookup accepts the first implicit attribute, and the two
// implicit constants stay separate symbols.
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

// Three undeclared ids: every axis gets a distinct SpecId.
// THREE: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %[[Y:[0-9A-Za-z_]+]] %[[Z:[0-9A-Za-z_]+]]
// THREE-DAG: OpDecorate %[[X]] SpecId 5
// THREE-DAG: OpDecorate %[[Y]] SpecId 6
// THREE-DAG: OpDecorate %[[Z]] SpecId 7
// THREE-DAG: %[[X]] = OpSpecConstant %int 1
// THREE-DAG: %[[Y]] = OpSpecConstant %int 1
// THREE-DAG: %[[Z]] = OpSpecConstant %int 1

layout(local_size_x_id = 5, local_size_y_id = 6, local_size_z_id = 7) in;

#elif defined(MIXED)

// An implicit constant, a constant_id declared before the layout, and one declared after.
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

// The same id on two axes of one layout shares one implicit constant.
// REPEAT: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %[[X]] %int_1
// REPEAT-DAG: OpDecorate %[[X]] SpecId 5
// REPEAT-DAG: %[[X]] = OpSpecConstant %int 1

layout(local_size_x_id = 5, local_size_y_id = 5) in;

#elif defined(AFTER)

// A constant_id declared after the layout is used instead of an implicit constant.
// AFTER: OpExecutionModeId %main LocalSizeId %declaredAfter %int_1 %int_1
// AFTER-DAG: OpDecorate %declaredAfter SpecId 5
// AFTER-DAG: %declaredAfter = OpSpecConstant %int 4

layout(local_size_x_id = 5) in;
layout(constant_id = 5) const int declaredAfter = 4;

#elif defined(AFTER_ATTR)

// The same with a `[vk::constant_id]` attribute, which `checkAttribute` checks.
// AFTER_ATTR: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %declaredAfter %int_1
// AFTER_ATTR-DAG: OpDecorate %[[X]] SpecId 5
// AFTER_ATTR-DAG: OpDecorate %declaredAfter SpecId 6
// AFTER_ATTR-DAG: %[[X]] = OpSpecConstant %int 1
// AFTER_ATTR-DAG: %declaredAfter = OpSpecConstant %int 7

layout(local_size_x_id = 5, local_size_y_id = 6) in;
[vk::constant_id(6)]
const int declaredAfter = 7;

#elif defined(SEPARATE)

// A second layout statement that repeats an id finds the first one's implicit constant.
// SEPARATE: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %[[X]] %int_1
// SEPARATE-DAG: OpDecorate %[[X]] SpecId 5
// SEPARATE-DAG: %[[X]] = OpSpecConstant %int 1

layout(local_size_x_id = 5) in;
layout(local_size_y_id = 5) in;

#elif defined(LARGE)

// The lookup compares the full id rather than the 32-bit `location`, so an id of 2^31 or
// more still matches its declaration.
// LARGE: OpExecutionModeId %main LocalSizeId %declaredLarge %[[Y:[0-9A-Za-z_]+]] %int_1
// LARGE-DAG: OpDecorate %declaredLarge SpecId 3000000000
// LARGE-DAG: OpDecorate %[[Y]] SpecId 6
// LARGE-DAG: %declaredLarge = OpSpecConstant %int 7

layout(local_size_x_id = 3000000000, local_size_y_id = 6) in;
layout(constant_id = 3000000000) const int declaredLarge = 7;

#elif defined(SIZE_FIRST)

// An explicit size for an axis is the default of that axis's implicit constant, as glslang
// gives for the GL_KHR_vulkan_glsl example below; an axis with no size defaults to 1.
// SIZE_FIRST: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %int_32 %[[Z:[0-9A-Za-z_]+]]
// SIZE_FIRST-DAG: OpDecorate %[[X]] SpecId 18
// SIZE_FIRST-DAG: OpDecorate %[[Z]] SpecId 19
// SIZE_FIRST-DAG: %[[X]] = OpSpecConstant %int 32
// SIZE_FIRST-DAG: %[[Z]] = OpSpecConstant %int 1

layout(local_size_x = 32, local_size_y = 32) in;
layout(local_size_x_id = 18) in;
layout(local_size_z_id = 19) in;

#elif defined(SIZE_LAST)

// The size statement may follow the id statement and use a named constant.
// SIZE_LAST: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %int_1 %int_1
// SIZE_LAST-DAG: OpDecorate %[[X]] SpecId 18
// SIZE_LAST-DAG: %[[X]] = OpSpecConstant %int 16

const int SIZE = 16;
layout(local_size_x_id = 18) in;
layout(local_size_x = SIZE) in;

#elif defined(SIZE_SPEC)

// A size given by a specialization constant is accepted when another statement names an id
// for the same axis; it has no compile-time value to use as the implicit constant's default.
// SIZE_SPEC-NOT: error[E
// SIZE_SPEC: OpExecutionModeId %main LocalSizeId %[[X:[0-9A-Za-z_]+]] %int_1 %int_1
// SIZE_SPEC-DAG: OpDecorate %[[X]] SpecId 18
// SIZE_SPEC-DAG: %[[X]] = OpSpecConstant %int 1

layout(constant_id = 3) const int SPEC_SIZE = 8;
layout(local_size_x = SPEC_SIZE) in;
layout(local_size_x_id = 18) in;

#endif

void main()
{
}
