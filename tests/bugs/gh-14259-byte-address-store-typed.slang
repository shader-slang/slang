// gh-14259-byte-address-store-typed.slang

// A byte-address buffer store names its value type in the emitted HLSL, C++ and CUDA. Without
// the explicit type, each of those compilers would deduce the template argument of `Store` from
// the spelling of the value, and some constants are spelled with another type: HLSL writes a
// `double` or `half` constant as a bare literal such as `42.0`, which dxc deduces as `float`, and
// C++ and CUDA write a `double` infinity as the `float` macro `SLANG_INFINITY` and a `float` NaN
// as the `double` expression `(0.0 / 0.0)`. A `uint` or `uintN` value uses the untemplated
// `Store`/`StoreN` methods, which FXC also accepts.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL):-target dxil-asm -entry computeMain -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=SM50_UINT):-target hlsl -entry sm50UIntMain -stage compute -profile cs_5_0
//TEST:SIMPLE(filecheck=SM50):-target hlsl -entry sm5Main -stage compute -profile cs_5_0
//TEST:SIMPLE(filecheck=SM51):-target hlsl -entry sm5Main -stage compute -profile cs_5_1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -profile cs_6_6 -render-features half -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -render-features half -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -render-features half -shaderobj

//TEST_INPUT:ubuffer(data=[0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA], stride=4):out,name=output
RWByteAddressBuffer output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // HLSL-LABEL: void computeMain
    // HLSL: output_0.Store<double >(0U,42.0);
    // HLSL: output_0.Store<double >(8U,{{.*}});
    // HLSL: output_0.Store<half >(16U,1.5);
    // HLSL: output_0.Store(20U,5U);
    // HLSL: output_0.Store2(24U,{{.*}});
    // HLSL: output_0.Store<float >(32U,2.5f);
    // HLSL: output_0.Store<int >(36U,int(-5));
    // HLSL: output_0.Store<float >(40U,{{.*}});
    // HLSL: output_0.Store4(48U,{{.*}});
    // HLSL: output_0.Store2(64U,{{.*}});
    // HLSL: output_0.Store(72U,31U);

    // DXIL: rawBufferStore.f64({{.*}}, i32 0, i32 undef, double 4.200000e+01,
    // DXIL: rawBufferStore.f64({{.*}}, i32 8, i32 undef, double 0x7FF0000000000000,
    // DXIL: rawBufferStore.f16({{.*}}, i32 16, i32 undef, half 0xH3E00,
    // DXIL: rawBufferStore.i32({{.*}}, i32 20, i32 undef, i32 5,
    // DXIL: rawBufferStore.i32({{.*}}, i32 24, i32 undef, i32 6, i32 7,
    // DXIL: rawBufferStore.f32({{.*}}, i32 32, i32 undef, float 2.500000e+00,
    // DXIL: rawBufferStore.i32({{.*}}, i32 36, i32 undef, i32 -5,

    output.Store<double>(0, double(42.0));
    output.Store<double>(8, double(1.0 / 0.0));
    output.Store<half>(16, half(1.5));
    output.Store<uint>(20, 5);
    output.Store<uint2>(24, uint2(6, 7));
    output.Store<float>(32, 2.5f);
    output.Store<int>(36, -5);
    output.Store<float>(40, float(0.0 / 0.0));
    output.Store<uint4>(48, uint4(11, 12, 13, 14));
    output.Store<uint3>(64, uint3(8, 9, 0x1F));

    // Every word starts as the sentinel `AAAAAAAA`, so a word that a store should not reach keeps
    // it, and a store of the wrong width shows up as a missing or an extra word. A `double`
    // occupies two words. A `half` writes the low two bytes of its word and leaves the high two
    // bytes unchanged. Word 11 follows the 4-byte `float` NaN and stays untouched. The `uint4` at a
    // 16-byte multiple is stored whole; the `uint3` at 64 is split into `Store2` and `Store`. A
    // `uint3` at a non-zero multiple of 12 would be stored whole, but on SPIR-V it lands at the
    // wrong offset.
    // BUF: 0
    // BUF-NEXT: 40450000
    // BUF-NEXT: 0
    // BUF-NEXT: 7FF00000
    // BUF-NEXT: AAAA3E00
    // BUF-NEXT: 5
    // BUF-NEXT: 6
    // BUF-NEXT: 7
    // BUF-NEXT: 40200000
    // BUF-NEXT: FFFFFFFB
    // BUF-NEXT: {{7FC00000|FFC00000|7FFFFFFF|FFFFFFFF}}
    // BUF-NEXT: AAAAAAAA
    // BUF-NEXT: B
    // BUF-NEXT: C
    // BUF-NEXT: D
    // BUF-NEXT: E
    // BUF-NEXT: 8
    // BUF-NEXT: 9
    // BUF-NEXT: 1F
    // BUF-NEXT: AAAAAAAA
}

// With an SM 5.0 profile, a byte-address store of a non-`uint` scalar is legalized to a `uint`
// store of its bits, so these stores need no templated `Store`. `1075838976U` is the bit pattern
// of `2.5f`, and `4294967291U` is that of `-5`.
[shader("compute")]
[numthreads(1, 1, 1)]
void sm50UIntMain()
{
    // SM50_UINT-LABEL: void sm50UIntMain
    // SM50_UINT-NOT: Store<
    // SM50_UINT: output_0.Store(0U,1075838976U);
    // SM50_UINT-NOT: Store<
    // SM50_UINT: output_0.Store(4U,4294967291U);
    // SM50_UINT-NOT: Store<
    // SM50_UINT: output_0.Store2(8U,uint2(6U, 7U));
    // SM50_UINT-NOT: Store<
    // SM50_UINT: output_0.Store4(16U,{{.*}});
    // SM50_UINT-NOT: Store<
    // SM50_UINT: output_0.Store3(48U,{{.*}});
    // SM50_UINT-NOT: Store<
    output.Store<float>(0, 2.5f);
    output.Store<int>(4, -5);
    output.Store<uint2>(8, uint2(6, 7));
    output.Store<uint4>(16, uint4(11, 12, 13, 14));
    output.Store<uint3>(48, uint3(8, 9, 10));
}

// With an SM 5.x profile, a store that legalization leaves non-`uint` keeps its type and uses
// `Store<T >`, as the loads at these profiles do. On SM 5.0, legalization bit-casts scalars but not
// a vector stored whole at a multiple of its size. On SM 5.1, which is also the default HLSL
// profile, it bit-casts nothing. FXC rejects these `Store<T >` lines, as it rejects the matching
// `Load<T >`; the checks record that known limitation rather than a supported form.
[shader("compute")]
[numthreads(1, 1, 1)]
void sm5Main()
{
    // SM50-LABEL: void sm5Main
    // SM50: output_0.Store(0U,4294967291U);
    // SM50: output_0.Store<float4 >(16U,{{.*}});
    // SM51-LABEL: void sm5Main
    // SM51: output_0.Store<int >(0U,int(-5));
    // SM51: output_0.Store<float4 >(16U,{{.*}});
    output.Store<int>(0, -5);
    output.Store<float4>(16, float4(1, 2, 3, 4));
}
