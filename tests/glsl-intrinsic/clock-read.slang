//TEST:SIMPLE(filecheck=CHECK1): -target glsl
//TEST:SIMPLE(filecheck=CHECK2): -target spirv
//TEST:SIMPLE(filecheck=CHECK3): -target spirv -emit-spirv-via-glsl


// CHECK1: GL_ARB_shader_clock : require
// CHECK2: OpCapability ShaderClockKHR
// CHECK2: OpExtension "SPV_KHR_shader_clock"
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    output[0] = clock2x32ARB().x;
    // CHECK1: clock2x32ARB
    // CHECK2: OpReadClockKHR %v2uint %uint_3
    // CHECK3: OpReadClockKHR %v2uint %uint_3
}
