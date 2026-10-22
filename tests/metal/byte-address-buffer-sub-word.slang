// 8- and 16-bit loads and stores on a byte-address buffer on Metal.
//
// A scalar `Load<half>`/`Store<half>` (and the 8/16-bit integer types) used to be emitted as a
// whole-word access with `as_type` between types of different sizes, which is invalid MSL. A
// store smaller than 4 bytes inside a vector or struct was lowered to a non-atomic
// read-modify-write of the containing word, so neighbouring invocations storing the two halves
// of one word lost one of them. Stores now write only the value's own bytes.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-mtl -compute -shaderobj

// MTL: ((device ushort*)(
// MTL: ((device uchar*)(
// MTL-NOT: as_type<uint32_t>(half
// MTL-NOT: as_type<half>(pool
// LIB: computeMain

// Words: 0 = halves 1.0 (bytes 0-1) and 2.0 (bytes 2-3); 1 = uint16_t 5 and 7; 2 = bytes 0x0A..0x0D.
//TEST_INPUT:ubuffer(data=[1073757184 458757 218893066], stride=4):name=src
ByteAddressBuffer src;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=pool
RWByteAddressBuffer pool;

struct H
{
    half v;
}

[shader("compute")]
[numthreads(8, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;

    // Each pair of neighbouring invocations writes the two halves of one word.
    pool.Store<half>(i * 2, half(1.0));
    pool.Store<uint16_t>(16 + i * 2, uint16_t(i + 1));
    pool.Store<uint8_t>(32 + i, uint8_t(0x10 + i));
    H h;
    h.v = half(2.0);
    pool.Store<H>(48 + i * 2, h);

    if (i == 0)
    {
        uint a = uint(src.Load<half>(2)) * 100 + uint(src.Load<uint16_t>(6)) * 10;
        pool.Store<uint>(40, a + uint(src.Load<uint8_t>(9)));
        pool.Store<uint>(44, uint(src.Load<half>(0)) + uint(src.Load<uint8_t>(11)));
    }
}

// BUF: 3C003C00
// BUF-NEXT: 3C003C00
// BUF-NEXT: 3C003C00
// BUF-NEXT: 3C003C00
// BUF-NEXT: 20001
// BUF-NEXT: 40003
// BUF-NEXT: 60005
// BUF-NEXT: 80007
// BUF-NEXT: 13121110
// BUF-NEXT: 17161514
// 2.0 * 100 + 7 * 10 + 0x0B = 281
// BUF-NEXT: 119
// 1.0 + 0x0D = 14
// BUF-NEXT: E
// BUF-NEXT: 40004000
// BUF-NEXT: 40004000
// BUF-NEXT: 40004000
// BUF-NEXT: 40004000
