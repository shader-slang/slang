//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
// Every scalar encoding crosses three nested records, both branches and both uint16 vector lanes.
// Integer guards and whole-inner assignments check layout and transport without float arithmetic.
//TEST_INPUT:ubuffer(data=[0], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;
//TEST_INPUT:ubuffer(data=[1 1 1 1], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Inner
{
    uint8_t tag;
    uint8_t a;
    uint8_t b;
    uint16_t scalar;
    vector<uint16_t, 2> pair;
    uint16_t suffix;
}

// The integer-only control exercises the same physical layout without substandard float types.
struct Guards
{
    uint16_t first;
    uint last;
}

struct Middle
{
    uint8_t tag;
    Inner payload;
    Guards guards;
}

struct Outer
{
    uint16_t prefix;
    Middle middle;
    uint64_t tail;
}

[noinline]
void initialize(out Outer p, uint bits)
{
    p.prefix = uint16_t(1701);
    p.middle.tag = uint8_t(23);
    p.middle.guards.first = uint16_t(bits ^ 0x369c);
    p.middle.guards.last = 0x12340000 | bits;
    p.middle.payload.tag = uint8_t(17);
    p.middle.payload.a = bit_cast<uint8_t>(uint8_t(bits));
    p.middle.payload.b = bit_cast<uint8_t>(uint8_t(bits ^ 255));
    p.middle.payload.scalar = bit_cast<uint16_t>(uint16_t(bits));
    p.middle.payload.pair = vector<uint16_t, 2>(bit_cast<uint16_t>(uint16_t(bits)),
        bit_cast<uint16_t>(uint16_t(bits ^ 65535)));
    p.middle.payload.suffix = uint16_t(1901);
    p.tail = uint64_t(0x98765432) << 32 | uint64_t(0xabcdef01);
}

[noinline]
Outer choose(Outer a, Outer b, bool first)
{
    Outer result = b;
    if (first)
        result = a;
    return result;
}

[noinline]
Outer replace(inout Outer p, Outer q)
{
    let previous = p;
    p = q;
    return previous;
}

// Read and replace the complete inner record through a derived local field address.
[noinline]
Inner replaceInner(inout Outer p, Outer q)
{
    let previous = p.middle.payload;
    p.middle.payload = q.middle.payload;
    p.middle.guards = q.middle.guards;
    return previous;
}

[noinline]
Outer mutatePair(inout Outer p, uint bits, uint lane)
{
    let previous = p;
    p.middle.payload.pair[lane] = bit_cast<uint16_t>(uint16_t(bits));
    return previous;
}

uint check(Outer p, uint bits, uint pairX, uint pairY)
{
    uint bad = uint(p.prefix != 1701 || p.middle.payload.tag != 17 || p.middle.payload.suffix != 1901);
    bad |= uint(p.middle.tag != 23 || uint(p.middle.guards.first) != (bits ^ 0x369c) ||
        p.middle.guards.last != (0x12340000 | bits));
    bad |= uint(p.tail != (uint64_t(0x98765432) << 32 | uint64_t(0xabcdef01)));
    bad |= uint(uint(bit_cast<uint8_t>(p.middle.payload.a)) != (bits & 255));
    bad |= uint(uint(bit_cast<uint8_t>(p.middle.payload.b)) != ((bits ^ 255) & 255));
    bad |= uint(uint(bit_cast<uint16_t>(p.middle.payload.scalar)) != bits);
    bad |= uint(uint(bit_cast<uint16_t>(p.middle.payload.pair.x)) != pairX);
    bad |= uint(uint(bit_cast<uint16_t>(p.middle.payload.pair.y)) != pairY);
    return bad;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint seed = inputBuffer[0];
    uint bad = 0;
    for (uint i = 0; i < 65536; ++i)
    {
        uint bits = (i ^ seed) & 65535;
        Outer a;
        Outer b;
        initialize(a, bits);
        initialize(b, bits ^ 65535);
        for (uint flag = 0; flag < 2; ++flag)
        {
            uint selectedBits = flag == 0 ? bits : bits ^ 65535;
            bad |= check(choose(a, b, flag == 0), selectedBits, selectedBits, selectedBits ^ 65535);
        }
        let previousGuards = a.middle.guards;
        let previousInner = replaceInner(a, b);
        bad |= check(a, bits ^ 65535, bits ^ 65535, bits);
        // Reconstructing the outer value also makes the returned inner value observable.
        a.middle.payload = previousInner;
        a.middle.guards = previousGuards;
        bad |= check(a, bits, bits, bits ^ 65535);
        let previous = replace(a, b);
        bad |= check(previous, bits, bits, bits ^ 65535);
        bad |= check(a, bits ^ 65535, bits ^ 65535, bits);
        for (uint lane = 0; lane < 2; ++lane)
        {
            initialize(a, bits);
            uint replacement = bits ^ 0x5555;
            let old = mutatePair(a, replacement, lane);
            bad |= check(old, bits, bits, bits ^ 65535);
            bad |= check(a, bits, lane == 0 ? replacement : bits,
                lane == 1 ? replacement : bits ^ 65535);
        }
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
