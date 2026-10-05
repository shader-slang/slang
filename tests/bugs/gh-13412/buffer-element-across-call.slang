// A read of a structured-buffer element must stay before a call that writes that element.
// The element address is a `rwstructuredBufferGetElementPtr` inside the function, and that
// is not storage private to the function (shader-slang/slang#13412).
// The CPU lane passes `-g0`: render-test compiles with debug info by default, and that keeps the
// load the forwarding would remove, so the lane would pass even on a miscompiling compiler.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0 -entry computeMain
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none

//TEST_INPUT: set buf = ubuffer(data=[3 0], stride=4)
RWStructuredBuffer<uint> buf;

//TEST_INPUT: set outBuf = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<uint> outBuf;

[noinline]
void writeZero()
{
    buf[0] = 7;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint a = buf[0];
    writeZero();
    uint b = buf[0];
    outBuf[0] = a * 100 + b;
}

// BUF: type: uint32_t
// BUF-NEXT: {{^}}307{{$}}

// HLSL-LABEL: void computeMain(
// HLSL: uint a_0 = buf_0[int(0)];
// HLSL-NEXT: writeZero_0();
// CUDA-LABEL: void computeMain(
// CUDA: uint a_0 = *
// CUDA-NEXT: writeZero_0();
// METAL-LABEL: void computeMain(
// METAL: uint a_0 = *
// METAL-NEXT: writeZero_0(
// GLSL-LABEL: void main(
// GLSL: uint a_0 = buf_0._data[uint(0)];
// GLSL-NEXT: writeZero_0();
