// Read-only `Texture1D.Load` on CUDA must return the stored texels (#12630).
//
// With `content=gradient`, texel x of a size-4 RGBA8Unorm texture is (x/3, 0, 0, 1), so every float
// texel is distinct and a fetch that returns zero or the wrong texel fails the CHECK lines. The
// RGBA32Uint gradient is (x == 3 ? 1 : 0, 0, 0, 1), so the uint loads only tell texel 0 from texel 3.
// The `-cpu` run does not use the CUDA prelude; it confirms the expected values match the upload.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

//TEST_INPUT: Texture1D(format=RGBA8Unorm, size=4, content=gradient, mipMaps=1):name tex4
Texture1D<float4> tex4;
//TEST_INPUT: Texture1D(format=RGBA8Unorm, size=4, content=gradient, mipMaps=1):name tex2
Texture1D<float2> tex2;
//TEST_INPUT: Texture1D(format=RGBA8Unorm, size=4, content=gradient, mipMaps=1):name tex1
Texture1D<float> tex1;
//TEST_INPUT: Texture1D(format=RGBA32Uint, size=4, content=gradient, mipMaps=1):name texU
Texture1D<uint4> texU;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

void store4(int base, float4 v)
{
    outputBuffer[base + 0] = v.x;
    outputBuffer[base + 1] = v.y;
    outputBuffer[base + 2] = v.z;
    outputBuffer[base + 3] = v.w;
}

// The PTX checks below match the prelude's lowering: each 1D load is a 2D texel fetch at row 0.
// The second component of each `Load` location below is the mip level, not a row.
//PTX: tex.level.2d.v4.f32.s32
//PTX: tex.level.2d.v4.u32.s32

[numthreads(1, 1, 1)]
void computeMain()
{
    for (int x = 0; x < 4; x++)
        store4(x * 4, tex4.Load(int2(x, 0)));

    float2 v2 = tex2.Load(int2(1, 0));
    outputBuffer[16] = v2.x;
    outputBuffer[17] = v2.y;

    outputBuffer[18] = tex1.Load(int2(2, 0));
    outputBuffer[19] = tex1.Load(int2(3, 0));

    store4(20, float4(texU.Load(int2(0, 0))));
    store4(24, float4(texU.Load(int2(3, 0))));
}

//CHECK: type: float
// tex4, texels 0..3
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
//CHECK-NEXT: {{^}}0.333333{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
//CHECK-NEXT: {{^}}0.666667{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
// tex2, texel 1
//CHECK-NEXT: {{^}}0.333333{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
// tex1, texels 2 and 3
//CHECK-NEXT: {{^}}0.666667{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
// texU, texels 0 and 3
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}0.000000{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
