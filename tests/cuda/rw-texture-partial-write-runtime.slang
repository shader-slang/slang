// Runs a write to part of a texel (shader-slang/slang#12627) on a CUDA device, so the
// `surf2Dread`/`surf2Dwrite` read-modify-write has to compile with nvrtc, load and execute.
//
// The written texel is not read back: a surface read of an address already written in the same
// kernel launch returns undefined data (CUDA C++ Programming Guide, "Read/Write Coherency"), and
// slang-test only reads back buffers. Reading the texel before the write is defined, because the
// texture is initialized before launch.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT: RWTexture2D(format=RGBA32Float, size=4, content = one, mipMaps = 1):name tex
RWTexture2D<float4> tex;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float4 initial = tex[uint2(0, 0)];
    tex[uint2(0, 0)].w = 7.0f;
    outputBuffer[0] = uint(all(initial == float4(1.0f, 1.0f, 1.0f, 1.0f)));
}

// BUF: 1
