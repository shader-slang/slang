// NV-named GLSL SER wrappers remain tied to SPV_NV_shader_invocation_reorder.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -allow-glsl -stage raygeneration -entry main -target spirv-assembly -emit-spirv-directly -capability spvShaderInvocationReorderEXT -restrictive-capability-check

// CHECK: error[E41013]
// CHECK: Missing capabilities are: 'spvShaderInvocationReorderNV'

void main()
{
    hitObjectNV hit;
    bool isHit = hitObjectIsHitNV(hit);
}
