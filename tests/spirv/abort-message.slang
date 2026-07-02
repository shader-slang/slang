// Runtime message coverage for the `abort()` intrinsic on Vulkan (VK_KHR_shader_abort),
// complementing the emit + keep-alive coverage in tests/spirv/abort-runtime.slang.
//
// Each case fires abort() on a path the 1-thread dispatch always takes. On capable hardware that
// loses the device; slang-rhi (shader-slang/slang-rhi#782) recovers the abort message via the
// device-fault path and forwards it through the debug callback, and render-test (in the
// `-render-features shader-abort` capture mode) prints the "Shader abort: ..." text to stdout. The
// `filecheck=` prefix asserts that captured message; the `filecheck-buffer=EMPTY` prefix asserts the
// readback buffer is empty (the lost device never writes it), which is how COMPARE_COMPUTE's buffer
// comparison is satisfied inline -- no separate .expected.txt file.
//
// Each aborting case is its own render-test invocation (own //TEST line + own preprocessor switch),
// since a fired abort is terminal for that invocation. COMPARE_COMPUTE always compiles the fixed
// computeMain entry point, so the test selects the scenario with `-xslang -D...` rather than
// `-entry`. Where the feature is unavailable the line is cleanly skipped (SLANG_E_NOT_AVAILABLE) --
// e.g. a GPU-less CI runner.
//
// HONESTY: the abort-fires -> message-round-trip -> FileCheck-match end-to-end is only validated on
// capable hardware (a maintainer run; see slang#11790 / slang-rhi#782).

// Case 0: a plain string abort message (no format arguments).
//TEST(compute):COMPARE_COMPUTE(filecheck=PLAIN,filecheck-buffer=EMPTY): -vk -compute -emit-spirv-directly -capability abort -render-features shader-abort -xslang -DTC_ABORT_PLAIN
// PLAIN: Shader abort: plain abort message

// Case 1: %f float formatting.
//TEST(compute):COMPARE_COMPUTE(filecheck=FLOAT,filecheck-buffer=EMPTY): -vk -compute -emit-spirv-directly -capability abort -render-features shader-abort -xslang -DTC_ABORT_FLOAT
// FLOAT: Shader abort: value={{.*}}1.5

// Case 2: more than one argument.
//TEST(compute):COMPARE_COMPUTE(filecheck=MULTI,filecheck-buffer=EMPTY): -vk -compute -emit-spirv-directly -capability abort -render-features shader-abort -xslang -DTC_ABORT_MULTI
// MULTI: Shader abort: multi 1 2 3

// Case 3: mixed argument types (an int and a float).
//TEST(compute):COMPARE_COMPUTE(filecheck=MIXED,filecheck-buffer=EMPTY): -vk -compute -emit-spirv-directly -capability abort -render-features shader-abort -xslang -DTC_ABORT_MIXED
// MIXED: Shader abort: mixed 7 {{.*}}2.5

// Shared by every case above: the aborted dispatch loses the device, so the readback buffer is
// never written -- assert it stays empty (no stray output) to satisfy the buffer comparison inline.
// EMPTY-NOT: {{.}}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = 0;
#if defined(TC_ABORT_FLOAT)
    abort("value=%f", 1.5f);
#elif defined(TC_ABORT_MULTI)
    abort("multi %u %u %u", 1u, 2u, 3u);
#elif defined(TC_ABORT_MIXED)
    abort("mixed %d %f", 7, 2.5f);
#else
    abort("plain abort message");
#endif
}
