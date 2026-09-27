// clang-format off
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
// clang-format on
// Independent raw-bit checks cover both runtime-loaded conformers and every 16-bit encoding.
// Fields vary through XOR-correlated patterns, not the Cartesian product of their encodings.
// Each conformer checks initial unpacking, mutation and pack-back, and an earlier interface copy.
// TEST_INPUT:ubuffer(data=[0 1 0], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;
// TEST_INPUT:ubuffer(data=[1 1 1 1 1 1 1 1], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[anyValueSize(20)]
interface IRecord
{
    uint inspect(uint bits, uint expectedKind);
    [mutating] void flip();
};

struct PairPayload
{
    uint16_t before;
    vector<uint16_t, 2> pair;
    uint16_t after;
};

struct Leaf
{
    uint8_t a;
    uint8_t b;
    uint16_t scalar;
};

// These integer words specify Natural payload bytes, independently of CUDA local layout.
struct PackedWords
{
    uint w0;
    uint w1;
    uint w2;
    uint w3;
    uint w4;
};

struct RecordA : IRecord
{
    uint head;
    PairPayload payload;
    Leaf leaf;
    uint tail;

    [noinline]
    uint inspect(uint bits, uint expectedKind)
    {
        uint bad = uint(head != (0xa5c30000 | bits));
        bad |= uint(tail != (0x5a3c0000 | (bits ^ 0xffff))) << 1;
        bad |= uint(uint(payload.before) != (bits ^ 0x2468)) << 2;
        bad |= uint(uint(payload.after) != (bits ^ 0xdb97)) << 3;
        bad |= uint(uint(bit_cast<uint8_t>(leaf.a)) != (bits & 255)) << 4;
        bad |= uint(uint(bit_cast<uint8_t>(leaf.b)) != ((bits ^ 255) & 255)) << 5;
        bad |= uint(uint(bit_cast<uint16_t>(leaf.scalar)) != (bits ^ 0x369c)) << 6;
        bad |= uint(uint(bit_cast<uint16_t>(payload.pair.x)) != (bits ^ 0x1357)) << 7;
        bad |= uint(uint(bit_cast<uint16_t>(payload.pair.y)) != (bits ^ 0xeca8)) << 8;
        bad |= uint(expectedKind != 0) << 9;
        return bad;
    }

    [mutating]
    [noinline]
    void flip()
    {
        head ^= 0x5aa5;
        tail ^= 0x5aa5;
        payload.before = uint16_t(uint(payload.before) ^ 0x5aa5);
        payload.after = uint16_t(uint(payload.after) ^ 0x5aa5);
        leaf.a = bit_cast<uint8_t>(uint8_t(uint(bit_cast<uint8_t>(leaf.a)) ^ 0xa5));
        leaf.b = bit_cast<uint8_t>(uint8_t(uint(bit_cast<uint8_t>(leaf.b)) ^ 0xa5));
        leaf.scalar = bit_cast<uint16_t>(uint16_t(uint(bit_cast<uint16_t>(leaf.scalar)) ^ 0x5aa5));
        payload.pair.x =
            bit_cast<uint16_t>(uint16_t(uint(bit_cast<uint16_t>(payload.pair.x)) ^ 0x5aa5));
        payload.pair.y =
            bit_cast<uint16_t>(uint16_t(uint(bit_cast<uint16_t>(payload.pair.y)) ^ 0x5aa5));
    }
};

struct RecordB : IRecord
{
    uint head;
    Leaf leaf;
    PairPayload payload;
    uint tail;

    [noinline]
    uint inspect(uint bits, uint expectedKind)
    {
        uint bad = uint(head != (0xa5c30000 | bits));
        bad |= uint(tail != (0x5a3c0000 | (bits ^ 0xffff))) << 1;
        bad |= uint(uint(payload.before) != (bits ^ 0x2468)) << 2;
        bad |= uint(uint(payload.after) != (bits ^ 0xdb97)) << 3;
        bad |= uint(uint(bit_cast<uint8_t>(leaf.a)) != (bits & 255)) << 4;
        bad |= uint(uint(bit_cast<uint8_t>(leaf.b)) != ((bits ^ 255) & 255)) << 5;
        bad |= uint(uint(bit_cast<uint16_t>(leaf.scalar)) != (bits ^ 0x369c)) << 6;
        bad |= uint(uint(bit_cast<uint16_t>(payload.pair.x)) != (bits ^ 0x1357)) << 7;
        bad |= uint(uint(bit_cast<uint16_t>(payload.pair.y)) != (bits ^ 0xeca8)) << 8;
        bad |= uint(expectedKind != 1) << 9;
        return bad;
    }

    [mutating]
    [noinline]
    void flip()
    {
        head ^= 0x5aa5;
        tail ^= 0x5aa5;
        payload.before = uint16_t(uint(payload.before) ^ 0x5aa5);
        payload.after = uint16_t(uint(payload.after) ^ 0x5aa5);
        leaf.a = bit_cast<uint8_t>(uint8_t(uint(bit_cast<uint8_t>(leaf.a)) ^ 0xa5));
        leaf.b = bit_cast<uint8_t>(uint8_t(uint(bit_cast<uint8_t>(leaf.b)) ^ 0xa5));
        leaf.scalar = bit_cast<uint16_t>(uint16_t(uint(bit_cast<uint16_t>(leaf.scalar)) ^ 0x5aa5));
        payload.pair.x =
            bit_cast<uint16_t>(uint16_t(uint(bit_cast<uint16_t>(payload.pair.x)) ^ 0x5aa5));
        payload.pair.y =
            bit_cast<uint16_t>(uint16_t(uint(bit_cast<uint16_t>(payload.pair.y)) ^ 0x5aa5));
    }
};

// TEST_INPUT: type_conformance RecordA:IRecord=0
// TEST_INPUT: type_conformance RecordB:IRecord=1

[numthreads(2, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint slot = dispatchThreadID.x;
    uint kind = inputBuffer[slot];
    uint seed = inputBuffer[2];
    uint beforeBad = 0;
    uint afterBad = 0;
    uint snapshotBad = 0;
    uint completed = 0;
    for (uint i = 0; i < 65536; ++i)
    {
        uint bits = (i ^ seed) & 65535;
        uint pairLo = (bits ^ 0x2468) | ((bits ^ 0x1357) << 16);
        uint pairHi = (bits ^ 0xeca8) | ((bits ^ 0xdb97) << 16);
        uint leafWord = (bits & 255) | (((bits ^ 255) & 255) << 8) | ((bits ^ 0x369c) << 16);
        PackedWords packed;
        packed.w0 = 0xa5c30000 | bits;
        packed.w1 = kind == 0 ? pairLo : leafWord;
        packed.w2 = kind == 0 ? pairHi : pairLo;
        packed.w3 = kind == 0 ? leafWord : pairHi;
        packed.w4 = 0x5a3c0000 | (bits ^ 65535);
        IRecord object = createDynamicObject<IRecord>(kind, packed);
        beforeBad |= object.inspect(bits, kind);
        let snapshot = object;
        object.flip();
        afterBad |= object.inspect(bits ^ 0x5aa5, kind);
        snapshotBad |= snapshot.inspect(bits, kind);
        ++completed;
    }
    outputBuffer[slot * 4] = beforeBad;
    outputBuffer[slot * 4 + 1] = afterBad;
    outputBuffer[slot * 4 + 2] = snapshotBad;
    outputBuffer[slot * 4 + 3] = completed;
}
// CHECK: type: uint
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 65536
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 65536
