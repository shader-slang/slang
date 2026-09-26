//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Every encoding crosses each branch and both whole-record memory directions. Integer guards
// check fields around the FP8/BF16 payloads. No floating conversion or arithmetic is involved.
//TEST_INPUT:ubuffer(data=[0], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;
//TEST_INPUT:ubuffer(data=[1 1 1 1], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Payload
{
    uint8_t prefix;
    FloatE4M3 a;
    uint16_t middle;
    FloatE5M2 b;
    BFloat16 scalar;
    uint guard;
    vector<BFloat16, 2> pair;
    uint64_t tail;
}

[noinline]
Payload choose(Payload a, Payload b, bool first)
{
    Payload result = b;
    if (first)
        result = a;
    return result;
}

[noinline]
void initialize(out Payload p, uint bits)
{
    p.prefix = uint8_t(17);
    p.a = bit_cast<FloatE4M3>(uint8_t(bits));
    p.middle = uint16_t(1901);
    p.b = bit_cast<FloatE5M2>(uint8_t(bits ^ 255));
    p.scalar = bit_cast<BFloat16>(uint16_t(bits));
    p.guard = 0x12345678;
    p.pair = vector<BFloat16, 2>(bit_cast<BFloat16>(uint16_t(bits)),
        bit_cast<BFloat16>(uint16_t(bits ^ 65535)));
    p.tail = uint64_t(0x98765432) << 32 | uint64_t(0xabcdef01);
}

[noinline]
Payload replace(inout Payload p, Payload q)
{
    let previous = p;
    p = q;
    return previous;
}

uint check(Payload p, uint bits)
{
    uint bad = uint(p.prefix != 17 || p.middle != 1901 || p.guard != 0x12345678);
    bad |= uint(p.tail != (uint64_t(0x98765432) << 32 | uint64_t(0xabcdef01)));
    bad |= uint(uint(bit_cast<uint8_t>(p.a)) != (bits & 255));
    bad |= uint(uint(bit_cast<uint8_t>(p.b)) != ((bits ^ 255) & 255));
    bad |= uint(uint(bit_cast<uint16_t>(p.scalar)) != (bits & 65535));
    bad |= uint(uint(bit_cast<uint16_t>(p.pair.x)) != (bits & 65535));
    bad |= uint(uint(bit_cast<uint16_t>(p.pair.y)) != ((bits ^ 65535) & 65535));
    return bad;
}

// BF2 must start at byte four, making the local record twelve bytes/alignment four.
// Natural AnyValue marshalling instead packs its halves at byte two; those are separate roles.
struct AlignedPair
{
    uint16_t prefix;
    vector<BFloat16, 2> value;
    uint16_t suffix;
}

[noinline]
void initializePair(out AlignedPair p, uint bits)
{
    p.prefix = uint16_t(1701);
    p.value.x = bit_cast<BFloat16>(uint16_t(bits));
    p.value.y = bit_cast<BFloat16>(uint16_t(bits ^ 65535));
    p.suffix = uint16_t(1901);
}

[noinline]
AlignedPair replacePair(inout AlignedPair p, uint bits, uint lane)
{
    let old = p;
    p.value[lane] = bit_cast<BFloat16>(uint16_t(bits));
    return old;
}

uint checkPair(AlignedPair p, uint bits)
{
    return uint(p.prefix != 1701 || p.suffix != 1901 ||
        uint(bit_cast<uint16_t>(p.value.x)) != bits ||
        uint(bit_cast<uint16_t>(p.value.y)) != (bits ^ 65535));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint seed = inputBuffer[0];
    uint bad = 0;
    for (uint i = 0; i < 65536; ++i)
    {
        uint bits = (i ^ seed) & 65535;
        Payload a;
        Payload b;
        initialize(a, bits);
        initialize(b, bits ^ 65535);
        for (uint flag = 0; flag < 2; ++flag)
        {
            Payload selected = choose(a, b, flag == 0);
            bad |= check(selected, flag == 0 ? bits : bits ^ 65535);
        }
        for (uint lane = 0; lane < 2; ++lane)
        {
            AlignedPair pair;
            initializePair(pair, bits);
            uint replacement = bits ^ 0x5555;
            let oldPair = replacePair(pair, replacement, lane);
            bad |= checkPair(oldPair, bits);
            bad |= uint(uint(bit_cast<uint16_t>(pair.value.x)) !=
                (lane == 0 ? replacement : bits));
            bad |= uint(uint(bit_cast<uint16_t>(pair.value.y)) !=
                (lane == 1 ? replacement : bits ^ 65535));
            bad |= uint(pair.prefix != 1701 || pair.suffix != 1901);
        }
        let previous = replace(a, b);
        bad |= check(previous, bits);
        bad |= check(a, bits ^ 65535);
    }
    outputBuffer[0] = bad;
    outputBuffer[1] = 123;
    outputBuffer[2] = seed;
    outputBuffer[3] = 456;
}
//CHECK: type: uint
//CHECK-NEXT: 0
//CHECK-NEXT: 123
//CHECK-NEXT: 0
//CHECK-NEXT: 456
