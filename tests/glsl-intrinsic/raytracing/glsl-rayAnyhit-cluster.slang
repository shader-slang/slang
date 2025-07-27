//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -stage anyhit -entry main -target glsl -capability GL_EXT_ray_tracing -capability GL_NV_cluster_acceleration_structure

//TEST:SIMPLE(filecheck=CHECK_SPV): -allow-glsl -emit-spirv-directly -stage anyhit -entry main -target spirv-assembly -capability SPV_KHR_ray_tracing -capability SPV_NV_cluster_acceleration_structure

// CHECK_GLSL: #extension GL_EXT_ray_tracing : require
// CHECK_GLSL: #extension GL_NV_cluster_acceleration_structure : require
__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_cluster_acceleration_structure)

layout(binding = 0) uniform accelerationStructureEXT as;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outBuffer
//TEST_INPUT:end

// This test demonstrates the usage of cluster acceleration structure intrinsics
// in ray tracing for the NVIDIA extension.

void main()
{
    // CHECK_GLSL-DAG: gl_ClusterIDNV
    // CHECK_SPV-DAG: ClusterIDNV

    // Get the cluster ID of the current hit
    int clusterId = gl_ClusterIDNV;

    // Check if the hit is part of a cluster
    bool isClusterHit = (clusterId != gl_ClusterIDNoneNV);

    // Skip hits for certain cluster IDs (example: skip cluster ID 5)
    if (isClusterHit && clusterId == 5)
    {
        ignoreIntersectionEXT;
    }
}
