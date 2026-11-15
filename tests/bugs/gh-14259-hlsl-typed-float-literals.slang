// The HLSL emitter writes every `half`, `float` and `double` literal with the `h`, `f` or `L`
// suffix, and a NaN or infinity as a division in those literals, such as `(0.0L / 0.0L)`. Slang
// writes these stores without a template argument, so dxc deduces the `T` of
// `RWByteAddressBuffer.Store<T>` from the literal, and the width of each store shows the literal's
// type. Each type stores its highest and lowest finite values, its smallest positive and negative
// denormals, `0.0`, `-0.0`, +Inf, -Inf and NaN. The vk lane compiles to SPIR-V rather than HLSL,
// so it only cross-checks the expected buffer words. Below SM 6.2, Slang does not enable 16-bit
// types, so a `half` is a 32-bit type and the `h` literals compile to 32-bit stores.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL):-target dxil-asm -entry computeMain -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL60):-target dxil-asm -entry computeMain -stage compute -profile cs_6_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -profile cs_6_6 -render-features half,double -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -render-features half,double -shaderobj

//TEST_INPUT:ubuffer(data=[0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA], stride=4):out,name=output
RWByteAddressBuffer output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // HLSL-LABEL: void computeMain
    // HLSL: output_0.Store(0U,1.79769313486231571e+308L);
    // HLSL: output_0.Store(8U,-1.79769313486231571e+308L);
    // HLSL: output_0.Store(16U,4.94065645841246544e-324L);
    // HLSL: output_0.Store(24U,-4.94065645841246544e-324L);
    // HLSL: output_0.Store(32U,0.0L);
    // HLSL: output_0.Store(40U,-0.0L);
    // HLSL: output_0.Store(48U,(1.0L / 0.0L));
    // HLSL: output_0.Store(56U,(-1.0L / 0.0L));
    // HLSL: output_0.Store(64U,(0.0L / 0.0L));
    // HLSL: output_0.Store(72U,3.4028234663852886e+38f);
    // HLSL: output_0.Store(76U,-3.4028234663852886e+38f);
    // HLSL: output_0.Store(80U,1.40129846432481707e-45f);
    // HLSL: output_0.Store(84U,-1.40129846432481707e-45f);
    // HLSL: output_0.Store(88U,0.0f);
    // HLSL: output_0.Store(92U,-0.0f);
    // HLSL: output_0.Store(96U,(1.0f / 0.0f));
    // HLSL: output_0.Store(100U,(-1.0f / 0.0f));
    // HLSL: output_0.Store(104U,(0.0f / 0.0f));
    // HLSL: output_0.Store(108U,65504.0h);
    // HLSL: output_0.Store(110U,-65504.0h);
    // HLSL: output_0.Store(112U,5.9604644775390625e-08h);
    // HLSL: output_0.Store(114U,-5.9604644775390625e-08h);
    // HLSL: output_0.Store(116U,0.0h);
    // HLSL: output_0.Store(118U,-0.0h);
    // HLSL: output_0.Store(120U,(1.0h / 0.0h));
    // HLSL: output_0.Store(122U,(-1.0h / 0.0h));
    // HLSL: output_0.Store(124U,(0.0h / 0.0h));

    // DXIL: rawBufferStore.f64({{.*}}, i32 0, i32 undef, double 0x7FEFFFFFFFFFFFFF,
    // DXIL: rawBufferStore.f64({{.*}}, i32 8, i32 undef, double 0xFFEFFFFFFFFFFFFF,
    // DXIL: rawBufferStore.f64({{.*}}, i32 16, i32 undef, double 4.940656e-324,
    // DXIL: rawBufferStore.f64({{.*}}, i32 24, i32 undef, double -4.940656e-324,
    // DXIL: rawBufferStore.f64({{.*}}, i32 32, i32 undef, double 0.000000e+00,
    // DXIL: rawBufferStore.f64({{.*}}, i32 40, i32 undef, double -0.000000e+00,
    // DXIL: rawBufferStore.f64({{.*}}, i32 48, i32 undef, double 0x7FF0000000000000,
    // DXIL: rawBufferStore.f64({{.*}}, i32 56, i32 undef, double 0xFFF0000000000000,
    // DXIL: rawBufferStore.f64({{.*}}, i32 64, i32 undef, double 0x{{7|F}}FF8000000000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 72, i32 undef, float 0x47EFFFFFE0000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 76, i32 undef, float 0xC7EFFFFFE0000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 80, i32 undef, float 0x36A0000000000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 84, i32 undef, float 0xB6A0000000000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 88, i32 undef, float 0.000000e+00,
    // DXIL: rawBufferStore.f32({{.*}}, i32 92, i32 undef, float -0.000000e+00,
    // DXIL: rawBufferStore.f32({{.*}}, i32 96, i32 undef, float 0x7FF0000000000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 100, i32 undef, float 0xFFF0000000000000,
    // DXIL: rawBufferStore.f32({{.*}}, i32 104, i32 undef, float 0x{{7|F}}FF8000000000000,
    // DXIL: rawBufferStore.f16({{.*}}, i32 108, i32 undef, half 0xH7BFF,
    // DXIL: rawBufferStore.f16({{.*}}, i32 110, i32 undef, half 0xHFBFF,
    // DXIL: rawBufferStore.f16({{.*}}, i32 112, i32 undef, half 0xH0001,
    // DXIL: rawBufferStore.f16({{.*}}, i32 114, i32 undef, half 0xH8001,
    // DXIL: rawBufferStore.f16({{.*}}, i32 116, i32 undef, half 0xH0000,
    // DXIL: rawBufferStore.f16({{.*}}, i32 118, i32 undef, half 0xH8000,
    // DXIL: rawBufferStore.f16({{.*}}, i32 120, i32 undef, half 0xH7C00,
    // DXIL: rawBufferStore.f16({{.*}}, i32 122, i32 undef, half 0xHFC00,
    // DXIL: rawBufferStore.f16({{.*}}, i32 124, i32 undef, half 0xH{{7|F}}E00,

    // DXIL60: bufferStore.f32({{.*}}, i32 108, i32 undef, float 6.550400e+04,
    // DXIL60: bufferStore.f32({{.*}}, i32 118, i32 undef, float -0.000000e+00,
    // DXIL60: bufferStore.f32({{.*}}, i32 120, i32 undef, float 0x7FF0000000000000,

    output.Store<double>(0, 1.7976931348623157e+308l);
    output.Store<double>(8, -1.7976931348623157e+308l);
    output.Store<double>(16, 4.9406564584124654e-324l);
    output.Store<double>(24, -4.9406564584124654e-324l);
    output.Store<double>(32, 0.0l);
    output.Store<double>(40, -0.0l);
    output.Store<double>(48, double(1.0 / 0.0));
    output.Store<double>(56, double(-1.0 / 0.0));
    output.Store<double>(64, double(0.0 / 0.0));
    output.Store<float>(72, 3.4028234663852886e+38f);
    output.Store<float>(76, -3.4028234663852886e+38f);
    output.Store<float>(80, 1.4012984643248171e-45f);
    output.Store<float>(84, -1.4012984643248171e-45f);
    output.Store<float>(88, 0.0f);
    output.Store<float>(92, -0.0f);
    output.Store<float>(96, 1.0f / 0.0f);
    output.Store<float>(100, -1.0f / 0.0f);
    output.Store<float>(104, 0.0f / 0.0f);
    output.Store<half>(108, half(65504.0));
    output.Store<half>(110, half(-65504.0));
    output.Store<half>(112, half(5.9604644775390625e-08));
    output.Store<half>(114, half(-5.9604644775390625e-08));
    output.Store<half>(116, half(0.0));
    output.Store<half>(118, half(-0.0));
    output.Store<half>(120, half(1.0 / 0.0));
    output.Store<half>(122, half(-1.0 / 0.0));
    output.Store<half>(124, half(0.0 / 0.0));

    // Every word starts as the sentinel `AAAAAAAA`, so a store of the wrong width shows up as a
    // word that keeps it or as a neighbour that it overwrites. Words 0-17 hold the `double` values,
    // words 18-26 the `float` values, and words 27-31 the `half` values, two to a word; the upper
    // half of word 31 and all of word 32 are never written. The `-0.0` words are checked bit for
    // bit, so they check the sign bit. The `float` and `double` NaNs are checked as the default quiet
    // NaN of either sign. The `half` NaN accepts any NaN, because its sign and payload differ between
    // targets.
    // BUF: FFFFFFFF
    // BUF-NEXT: 7FEFFFFF
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: FFEFFFFF
    // BUF-NEXT: 1
    // BUF-NEXT: 0
    // BUF-NEXT: 1
    // BUF-NEXT: 80000000
    // BUF-NEXT: 0
    // BUF-NEXT: 0
    // BUF-NEXT: 0
    // BUF-NEXT: 80000000
    // BUF-NEXT: 0
    // BUF-NEXT: 7FF00000
    // BUF-NEXT: 0
    // BUF-NEXT: FFF00000
    // BUF-NEXT: 0
    // BUF-NEXT: {{7|F}}FF80000
    // BUF-NEXT: 7F7FFFFF
    // BUF-NEXT: FF7FFFFF
    // BUF-NEXT: 1
    // BUF-NEXT: 80000001
    // BUF-NEXT: 0
    // BUF-NEXT: 80000000
    // BUF-NEXT: 7F800000
    // BUF-NEXT: FF800000
    // BUF-NEXT: {{7|F}}FC00000
    // BUF-NEXT: FBFF7BFF
    // BUF-NEXT: 80010001
    // BUF-NEXT: 80000000
    // BUF-NEXT: FC007C00
    // BUF-NEXT: AAAA{{[7F](C0[1-9A-F]|C[1-9A-F][0-9A-F]|[D-F][0-9A-F][0-9A-F])}}
    // BUF-NEXT: AAAAAAAA
}
