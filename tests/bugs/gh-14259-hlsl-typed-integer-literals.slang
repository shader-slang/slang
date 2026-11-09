// The HLSL emitter writes every integer literal so that its spelling alone has the literal's type:
// `U`, `L`, `LL` and `ULL` suffixes, and a cast for `int16_t` and `uint16_t`, which have no suffix,
// and for an `int` zero, because dxc types `0L` as `uint`. Slang writes these stores without a
// template argument, so dxc deduces the `T` of `RWByteAddressBuffer.Store<T>` from the literal,
// and the width of each store shows the literal's type. The shifts by a run-time amount show
// signedness: they are `ashr` for a signed literal and `lshr` for an unsigned one. The vk lane
// compiles to SPIR-V rather than HLSL, so it only cross-checks the expected buffer words.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL):-target dxil-asm -entry computeMain -stage compute -profile cs_6_6
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -profile cs_6_6 -render-features int16,int64 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -render-features int16,int64 -shaderobj

//TEST_INPUT:ubuffer(data=[0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA], stride=4):out,name=output
RWByteAddressBuffer output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // The shift amount is 1, but it depends on the thread ID, so Slang does not fold the shifts.
    uint s = tid.x + 1;

    // HLSL-LABEL: void computeMain
    // HLSL: output_0.Store(0U,(-9223372036854775807LL - 1LL));
    // HLSL: output_0.Store(8U,9223372036854775807LL);
    // HLSL: output_0.Store(16U,0LL);
    // HLSL: output_0.Store(24U,5000000000LL);
    // HLSL: output_0.Store(32U,-5000000000LL);
    // HLSL: output_0.Store(40U,0ULL);
    // HLSL: output_0.Store(48U,18446744073709551615ULL);
    // HLSL: output_0.Store(56U,10000000000ULL);
    // HLSL: output_0.Store(64U,(-2147483647L - 1L));
    // HLSL: output_0.Store(68U,2147483647L);
    // HLSL: output_0.Store(72U,int(0));
    // HLSL: output_0.Store(76U,12345L);
    // HLSL: output_0.Store(80U,-12345L);
    // HLSL: output_0.Store(84U,0U);
    // HLSL: output_0.Store(88U,4294967295U);
    // HLSL: output_0.Store(92U,12345U);
    // HLSL: output_0.Store(96U,false);
    // HLSL: output_0.Store(100U,true);
    // HLSL: output_0.Store(104U,int16_t(-32768));
    // HLSL: output_0.Store(106U,int16_t(32767));
    // HLSL: output_0.Store(108U,int16_t(0));
    // HLSL: output_0.Store(110U,int16_t(863));
    // HLSL: output_0.Store(112U,int16_t(-863));
    // HLSL: output_0.Store(114U,uint16_t(0U));
    // HLSL: output_0.Store(116U,uint16_t(65535U));
    // HLSL: output_0.Store(118U,uint16_t(4660U));
    // HLSL: output_0.Store(120U,-12345L >> s_0);
    // HLSL: output_0.Store(124U,4294967295U >> s_0);
    // HLSL: output_0.Store(128U,-5000000000LL >> s_0);
    // HLSL: output_0.Store(136U,18446744073709551615ULL >> s_0);
    // HLSL: output_0.Store(144U,int16_t(-863) >> s_0);
    // HLSL: output_0.Store(146U,uint16_t(65535U) >> s_0);
    // HLSL: output_0.Store(148U,(int(0) - int(s_0)) >> 1L);

    // DXIL: rawBufferStore.i64({{.*}}, i32 0, i32 undef, i64 -9223372036854775808,
    // DXIL: rawBufferStore.i64({{.*}}, i32 8, i32 undef, i64 9223372036854775807,
    // DXIL: rawBufferStore.i64({{.*}}, i32 16, i32 undef, i64 0,
    // DXIL: rawBufferStore.i64({{.*}}, i32 24, i32 undef, i64 5000000000,
    // DXIL: rawBufferStore.i64({{.*}}, i32 32, i32 undef, i64 -5000000000,
    // DXIL: rawBufferStore.i64({{.*}}, i32 40, i32 undef, i64 0,
    // DXIL: rawBufferStore.i64({{.*}}, i32 48, i32 undef, i64 -1,
    // DXIL: rawBufferStore.i64({{.*}}, i32 56, i32 undef, i64 10000000000,
    // DXIL: rawBufferStore.i32({{.*}}, i32 64, i32 undef, i32 -2147483648,
    // DXIL: rawBufferStore.i32({{.*}}, i32 68, i32 undef, i32 2147483647,
    // DXIL: rawBufferStore.i32({{.*}}, i32 72, i32 undef, i32 0,
    // DXIL: rawBufferStore.i32({{.*}}, i32 76, i32 undef, i32 12345,
    // DXIL: rawBufferStore.i32({{.*}}, i32 80, i32 undef, i32 -12345,
    // DXIL: rawBufferStore.i32({{.*}}, i32 84, i32 undef, i32 0,
    // DXIL: rawBufferStore.i32({{.*}}, i32 88, i32 undef, i32 -1,
    // DXIL: rawBufferStore.i32({{.*}}, i32 92, i32 undef, i32 12345,
    // DXIL: rawBufferStore.i32({{.*}}, i32 96, i32 undef, i32 0,
    // DXIL: rawBufferStore.i32({{.*}}, i32 100, i32 undef, i32 1,
    // DXIL: rawBufferStore.i16({{.*}}, i32 104, i32 undef, i16 -32768,
    // DXIL: rawBufferStore.i16({{.*}}, i32 106, i32 undef, i16 32767,
    // DXIL: rawBufferStore.i16({{.*}}, i32 108, i32 undef, i16 0,
    // DXIL: rawBufferStore.i16({{.*}}, i32 110, i32 undef, i16 863,
    // DXIL: rawBufferStore.i16({{.*}}, i32 112, i32 undef, i16 -863,
    // DXIL: rawBufferStore.i16({{.*}}, i32 114, i32 undef, i16 0,
    // DXIL: rawBufferStore.i16({{.*}}, i32 116, i32 undef, i16 -1,
    // DXIL: rawBufferStore.i16({{.*}}, i32 118, i32 undef, i16 4660,
    // DXIL: ashr i32 -12345,
    // DXIL: lshr i32 -1,
    // DXIL: ashr i64 -5000000000,
    // DXIL: lshr i64 -1,
    // DXIL: ashr i16 -863,
    // DXIL: lshr i16 -1,
    // DXIL: ashr i32 %{{.*}}, 1

    output.Store<int64_t>(0, int64_t.minValue);
    output.Store<int64_t>(8, int64_t.maxValue);
    output.Store<int64_t>(16, int64_t(0));
    output.Store<int64_t>(24, int64_t(5000000000));
    output.Store<int64_t>(32, int64_t(-5000000000));
    output.Store<uint64_t>(40, uint64_t(0));
    output.Store<uint64_t>(48, uint64_t.maxValue);
    output.Store<uint64_t>(56, uint64_t(10000000000));
    output.Store<int>(64, int.minValue);
    output.Store<int>(68, int.maxValue);
    output.Store<int>(72, int(0));
    output.Store<int>(76, int(12345));
    output.Store<int>(80, int(-12345));
    output.Store<uint>(84, uint(0));
    output.Store<uint>(88, uint.maxValue);
    output.Store<uint>(92, uint(12345));
    output.Store<bool>(96, false);
    output.Store<bool>(100, true);
    output.Store<int16_t>(104, int16_t.minValue);
    output.Store<int16_t>(106, int16_t.maxValue);
    output.Store<int16_t>(108, int16_t(0));
    output.Store<int16_t>(110, int16_t(863));
    output.Store<int16_t>(112, int16_t(-863));
    output.Store<uint16_t>(114, uint16_t(0));
    output.Store<uint16_t>(116, uint16_t.maxValue);
    output.Store<uint16_t>(118, uint16_t(4660));
    output.Store<int>(120, int(-12345) >> s);
    output.Store<uint>(124, uint.maxValue >> s);
    output.Store<int64_t>(128, int64_t(-5000000000) >> s);
    output.Store<uint64_t>(136, uint64_t.maxValue >> s);
    output.Store<int16_t>(144, int16_t(-863) >> s);
    output.Store<uint16_t>(146, uint16_t.maxValue >> s);
    output.Store<int>(148, (0 - int(s)) >> 1);

    // Every word starts as the sentinel `AAAAAAAA`, so a store of the wrong width shows up as a
    // word that keeps it or as a neighbour that it overwrites. The last word is never written.
    // BUF: 0
    // BUF-NEXT: 80000000
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: 7FFFFFFF
    // BUF-NEXT: 0
    // BUF-NEXT: 0
    // BUF-NEXT: 2A05F200
    // BUF-NEXT: 1
    // BUF-NEXT: D5FA0E00
    // BUF-NEXT: FFFFFFFE
    // BUF-NEXT: 0
    // BUF-NEXT: 0
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: 540BE400
    // BUF-NEXT: 2
    // BUF-NEXT: 80000000
    // BUF-NEXT: 7FFFFFFF
    // BUF-NEXT: 0
    // BUF-NEXT: 3039
    // BUF-NEXT: FFFFCFC7
    // BUF-NEXT: 0
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: 3039
    // BUF-NEXT: 0
    // BUF-NEXT: 1
    // BUF-NEXT: 7FFF8000
    // BUF-NEXT: 35F0000
    // BUF-NEXT: FCA1
    // BUF-NEXT: 1234FFFF
    // BUF-NEXT: FFFFE7E3
    // BUF-NEXT: 7FFFFFFF
    // BUF-NEXT: 6AFD0700
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: 7FFFFFFF
    // BUF-NEXT: 7FFFFE50
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: AAAAAAAA
}
