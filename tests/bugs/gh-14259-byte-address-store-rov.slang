// gh-14259-byte-address-store-rov.slang

// DXC rejects explicit template arguments on the methods of `RasterizerOrderedByteAddressBuffer`,
// so a store to one cannot name its value type with `Store<T >`. The store calls the untemplated
// `Store` and converts the value to its own type, so that DXC still writes a `double` or `half`
// constant with its own width.

//TEST:SIMPLE(filecheck=ROV):-target hlsl -entry rovMain -stage fragment -profile ps_6_6
//TEST:SIMPLE(filecheck=ROV_DXIL):-target dxil-asm -entry rovMain -stage fragment -profile ps_6_6

RasterizerOrderedByteAddressBuffer rov;

[shader("fragment")]
float4 rovMain(float4 pos : SV_Position) : SV_Target
{
    // ROV-LABEL: rovMain
    // ROV-NOT: Store<
    // ROV: rov_0.Store(0U,double(42.0));
    // ROV: rov_0.Store(8U,half(1.5));
    // ROV: rov_0.Store(12U,int(
    // ROV: rov_0.Store2(16U,
    // ROV: rov_0.Store(24U,float2(
    // ROV-NOT: Store<

    // ROV_DXIL: rawBufferStore.f64({{.*}}, i32 0, i32 undef, double 4.200000e+01,
    // ROV_DXIL: rawBufferStore.f16({{.*}}, i32 8, i32 undef, half 0xH3E00,
    // ROV_DXIL: rawBufferStore.i32({{.*}}, i32 12, i32 undef,
    // ROV_DXIL: rawBufferStore.i32({{.*}}, i32 16, i32 undef, i32 6, i32 7,
    // ROV_DXIL: rawBufferStore.f32({{.*}}, i32 24, i32 undef, float
    rov.Store<double>(0, double(42.0));
    rov.Store<half>(8, half(1.5));
    rov.Store<int>(12, int(pos.x));
    rov.Store<uint2>(16, uint2(6, 7));
    rov.Store<float2>(24, pos.xy);
    return 0;
}
