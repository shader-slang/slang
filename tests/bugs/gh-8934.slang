//TEST:SIMPLE(filecheck=CHECK):-target spirv -profile spirv_1_5 -stage fragment -entry main

// For SPIRV version < 1.6, `discard` is a terminator and any instructions after `discard`
// in the same block are unreachable.

struct PerDrawObjectIndexInfo
{
    uint obj_bool_flags;
};

layout (set = 2, binding = 0) StructuredBuffer<PerDrawObjectIndexInfo> obj_index_array;

layout (location = 0) in nointerpolation uint in_obj_info_array_idx;
layout (location = 0) out float4 outColour;

[shader("fragment")]
void main()
{
    // Unconditional discard - everything after in this block is unreachable
    discard;

    // This code is unreachable but was causing SPIRV emission to fail
    // because the load instruction couldn't be emitted after the OpKill terminator
    let obj_flags = obj_index_array[in_obj_info_array_idx].obj_bool_flags;

    uint albedo_idx = obj_flags;

    // Conditional discard
    if (obj_flags > 1)
        discard;

    // Use the value
    outColour = float4(float(albedo_idx));
}

// CHECK: OpKill
