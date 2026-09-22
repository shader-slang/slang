//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int8_t funcInt8(int8_t x) { return x; }
int16_t funcInt16(int16_t x) { return x; }
int funcInt32(int x) { return x; }
int64_t funcInt64(int64_t x) { return x; }
uint8_t funcUint8(uint8_t x) { return x; }
uint16_t funcUint16(uint16_t x) { return x; }
uint funcUint32(uint x) { return x; }
uint64_t funcUint64(uint64_t x) { return x; }
half funcHalf(half x) { return x; }
bool funcBool(bool x) { return x; }

StructuredBuffer<uint64_t> inputs;
RWStructuredBuffer<uint64_t> outputs;

[numthreads(1, 1, 1)]
void computeMain()
{
    let c_s8  : int8_t   = (int8_t)  inputs[0];
    let c_s16 : int16_t  = (int16_t) inputs[1];
    let c_s32 : int      = (int)     inputs[2];
    let c_s64 : int64_t  = (int64_t) inputs[3];
    let c_u8  : uint8_t  = (uint8_t) inputs[0];
    let c_u16 : uint16_t = (uint16_t)inputs[1];
    let c_u32 : uint     = (uint)    inputs[2];
    let c_u64 : uint64_t = (uint64_t)inputs[3];

    // these should not trigger a warning (bitwise value; unsigned->signed conversion ok)
    funcInt8 (0x80 | 0x08);
    funcInt16(0x8000 | 0x0008);
    funcInt32(0x80000000 | 0x00000008);
    funcInt64(0x8000000000000000 | 0x0000000000000008);

    funcInt8(~0);
    funcInt16(~0);
    funcInt32(~0);
    funcInt64(~0);
    funcInt64(~0x8000000000000000U);
    funcUint8(~0);
    funcUint16(~0);
    funcUint32(~0);
    funcUint64(~0);
    funcUint64(~0x8000000000000000U);

    funcInt8(0xffffffff & 0xff);
    funcInt16(0xffffffff & 0xffff);
    funcInt32(0xffffffffffffffff & 0xffffffff);
    funcUint8(0xffffffff & 0xff);
    funcUint16(0xffffffff & 0xffff);
    funcUint32(0xffffffffffffffff & 0xffffffff);

    funcInt8  (0xffffffff ^ 0xffffff00);
    funcInt16 (0xffffffff ^ 0xffff0000);
    funcInt32 (0xffffffffffffffff ^ 0xffffffff00000000);
    funcUint8 (0xffffffff ^ 0xffffff00);
    funcUint16(0xffffffff ^ 0xffff0000);
    funcUint32(0xffffffffffffffff ^ 0xffffffff00000000);

    funcInt8(1 << 7);
    funcUint8(1 << 7);
    funcInt16(1 << 15);
    funcUint16(1 << 15);
    funcInt32(1 << 31);
    funcUint32(1 << 31);
    funcInt64(1 << 63);
    funcUint64(1 << 63);

    // then the variants with warnings
    funcInt8 (0x80 | 0x108);
    funcInt16(0x8000 | 0x10008);
    funcInt32(0x80000000 | 0x100000008);

    funcInt8(~0x00000080);
    funcInt16(~0x00008000);
    funcInt32(~0x80000000);
    funcUint8(~0xFFFFFF00);
    funcUint16(~0xFFFF0000);
    funcUint32(~0x00000000);

    funcInt8(0xffffffff & 0x1ff);
    funcInt16(0xffffffff & 0x1ffff);
    funcInt32(0xffffffffffffffff & 0x1ffffffff);
    funcUint8(0xffffffff & 0x1ff);
    funcUint16(0xffffffff & 0x1ffff);
    funcUint32(0xffffffffffffffff & 0x1ffffffff);

    funcInt8  (0xefffffff ^ 0xffffff00);
    funcInt16 (0xefffffff ^ 0xffff0000);
    funcInt32 (0xefffffffffffffff ^ 0xffffffff00000000);
    funcUint8 (0xefffffff ^ 0xffffff00);
    funcUint16(0xefffffff ^ 0xffff0000);
    funcUint32(0xefffffffffffffff ^ 0xffffffff00000000);

    funcInt8(1 << 8);
    funcUint8(1 << 8);
    funcInt16(1 << 16);
    funcUint16(1 << 16);
    funcInt32(1 << 32);
    funcUint32(1 << 32);
    funcInt64(1 << 64);
}
