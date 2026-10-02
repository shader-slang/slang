// Read-only `Texture1D.Load` of a signed-integer texel on CUDA must return the stored texels (#12630).
//
// With `content=gradient`, texel x of a size-4 RGBA32Sint texture is (x == 3 ? 1 : 0, 0, 0, 1), so
// the two loads tell texel 0 from texel 3 and catch a fetch that returns zero. The CPU backend cannot
// unpack RGBA32Sint, so this runs on CUDA only; `texture1d-load.slang` covers float and uint texels.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

//TEST_INPUT: Texture1D(format=RGBA32Sint, size=4, content=gradient, mipMaps=1):name texI
Texture1D<int4> texI;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// The PTX check below matches the prelude's lowering: the 1D load is a 2D texel fetch at row 0.
// The second component of each `Load` location below is the mip level, not a row.
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
