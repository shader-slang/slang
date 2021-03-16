#version 460

#extension GL_NV_ray_tracing : require

int helper_0(accelerationStructureNV a_0, int b_0)
{
    return b_0;
}

layout(binding = 1)
uniform accelerationStructureNV entryPointParams_x_0;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    int _S1 = helper_0(entryPointParams_x_0, 1);
    return;
}
