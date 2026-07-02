// Runtime integration test for the `abort()` intrinsic on Vulkan (VK_KHR_shader_abort),
// exercising the slang-rhi support from shader-slang/slang-rhi#782 (Feature::ShaderAbort /
// "shader-abort") end-to-end through slang-test. It complements the emit-only coverage in
// tests/spirv/abort*.slang with two lines: a SIMPLE SPIR-V emit check (device-independent,
// runs/verifies anywhere incl. GPU-less CI) and a -vk COMPARE_COMPUTE line gated on
// `-render-features shader-abort` (render-test maps that name to rhi::Feature::ShaderAbort via
// SLANG_RHI_FEATURES; an unsupporting device/driver -> SLANG_E_NOT_AVAILABLE -> clean skip).
//
// A PASS on capable hardware validates feature selection, device creation with the abort
// extensions enabled, and execution of an abort-containing pipeline. It does NOT (and cannot,
// in slang-test today) validate the abort actually firing, the device-fault path, or the
// abort-message round-trip via vkGetDeviceFaultDebugInfoKHR -- those need a device-loss-tolerant
// harness mode and a maintainer with capable hardware (see slang#11790, slang-rhi#782).
//
// The abort() call sits on a branch the 4-thread dispatch never reaches: OpAbortKHR causes
// device loss, which is incompatible with slang-test's dispatch -> readback -> compare model.
// Keeping abort() present still forces the abort-capable pipeline to be created, but never
// firing it keeps the device alive so the line can produce a clean PASS.

//TEST:SIMPLE(filecheck=SPIRV): -stage compute -entry computeMain -target spirv -capability abort
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type -emit-spirv-directly -capability abort -render-features shader-abort

// SPIRV: OpCapability AbortKHR
// SPIRV: OpExtension "SPV_KHR_abort"
// SPIRV: OpAbortKHR

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Path the dispatch always takes: write a deterministic value per thread.
    outputBuffer[tid.x] = tid.x + 1;

    // Never taken for this 4-thread dispatch (tid.x is in 0..3), but tid.x is a runtime value
    // so the compiler keeps the abort() and emits OpAbortKHR. On capable hardware this forces
    // the abort-enabled pipeline without ever losing the device.
    if (tid.x > 0x1000)
    {
        abort("tid %u out of range", tid.x);
    }
}

// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
