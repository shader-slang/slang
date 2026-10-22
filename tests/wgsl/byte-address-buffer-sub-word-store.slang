// WGSL byte-address buffers are arrays of 32-bit words, so a store of 8- or 16-bit values
// would have to rewrite the whole word and could lose a concurrent write to its other bytes.
// Such stores are reported at the store, instead of failing later without a location (or
// with an internal error for 8-bit integers).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -stage compute -entry computeMain

struct S
{
    float a;
    half b;
}

RWByteAddressBuffer buf;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    buf.Store<half>(i * 2, half(i));
//CHECK:           ^ byte-address buffer stores of 8- or 16-bit values are not supported in WGSL
//CHECK:           ^ storing 'half' to a byte-address buffer writes 8- or 16-bit values
    buf.Store<half2>(64 + i * 4, half2(half(i), half(1)));
//CHECK:            ^ byte-address buffer stores of 8- or 16-bit values are not supported in WGSL
//CHECK:            ^ storing 'vector<half,2>' to a byte-address buffer writes 8- or 16-bit values
    buf.Store<uint16_t>(256 + i * 2, uint16_t(i));
//CHECK:               ^ byte-address buffer stores of 8- or 16-bit values are not supported in WGSL
//CHECK:               ^ storing 'uint16_t' to a byte-address buffer writes 8- or 16-bit values
    buf.Store<uint8_t>(512 + i, uint8_t(i));
//CHECK:              ^ byte-address buffer stores of 8- or 16-bit values are not supported in WGSL
//CHECK:              ^ storing 'uint8_t' to a byte-address buffer writes 8- or 16-bit values
    buf.Store<S>(1024 + i * 8, { float(i), half(i) });
//CHECK:        ^ byte-address buffer stores of 8- or 16-bit values are not supported in WGSL
//CHECK:        ^ storing 'S' to a byte-address buffer writes 8- or 16-bit values

    half arr[2] = { half(i), half(2) };
    buf.Store<half[2]>(1536 + i * 4, arr);
//CHECK:              ^ byte-address buffer stores of 8- or 16-bit values are not supported in WGSL
//CHECK:              ^ storing 'array<half,2>' to a byte-address buffer writes 8- or 16-bit values

    // 32-bit stores are unaffected, and so are `bool` stores, which are stored as 32-bit words.
    buf.Store<float>(2048 + i * 4, float(i));
    buf.Store<bool>(3072 + i * 4, (i & 1) != 0);
}
