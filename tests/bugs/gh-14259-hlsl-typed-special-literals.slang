// gh-14259-hlsl-typed-special-literals.slang

// The HLSL emitter casts a `half` literal and every NaN or infinity to its type. An unsuffixed
// literal or a division of literals is a 32-bit `float` in HLSL, and dxc deduces `float` from one
// for the `T` of `RWByteAddressBuffer.Store<T>`, which would store four bytes for a `half` or a
// `double`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL):-target dxil-asm -entry computeMain -stage compute -profile cs_6_6
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -profile cs_6_6 -render-features half,double -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -render-features half -shaderobj

//TEST_INPUT:ubuffer(data=[0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA], stride=4):out,name=output
RWByteAddressBuffer output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // HLSL-LABEL: void computeMain
    // HLSL: output_0.Store(0U,double(1.0L / 0.0L));
    // HLSL: output_0.Store(8U,double(-1.0L / 0.0L));
    // HLSL: output_0.Store(16U,double(0.0L / 0.0L));
    // HLSL: output_0.Store(24U,float(1.0f / 0.0f));
    // HLSL: output_0.Store(28U,float(-1.0f / 0.0f));
    // HLSL: output_0.Store(32U,float(0.0f / 0.0f));
    // HLSL: output_0.Store(36U,half(1.5));
    // HLSL: output_0.Store(38U,half(1.0 / 0.0));
    // HLSL: output_0.Store(40U,half(-1.0 / 0.0));
    // HLSL: output_0.Store(42U,half(0.0 / 0.0));

    // DXIL: rawBufferStore.f64({{.*}}, i32 0, i32 undef, double 0x7FF0000000000000,
    // DXIL: rawBufferStore.f64({{.*}}, i32 8, i32 undef, double 0xFFF0000000000000,
    // DXIL: rawBufferStore.f64({{.*}}, i32 16, i32 undef, double 0x{{7|F}}FF8000000000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 24, i32 undef, float 0x7FF0000000000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 28, i32 undef, float 0xFFF0000000000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 32, i32 undef, float 0x{{7|F}}FF8000000000000,
    // DXIL: rawBufferStore.f16({{.*}}, i32 36, i32 undef, half 0xH3E00,
    // DXIL: rawBufferStore.f16({{.*}}, i32 38, i32 undef, half 0xH7C00,
    // DXIL: rawBufferStore.f16({{.*}}, i32 40, i32 undef, half 0xHFC00,
    // DXIL: rawBufferStore.f16({{.*}}, i32 42, i32 undef, half 0xH{{7|F}}E00,

    output.Store<double>(0, double(1.0 / 0.0));
    output.Store<double>(8, double(-1.0 / 0.0));
    output.Store<double>(16, double(0.0 / 0.0));
    output.Store<float>(24, 1.0f / 0.0f);
    output.Store<float>(28, -1.0f / 0.0f);
    output.Store<float>(32, 0.0f / 0.0f);
    output.Store<half>(36, half(1.5));
    output.Store<half>(38, half(1.0 / 0.0));
    output.Store<half>(40, half(-1.0 / 0.0));
    output.Store<half>(42, half(0.0 / 0.0));

    // Every word starts as the sentinel `AAAAAAAA`, so a store of the wrong width shows up as a
    // word that keeps it or as a neighbour that it overwrites. Words 0-5 hold the three `double`
    // values, words 6-8 the three `float` values, word 9 the `half` 1.5 and +Inf, word 10 the
    // `half` -Inf and NaN, and word 11 is never written. The sign and payload of a NaN are not
    // checked.
    // BUF: 0
    // BUF-NEXT: 7FF00000
    // BUF-NEXT: 0
    // BUF-NEXT: FFF00000
    // BUF-NEXT: 0
    // BUF-NEXT: {{7|F}}FF80000
    // BUF-NEXT: 7F800000
    // BUF-NEXT: FF800000
    // BUF-NEXT: {{7|F}}FC00000
    // BUF-NEXT: 7C003E00
    // BUF-NEXT: {{[7F](C0[1-9A-F]|C[1-9A-F][0-9A-F]|[D-F][0-9A-F][0-9A-F])}}FC00
    // BUF-NEXT: AAAAAAAA
}
