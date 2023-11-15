//TEST_IGNORE_FILE:
#version 450
layout(row_major) uniform;
layout(row_major) buffer;

layout(r32f)
layout(binding = 0)
uniform image2DArray gParams_tex_0;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    float f_0 = (imageLoad((gParams_tex_0), (ivec3(ivec2(gl_GlobalInvocationID.xy), int(gl_GlobalInvocationID.z)))).x);
    imageStore((gParams_tex_0), (ivec3(gl_GlobalInvocationID)), vec4(f_0 + 1.0, float(0), float(0), float(0)));
    return;
}
