//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes how the CUDA/OptiX varying-parameter legalizer packs a mixed-width ray payload struct into 32-bit payload registers, sub-word packing the bool/uint8/half/int16 fields, splitting each vector, matrix and array element across one register, and spanning double and int64 over a register pair.
//META: intent=characterization
//META: covers=source/slang/slang-ir-legalize-varying-params.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `emitOptiXPayloadRead` / `emitOptiXPayloadWrite`
// (slang-ir-legalize-varying-params.cpp:1195 and :1424). OptiX has no payload
// memory, only up to 32 opaque 32-bit registers, so an `inout` payload struct
// small enough to fit is decomposed field by field into
// `optixGetPayload_N` / `optixSetPayload_N` pairs at entry and exit. Each
// field kind takes a different route through the packer, and this payload
// exercises them in one shot:
//   * `bool`, `uint8_t`, `half` and `int16_t` are sub-word, so several share
//     one register and are extracted with a shift plus a 0xFF / 0xFFFF mask,
//     with the write side doing a read-modify-write to preserve neighbours;
//   * `uint` occupies a register directly, `float` needs a bit-cast;
//   * `float3`, `float2x2` and `float[2]` recurse element-by-element, so
//     their registers are consecutive and in declaration order;
//   * `double` and `int64_t` need two registers each, recombined as
//     `(high << 32) | low`.
// CHECK asserts the register index each field lands on and the exact
// extract/recombine expression, which together pin the byte-offset arithmetic
// the packer performs. A change in field alignment or ordering moves the
// indices and fails here.
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry chit -stage closesthit

struct Inner
{
    double d;
    int64_t i64;
}
struct Payload
{
    bool flag;
    uint8_t b8;
    half h;
    int16_t i16;
    uint u32;
    float3 v3;
    float2x2 m22;
    float arr[2];
    Inner inner;
}

[shader("closesthit")]
void chit(inout Payload p, in BuiltInTriangleIntersectionAttributes attr)
{
    p.flag = !p.flag;
    p.b8 = p.b8 + uint8_t(1);
    p.h = p.h + half(1);
    p.i16 = p.i16 + int16_t(1);
    p.u32 = p.u32 + 1;
    p.v3 = p.v3 + float3(attr.barycentrics, 0);
    p.m22 = p.m22 + float2x2(1, 2, 3, 4);
    p.arr[0] = p.arr[0] + 1;
    p.arr[1] = p.arr[1] + 2;
    p.inner.d = p.inner.d + 1.0;
    p.inner.i64 = p.inner.i64 + 1;
}

// Read side: bool / uint8 / half share register 0 at byte offsets 0, 1 and 2;
// int16 starts register 1 because of its 2-byte alignment.
//CUDA: bool _S{{[0-9]+}} = bool(_S{{[0-9]+}} & 255U);
//CUDA: uchar _S{{[0-9]+}} = uchar((_S{{[0-9]+}} >> 8U) & 255U);
//CUDA: __half _S{{[0-9]+}} = (slang_bit_cast<__half>(ushort((_S{{[0-9]+}} >> 16U) & 65535U)));
//CUDA: short _S{{[0-9]+}} = short(_S{{[0-9]+}} & 65535U);
// 64-bit fields are read as a register pair and recombined high:low.
//CUDA: double _S{{[0-9]+}} = (slang_bit_cast<double>((ulonglong(_S{{[0-9]+}}) << 32ULL) | ulonglong(_S{{[0-9]+}})));
//CUDA: (slang_bit_cast<longlong>((ulonglong(_S{{[0-9]+}}) << 32ULL) | ulonglong(_S{{[0-9]+}})))

// Write side: the sub-word fields are read-modify-written into register 0/1...
//CUDA: optixSetPayload_0((_S{{[0-9]+}} & 4294967040U) | _S{{[0-9]+}});
//CUDA: optixSetPayload_0((_S{{[0-9]+}} & 4294902015U) | _S{{[0-9]+}});
//CUDA: optixSetPayload_0((_S{{[0-9]+}} & 65535U) | _S{{[0-9]+}});
//CUDA: optixSetPayload_1((_S{{[0-9]+}} & 4294901760U) | _S{{[0-9]+}});
// ...the uint takes a whole register directly...
//CUDA: optixSetPayload_2(_S{{[0-9]+}}.u32_0);
// ...vector, matrix and array elements each take consecutive registers...
//CUDA: optixSetPayload_3((slang_bit_cast<uint>(_S{{[0-9]+}}.v3_0.x)));
//CUDA: optixSetPayload_5((slang_bit_cast<uint>(_S{{[0-9]+}}.v3_0.z)));
//CUDA: optixSetPayload_6((slang_bit_cast<uint>(_S{{[0-9]+}}.m22_0.rows[int(0)].x)));
//CUDA: optixSetPayload_9((slang_bit_cast<uint>(_S{{[0-9]+}}.m22_0.rows[int(1)].y)));
//CUDA: optixSetPayload_10((slang_bit_cast<uint>(_S{{[0-9]+}}.arr_0[int(0)])));
//CUDA: optixSetPayload_11((slang_bit_cast<uint>(_S{{[0-9]+}}.arr_0[int(1)])));
// ...and the nested struct's two 64-bit fields take registers 12-15.
//CUDA: optixSetPayload_12(
//CUDA: optixSetPayload_15(
