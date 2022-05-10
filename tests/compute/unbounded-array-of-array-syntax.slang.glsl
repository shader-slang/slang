//TEST_IGNORE_FILE:

#version 450
layout(row_major) uniform;
layout(row_major) buffer;
#extension GL_EXT_nonuniform_qualifier : require

layout(std430, binding = 1) buffer _S1 {
    int _data[];
} g_aoa_0[];
layout(std430, binding = 0) buffer _S2 {
    int _data[];
} outputBuffer_0;

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;
void main()
{
    int index_0 = int(gl_GlobalInvocationID.x);
    int innerIndex_0 = index_0 & 3;

    uint bufferCount_0;
    uint bufferStride_0;
    (bufferCount_0) = (g_aoa_0[nonuniformEXT(index_0 >> 2)])._data.length();
    (bufferStride_0) = 0;

    int innerIndex_1;
    if(uint(innerIndex_0) >= bufferCount_0)
    {
        innerIndex_1 = int(bufferCount_0 - uint(1));
    }
    else
    {
        innerIndex_1 = innerIndex_0;
    }
    uint _S3 = uint(innerIndex_1);
    ((outputBuffer_0)._data[(uint(index_0))]) = ((g_aoa_0[nonuniformEXT(index_0 >> 2)])._data[(_S3)]);
    return;
}
