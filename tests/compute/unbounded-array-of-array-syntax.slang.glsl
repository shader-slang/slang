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
    int innerIndex_0;

    int index_0 = int(gl_GlobalInvocationID.x);

    int innerIndex_1 = index_0 & 3;

    uint bufferCount_0;
    uint bufferStride_0;
    (bufferCount_0) = (g_aoa_0[nonuniformEXT(index_0 >> 2)])._data.length();
    (bufferStride_0) = 0;

    if(uint(innerIndex_1) >= bufferCount_0)
    {
        int _S3 = int(bufferCount_0 - uint(1));
        innerIndex_0 = _S3;
    }
    else
    {
        innerIndex_0 = innerIndex_1;
    }
    uint _S4 = uint(innerIndex_0);
    ((outputBuffer_0)._data[(uint(index_0))]) = ((g_aoa_0[nonuniformEXT(index_0 >> 2)])._data[(_S4)]);
    return;
}
