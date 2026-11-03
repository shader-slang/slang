// A discarded call result is not bound to a temporary, which the Metal compiler would report as
// unused (shader-slang/slang#14058); a used result still is. Target intrinsics keep the temporary
// on HLSL and GLSL, and WGSL keeps every temporary.

//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=LIB):-target metallib -entry computeMain -stage compute -Xmetal -Werror=unused-variable -Xmetal -Werror=unused-value
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute -line-directive-mode none -g
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=WAVE):-target metal -entry waveMain -stage compute -line-directive-mode none -DWAVE
//TEST:SIMPLE(filecheck=WAVEHLSL):-target hlsl -entry waveMain -stage compute -line-directive-mode none -DWAVE
//TEST:SIMPLE(filecheck=LIBWAVE):-target metallib -entry waveMain -stage compute -Xmetal -Werror=unused-variable -Xmetal -Werror=unused-value -DWAVE
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

static int a = 0;

int setA(int v)
{
    a = v;
    return v * 2;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // METAL-NOT: = setA_0(
    // METAL: {{^ *}}setA_0({{.*}}int(2)
    // HLSL-NOT: = setA_0(
    // HLSL: {{^ *}}setA_0({{.*}}int(2)
    // GLSL-NOT: = setA_0(
    // GLSL: {{^ *}}setA_0({{.*}} + 2)
    // CUDA-NOT: = setA_0(
    // CUDA: {{^ *}}setA_0({{.*}}int(2)
    // CPP-NOT: = setA_0(
    // CPP: {{^ *}}setA_0({{.*}}int(2)
    // WGSL: = setA_0({{.*}}i32(2)
    setA(int(tid.x) + 2);

    // The source's own `void` cast emits the same bare call, with neither `(void)` nor a temporary.
    // METAL-NOT: = setA_0(
    // METAL: {{^ *}}setA_0({{.*}}int(3)
    // HLSL-NOT: = setA_0(
    // HLSL: {{^ *}}setA_0({{.*}}int(3)
    // GLSL-NOT: = setA_0(
    // GLSL: {{^ *}}setA_0({{.*}} + 3)
    // CUDA-NOT: = setA_0(
    // CUDA: {{^ *}}setA_0({{.*}}int(3)
    // CPP-NOT: = setA_0(
    // CPP: {{^ *}}setA_0({{.*}}int(3)
    (void)setA(int(tid.x) + 3);

    // METAL-NOT: = (frexp
    // METAL: {{^ *}}(void){{.*}}frexp(
    // HLSL: = frexp(
    // GLSL: = frexp(
    // CUDA-NOT: = (F32_frexp
    // CUDA: {{^ *}}(void){{.*}}F32_frexp(
    // CPP-NOT: = (F32_frexp
    // CPP: {{^ *}}(void){{.*}}F32_frexp(
    int e;
    frexp(1.5f, e);

    // With `-g`, the never-read `unread` keeps a debug-only use of the call's result.
    // METAL: {{^ *}}setA_0({{.*}}int(5)
    // HLSL: {{^ *}}setA_0({{.*}}int(5)
    // GLSL: {{^ *}}setA_0({{.*}} + 5)
    // CUDA: {{^ *}}setA_0({{.*}}int(5)
    // CPP: {{^ *}}setA_0({{.*}}int(5)
    int unread = setA(int(tid.x) + 5);

    // METAL: = setA_0({{.*}}int(4)
    // HLSL: used_0 = setA_0(
    // GLSL: used_0 = setA_0(
    // CUDA: = setA_0({{.*}}int(4)
    // CPP: = setA_0({{.*}}int(4)
    int used = setA(int(tid.x) + 4);
    outputBuffer[0] = a + e;
    outputBuffer[1] = used;
}

// The CPU target has no wave operations, so `waveMain` is compiled only by the `-DWAVE` runs.
#ifdef WAVE
RWTexture2D<float> tex;

[numthreads(4, 1, 1)]
void waveMain(uint3 tid: SV_DispatchThreadID)
{
    // WAVE-NOT: = simd_sum(
    // WAVE: {{^ *}}(void)simd_sum(
    // WAVEHLSL: = WaveActiveSum(
    WaveActiveSum(int(tid.x));
    // DXC reports a bare `tex_0.Load(...);` as an unused read-only result, so HLSL keeps it bound.
    // On Metal it expands to the non-call `(tex.read(...).x)`, which needs the `void` cast.
    // WAVE-NOT: = ({{.*}}.read(
    // WAVE: {{^ *}}(void)((tex_{{[0-9]+}}).read(
    // WAVEHLSL: = tex_0.Load(
    tex.Load(int2(tid.xy));
}
#endif

// LIB: @computeMain
// LIBWAVE: @waveMain

// BUF:      5
// BUF-NEXT: 8
