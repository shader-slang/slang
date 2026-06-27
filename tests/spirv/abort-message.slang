// Runtime message-round-trip coverage for the `abort()` intrinsic on Vulkan (VK_KHR_shader_abort),
// complementing the emit + keep-alive coverage in tests/spirv/abort-runtime.slang. Each case here
// fires abort() on a path the 1-thread dispatch always takes, which on capable hardware loses the
// device; slang-rhi (shader-slang/slang-rhi#782) recovers the abort message via the device-fault
// path and forwards it through the debug callback. render-test, when `-render-features shader-abort`
// is requested, runs a device-loss-tolerant capture mode that prints any "Shader abort: ..." text it
// receives to stdout, so the `filecheck=` lines below can assert the formatted message.
//
// Each aborting case is its own render-test invocation (its own //TEST line + its own entry point),
// because a fired abort loses the device and is terminal for that invocation. An unsupporting
// device/driver -> SLANG_E_NOT_AVAILABLE -> the line is cleanly skipped (no GPU in CI).
//
// HONESTY: the abort-fires -> message-round-trip -> FileCheck-match end-to-end is only validated on
// capable hardware (a maintainer run, see slang#11790 / slang-rhi#782). In a GPU-less environment
// these `-vk` lines are skipped before any output is checked.
//
// On a successful capture, render-test prints lines of the form:
//     Shader abort: <your formatted message>
// The device-loss buffer never gets written, so each line's empty <stem>.N.expected.txt makes the
// harness buffer-comparison pass against the empty (pre-cleared) actual buffer.

// Case 0: a plain string abort message (no format arguments).
//TEST(compute):COMPARE_COMPUTE(filecheck=PLAIN): -vk -compute -emit-spirv-directly -capability abort -render-features shader-abort -entry abortPlain
// PLAIN: Shader abort: plain abort message

// Case 1: %f float formatting.
//TEST(compute):COMPARE_COMPUTE(filecheck=FLOAT): -vk -compute -emit-spirv-directly -capability abort -render-features shader-abort -entry abortFloat
// FLOAT: Shader abort: value={{.*}}1.5

// Case 2: more than one argument.
//TEST(compute):COMPARE_COMPUTE(filecheck=MULTI): -vk -compute -emit-spirv-directly -capability abort -render-features shader-abort -entry abortMulti
// MULTI: Shader abort: multi 1 2 3

// Case 3: mixed argument types (an int and a float).
//TEST(compute):COMPARE_COMPUTE(filecheck=MIXED): -vk -compute -emit-spirv-directly -capability abort -render-features shader-abort -entry abortMixed
// MIXED: Shader abort: mixed 7 2.5

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void abortPlain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = 0;
    abort("plain abort message");
}

[shader("compute")]
[numthreads(1, 1, 1)]
void abortFloat(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = 0;
    abort("value=%f", 1.5f);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void abortMulti(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = 0;
    abort("multi %u %u %u", 1u, 2u, 3u);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void abortMixed(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = 0;
    abort("mixed %d %f", 7, 2.5f);
}
