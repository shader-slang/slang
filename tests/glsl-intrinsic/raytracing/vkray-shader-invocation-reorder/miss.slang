//DISABLE_TEST:CROSS_COMPILE: -profile glsl_460+GL_EXT_ray_tracing -stage miss -entry main -target spirv-assembly

//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -profile glsl_460+GL_NV_ray_tracing -stage miss -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -allow-glsl -emit-spirv-directly -stage miss -entry main -target spirv-assembly

struct ShadowRay
{
    float hitDistance;
};

// CHECK_GLSL: void main(
// CHECK_SPIRV: OpEntryPoint
void main(in out ShadowRay ray)
{
	ray.hitDistance = 10000.0f;
}
