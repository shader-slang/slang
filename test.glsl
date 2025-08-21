#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 5 0
layout(std430, binding = 0) buffer StructuredBuffer_uint_t_0 {
    uint _data[];
} outputBuffer_0;



layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    uint index_0 = gl_GlobalInvocationID.x;
    if(index_0 < 3U)
    {
        if(index_0 == 0U)
        {

#line 18
            outputBuffer_0._data[uint(index_0)] = 8U;

#line 17
        }
        else
        {

#line 19
            if(index_0 == 1U)
            {

#line 20
                outputBuffer_0._data[uint(index_0)] = 8U;

#line 19
            }
            else
            {

#line 21
                if(index_0 == 2U)
                {

#line 22
                    outputBuffer_0._data[uint(index_0)] = 1U;

#line 21
                }

#line 19
            }

#line 17
        }

#line 15
    }

#line 24
    return;
}

