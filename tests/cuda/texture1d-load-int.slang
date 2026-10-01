// Read-only `Texture1D.Load` on CUDA must fetch the stored texels (#12630). It used to lower to an
// empty prelude stub, so the fetch and the dependent stores were dead-code-eliminated.
//
// The textures use `content=gradient`, so each texel holds a distinct value: for RGBA8Unorm texel x
// of a size-4 texture is (x/3, 0, 0, 1); for RGBA32Uint/Sint it is (x == 3 ? 1 : 0, 0, 0, 1). A
// fetch that returns zero, or reads the wrong texel, fails the CHECK lines.
// This file covers the signed-integer texel type; the CPU backend cannot unpack RGBA32Sint, so it runs
// on CUDA only. `texture1d-load.slang` covers float and uint texels.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

//TEST_INPUT: Texture1D(format=RGBA32Sint, size=4, content=gradient, mipMaps=1):name texI
Texture1D<int4> texI;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//PTX: tex.level.2d.v4.s32.s32

[numthreads(1, 1, 1)]
void computeMain()
{
    int4 a = texI.Load(int2(0, 0));
    int4 b = texI.Load(int2(3, 0));
    outputBuffer[0] = a.x;
    outputBuffer[1] = a.y;
    outputBuffer[2] = a.z;
    outputBuffer[3] = a.w;
    outputBuffer[4] = b.x;
    outputBuffer[5] = b.y;
    outputBuffer[6] = b.z;
    outputBuffer[7] = b.w;
}

//CHECK: type: int32_t
// texI, texel 0
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}1{{$}}
// texI, texel 3
//CHECK-NEXT: {{^}}1{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}1{{$}}
