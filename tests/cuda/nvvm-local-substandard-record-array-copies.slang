//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
// BF2 forces padding after a narrow field; nesting adds a second padded record boundary.
// Every expected bit comes from the input and slot, independently of the copied source value.
//TEST_INPUT:ubuffer(data=[9321 1], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;
//TEST_INPUT:ubuffer(data=[3735928559 3735928559 3735928559 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Payload
{
    uint16_t before;
    FloatE4M3 a;
    FloatE5M2 b;
    BFloat16 scalar;
    vector<BFloat16, 2> pair;
    uint16_t after;
}

struct Outer
{
    uint8_t tag;
    Payload payload;
    uint tail;
}

// This call transports a selected record by value; no helper receives an array or element pointer.
[noinline]
uint checkPayload(Payload value, uint bits, uint before, uint pairX, uint pairY)
{
    uint bad = uint(uint(value.before) != before);
    bad |= uint(uint(bit_cast<uint16_t>(value.pair.x)) != pairX) << 1;
    bad |= uint(uint(bit_cast<uint16_t>(value.pair.y)) != pairY) << 2;
    bad |= uint(uint(value.after) != (bits ^ 0xe17b)) << 3;
    bad |= uint(uint(bit_cast<uint8_t>(value.a)) != (bits & 255)) << 4;
    bad |= uint(uint(bit_cast<uint8_t>(value.b)) != ((bits >> 8) ^ 0xa5)) << 5;
    bad |= uint(uint(bit_cast<uint16_t>(value.scalar)) != (bits ^ 0x6c39)) << 6;
    return bad;
}

// Keep the outer tag and tail observable across the nested whole-array copy.
[noinline]
uint checkOuter(Outer value, uint bits, uint pairX, uint pairY, uint tail)
{
    uint bad = checkPayload(value.payload, bits, bits ^ 0x1357, pairX, pairY);
    bad |= uint(uint(value.tag) != ((bits ^ 0x23) & 255)) << 7;
    bad |= uint(value.tail != tail) << 8;
    return bad;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint seed = inputBuffer[0];
    uint order = inputBuffer[1];
    uint badSaved = 0;
    uint badLive = 0;
    uint completed = 0;
    for (uint i = 0; i < 65536; ++i)
    {
        uint bits = (i ^ seed) & 65535;
        Payload values[2];
        Outer nested[2];
        for (uint k = 0; k < 2; ++k)
        {
            uint t = bits ^ (k * 0x5a5a);
            values[k].before = uint16_t(t ^ 0x1357);
            values[k].a = bit_cast<FloatE4M3>(uint8_t(t));
            values[k].b = bit_cast<FloatE5M2>(uint8_t((t >> 8) ^ 0xa5));
            values[k].scalar = bit_cast<BFloat16>(uint16_t(t ^ 0x6c39));
            values[k].pair = vector<BFloat16, 2>(bit_cast<BFloat16>(uint16_t(t)),
                bit_cast<BFloat16>(uint16_t(t ^ 65535)));
            values[k].after = uint16_t(t ^ 0xe17b);
            uint n = t ^ 0xa55a;
            nested[k].tag = uint8_t(n ^ 0x23);
            nested[k].payload.before = uint16_t(n ^ 0x1357);
            nested[k].payload.a = bit_cast<FloatE4M3>(uint8_t(n));
            nested[k].payload.b = bit_cast<FloatE5M2>(uint8_t((n >> 8) ^ 0xa5));
            nested[k].payload.scalar = bit_cast<BFloat16>(uint16_t(n ^ 0x6c39));
            nested[k].payload.pair = vector<BFloat16, 2>(bit_cast<BFloat16>(uint16_t(n)),
                bit_cast<BFloat16>(uint16_t(n ^ 65535)));
            nested[k].payload.after = uint16_t(n ^ 0xe17b);
            nested[k].tail = 0x12340000 | n;
        }
        let saved = values;
        let savedNested = nested;
        uint selected = (bits ^ order) & 1;
        uint lane = (bits >> 1) & 1;
        uint replacement = bits ^ 0x3333;
        uint nestedReplacement = bits ^ 0x7777;
        values[selected].before = uint16_t(bits ^ 0x4444);
        values[selected].pair[lane] = bit_cast<BFloat16>(uint16_t(replacement));
        nested[selected].payload.pair[lane] = bit_cast<BFloat16>(uint16_t(nestedReplacement));
        nested[selected].tail = 0xbeef0000 | bits;

        // Literal slots detect a consistently wrong dynamic write/read pair.
        badSaved |= checkPayload(saved[0], bits, bits ^ 0x1357, bits, bits ^ 65535);
        uint other = bits ^ 0x5a5a;
        badSaved |= checkPayload(saved[1], other, other ^ 0x1357, other, other ^ 65535);
        uint n0 = bits ^ 0xa55a;
        uint n1 = other ^ 0xa55a;
        badSaved |= checkOuter(savedNested[0], n0, n0, n0 ^ 65535, 0x12340000 | n0);
        badSaved |= checkOuter(savedNested[1], n1, n1, n1 ^ 65535, 0x12340000 | n1);
        badLive |= uint(uint(values[0].before) != (selected == 0 ? bits ^ 0x4444 : bits ^ 0x1357));
        badLive |= uint(uint(values[1].before) != (selected == 1 ? bits ^ 0x4444 : other ^ 0x1357));
        badLive |= uint(nested[0].tail != (selected == 0 ? 0xbeef0000 | bits : 0x12340000 | n0));
        badLive |= uint(nested[1].tail != (selected == 1 ? 0xbeef0000 | bits : 0x12340000 | n1));
        for (uint k = 0; k < 2; ++k)
        {
            uint j = (k ^ order) & 1;
            uint t = bits ^ (j * 0x5a5a);
            uint n = t ^ 0xa55a;
            // Dynamic extraction from the saved array value also crosses a by-value record call.
            badSaved |= checkPayload(saved[j], t, t ^ 0x1357, t, t ^ 65535);
            badSaved |= checkOuter(savedNested[j], n, n, n ^ 65535, 0x12340000 | n);
            bool changed = j == selected;
            uint before = changed ? bits ^ 0x4444 : t ^ 0x1357;
            uint x = changed && lane == 0 ? replacement : t;
            uint y = changed && lane == 1 ? replacement : t ^ 65535;
            uint nx = changed && lane == 0 ? nestedReplacement : n;
            uint ny = changed && lane == 1 ? nestedReplacement : n ^ 65535;
            uint tail = changed ? 0xbeef0000 | bits : 0x12340000 | n;
            badLive |= checkPayload(values[j], t, before, x, y);
            badLive |= checkOuter(nested[j], n, nx, ny, tail);
        }
        completed += 1;
    }
    outputBuffer[0] = badSaved;
    outputBuffer[1] = badLive;
    outputBuffer[2] = completed;
    // The final output word is an untouched guard.
}
// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 65536{{$}}
// CHECK-NEXT: 3735928559{{$}}
