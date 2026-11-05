//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target spirv -emit-spirv-directly -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target spirv -emit-spirv-directly -entry computeMain -stage compute -ignore-capabilities
// CHECK_IGNORE_CAPS-NOT: E36113
// CHECK: warning[E36113]
// CHECK: _GL_NV_ray_tracing_motion_blur
// A compute entry point cannot use this ray tracing capability, so the first line also fails with
// a capability error. Both diagnostics disappear with -ignore-capabilities.
// CHECK: error[E36107]
[require(_GL_NV_ray_tracing_motion_blur)]
[numthreads(1,1,1)]
void computeMain()
{
}