// NV-named GLSL SER wrappers remain tied to SPV_NV_shader_invocation_reorder.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -stage raygeneration -entry main -target spirv-assembly -emit-spirv-directly -capability spvShaderInvocationReorderEXT -restrictive-capability-check

// CHECK: error[E41013]
// CHECK: Missing capabilities are: 'spvShaderInvocationReorderNV'

import glsl;

void main()
{
    hitObjectNV hit;
    bool isHit = hitObjectIsHitNV(hit);
}
