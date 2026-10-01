//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:ubuffer(data=[291 65244 32769 32705],stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[324508639 99 99 99 99 99 99 99 99 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;

struct Inner
{
    uint16_t before;
    vector<BFloat16, 2> pair;
    FloatE4M3 tag;
    BFloat16 scalar;
    uint16_t after;
}
struct Outer { uint8_t tag; Inner inner; uint tail; }
typedef Outer Pair[2];

[noinline]
uint readLane(__constref Pair values, uint slot, uint lane)
{
    return uint(bit_cast<uint16_t>(values[slot].inner.pair[lane]));
}

[noinline]
uint readForward(__constref Pair values, uint slot, uint lane)
{
    return readLane(values, slot, lane);
}

[noinline]
void mutate(inout Pair values, uint slot, uint lane, uint bits)
{
    values[slot].inner.pair[lane] = bit_cast<BFloat16>(uint16_t(bits));
}

[noinline]
uint forward(inout Pair values, uint slot, uint lane, uint bits)
{
    uint before = readForward(values, slot, lane);
    mutate(values, slot, lane, bits);
    return before;
}

[noinline]
void initialize(out Pair values, uint seed)
{
    for (uint k = 0; k < 2; ++k)
    {
        uint bits = seed ^ (k * 0x5a5a);
        values[k].tag = uint8_t(bits ^ 0x93);
        values[k].inner.before = uint16_t(bits ^ 0x1357);
        values[k].inner.pair = vector<BFloat16, 2>(
            bit_cast<BFloat16>(uint16_t(bits)), bit_cast<BFloat16>(uint16_t(bits ^ 65535)));
        values[k].inner.tag = bit_cast<FloatE4M3>(uint8_t(bits));
        values[k].inner.scalar = bit_cast<BFloat16>(uint16_t(bits ^ 0x6c39));
        values[k].inner.after = uint16_t(bits ^ 0xe17b);
        values[k].tail = bits ^ 0x12345678;
    }
    // Storage is fully initialized before an out parameter is forwarded as mutable input.
    forward(values, 0, 0, seed ^ 0x2222);
}

[noinline]
Pair make(uint seed)
{
    Pair values;
    initialize(values, seed);
    return values;
}

uint check(Outer value, uint bits, uint x, uint y)
{
    uint bad = uint(uint(value.tag) != ((bits ^ 0x93) & 255));
    bad |= uint(uint(value.inner.before) != (bits ^ 0x1357)) << 1;
    bad |= uint(uint(bit_cast<uint16_t>(value.inner.pair.x)) != x) << 2;
    bad |= uint(uint(bit_cast<uint16_t>(value.inner.pair.y)) != y) << 3;
    bad |= uint(uint(bit_cast<uint8_t>(value.inner.tag)) != (bits & 255)) << 4;
    bad |= uint(uint(bit_cast<uint16_t>(value.inner.scalar)) != (bits ^ 0x6c39)) << 5;
    bad |= uint(uint(value.inner.after) != (bits ^ 0xe17b)) << 6;
    bad |= uint(value.tail != (bits ^ 0x12345678)) << 7;
    return bad;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint seed = inputs[tid.x];
    uint other = seed ^ 0x5a5a;
    uint slot = tid.x & 1;
    uint lane = (tid.x >> 1) & 1;
    uint replacement = seed ^ 0x5555;
    Pair direct;
    initialize(direct, seed);
    Pair values = make(seed);
    let saved = values;
    uint expectedBefore = slot == 0 && lane == 0 ? seed ^ 0x2222
        : (slot == 0 ? seed : other) ^ (lane == 0 ? 0 : 65535);
    uint before = readForward(values, slot, lane);
    uint forwardedBefore = forward(values, slot, lane, replacement);
    uint after = readForward(values, slot, lane);
    uint savedAfter = readForward(saved, slot, lane);
    uint bad = uint(before != expectedBefore || forwardedBefore != expectedBefore
        || after != replacement || savedAfter != expectedBefore) << 8;
    bad |= check(direct[0], seed, seed ^ 0x2222, seed ^ 65535);
    bad |= check(direct[1], other, other, other ^ 65535);
    bad |= check(saved[0], seed, seed ^ 0x2222, seed ^ 65535);
    bad |= check(saved[1], other, other, other ^ 65535);
    // Literal reads make a consistently wrong dynamic write/read pair observable.
    bad |= check(values[0], seed,
        slot == 0 && lane == 0 ? replacement : seed ^ 0x2222,
        slot == 0 && lane == 1 ? replacement : seed ^ 65535);
    bad |= check(values[1], other,
        slot == 1 && lane == 0 ? replacement : other,
        slot == 1 && lane == 1 ? replacement : other ^ 65535);
    output[1 + 2 * tid.x] = bad;
    output[2 + 2 * tid.x] = 4096 + tid.x;
}

//CHECK: type: uint32_t
//CHECK-NEXT: {{^}}324508639{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}4096{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}4097{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}4098{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}4099{{$}}
//CHECK-NEXT: {{^}}3735928559{{$}}
