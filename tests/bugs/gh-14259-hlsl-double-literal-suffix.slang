// gh-14259-hlsl-double-literal-suffix.slang

// The HLSL emitter writes a finite `double` literal with the `L` suffix; "finite" excludes NaN and
// the infinities, which are spelled as `float` expressions. HLSL reads an unsuffixed
// floating-point literal as a 32-bit `float`, so a template argument deduced from it, such as the
// `T` of `RWByteAddressBuffer.Store<T>`, would be `float`, and the store would write four bytes of
// a `float` value instead of the eight bytes of the `double`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL):-target dxil-asm -entry computeMain -stage compute -profile cs_6_6
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -profile cs_6_6 -render-feature double -shaderobj

//TEST_INPUT:ubuffer(data=[0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA], stride=4):out,name=output
RWByteAddressBuffer output;

static const double kScale = -0.6473313946860445l;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // HLSL-LABEL: void computeMain
    // HLSL: output_0.Store(0U,42.0L);
    // HLSL: output_0.Store(8U,-0.6473313946860445L);
    // HLSL: output_0.Store(16U,{{[0-9.]+}}e+300L);
    // HLSL: output_0.Store(24U,2.5f);

    // DXIL: rawBufferStore.f64({{.*}}, i32 0, i32 undef, double 4.200000e+01,
    // DXIL: rawBufferStore.f64({{.*}}, i32 8, i32 undef, double 0xBFE4B6F0543B385B,
    // DXIL: rawBufferStore.f64({{.*}}, i32 16, i32 undef, double 1.000000e+300,
    // DXIL: rawBufferStore.f32({{.*}}, i32 24, i32 undef, float 2.500000e+00,

    // A `float` literal converted to `double`, folded to a `double` constant.
    output.Store<double>(0, double(42.0));
    // A `static const` whose digits do not fit in a `float`.
    output.Store<double>(8, kScale);
    // A `double` literal whose value would overflow a `float`.
    output.Store<double>(16, 1e300l);
    output.Store<float>(24, 2.5f);

    // Every word starts as the sentinel `AAAAAAAA`, so a store of the wrong width shows up as a
    // word that keeps it. Words 0-1 hold 42.0, words 2-3 `kScale`, words 4-5 1e300, word 6 the
    // `float` 2.5, and word 7 is never written.
    // BUF: 0
    // BUF-NEXT: 40450000
    // BUF-NEXT: 543B385B
    // BUF-NEXT: BFE4B6F0
    // BUF-NEXT: 8800759C
    // BUF-NEXT: 7E37E43C
    // BUF-NEXT: 40200000
    // BUF-NEXT: AAAAAAAA
}
