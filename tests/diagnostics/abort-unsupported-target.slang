// Verify that calling the variadic abort() on a target that does not support the
// `abort` capability (which expands to GL_EXT_shader_abort,
// i.e. GLSL and SPIR-V only) produces a clean capability diagnostic instead of an
// unresolved overload or an internal error. The target exclusion is intentional:
// VK_KHR_shader_abort has no HLSL/Metal/WGSL/CUDA/CPU counterpart wired up.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target dxil -profile cs_6_0 -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cpp -entry computeMain -stage compute

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x > 100)
    {
        abort("thread id out of range: %u", tid.x);
    }
}

//CHECK: E36107
