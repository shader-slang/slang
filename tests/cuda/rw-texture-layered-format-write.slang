//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

// Each texture's format differs from its element type, which forces the converting layered-write
// path (`surf{1D,2D}Layeredwrite_convert`). The runtime check is
// tools/gfx-unit-test/cuda-layered-format-write.cpp.

[format("rgba8")] RWTexture2DArray<float4> a2;
[format("rg16f")] RWTexture2DArray<float2> a2h;
[format("r16f")] RWTexture2DArray<float> a2s;
[format("rgba8ui")] RWTexture2DArray<uint4> a2u;
[format("r8i")] RWTexture2DArray<int> a2i;
[format("rgba8")] RWTexture1DArray<float4> a1;
[format("rg16f")] RWTexture1DArray<float2> a1h;
[format("r16f")] RWTexture1DArray<float> a1s;
[format("rg8ui")] RWTexture1DArray<uint2> a1u;
[format("rgba8i")] RWTexture1DArray<int4> a1i;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CUDA: surf2DLayeredwrite_convert<float4 >({{.*}}a2{{.*}}.x * 1, {{.*}}.y, {{.*}}.z,
    // PTX: sust.p.a2d.v4.b32.zero [{{%rd[0-9]+}}, {{{%r[0-9]+}}, {{%r[0-9]+}}, [[Y:%r[0-9]+]], [[Y]]}], {{{.*}}, {{.*}}, {{.*}}, {{.*}}}
    a2[uint3(3, 5, 7)] = float4(0.25, 0.5, 0.75, 1.0);

    // CUDA: surf2DLayeredwrite_convert<float2 >
    // PTX: sust.p.a2d.v2.b32.zero
    a2h[uint3(3, 5, 7)] = float2(0.25, 0.5);

    // CUDA: surf2DLayeredwrite_convert<float>
    // PTX: sust.p.a2d.b32.zero
    a2s.Store(uint3(3, 5, 7), 0.25);

    // CUDA: surf2DLayeredwrite_convert<uint4 >
    // PTX: sust.p.a2d.v4.b32.zero
    a2u[uint3(3, 5, 7)] = uint4(1, 2, 3, 4);

    // CUDA: surf2DLayeredwrite_convert<int>
    // PTX: sust.p.a2d.b32.zero
    a2i[uint3(3, 5, 7)] = -3;

    // CUDA: surf1DLayeredwrite_convert<float4 >({{.*}}a1{{.*}}.x * 1, {{.*}}.y,
    // PTX: sust.p.a1d.v4.b32.zero [{{%rd[0-9]+}}, {{{%r[0-9]+}}, {{%r[0-9]+}}}], {{{.*}}, {{.*}}, {{.*}}, {{.*}}}
    a1[uint2(9, 11)] = float4(0.25, 0.5, 0.75, 1.0);

    // CUDA: surf1DLayeredwrite_convert<float2 >
    // PTX: sust.p.a1d.v2.b32.zero
    a1h[uint2(9, 11)] = float2(0.25, 0.5);

    // CUDA: surf1DLayeredwrite_convert<float>
    // PTX: sust.p.a1d.b32.zero
    a1s[uint2(9, 11)] = 0.25;

    // CUDA: surf1DLayeredwrite_convert<uint2 >
    // PTX: sust.p.a1d.v2.b32.zero
    a1u[uint2(9, 11)] = uint2(1, 2);

    // CUDA: surf1DLayeredwrite_convert<int4 >
    // PTX: sust.p.a1d.v4.b32.zero
    a1i[uint2(9, 11)] = int4(-1, -2, 3, 4);
}
