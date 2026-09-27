// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[9321 1],stride=4):name=inputs
StructuredBuffer<uint> inputs;

// All leaves already have the same local-storage and internal-value representation.
// BF2 forces CUDA padding at bytes6..7 and14..15; the record stride is16.
struct Cell
{
    uint16_t before;
    FloatE4M3 a;
    FloatE5M2 b;
    BFloat16 scalar;
    vector<BFloat16, 2> pair;
    uint16_t after;
}
typedef Cell Pair[2];

// Expected values come from scalar bits and the chosen slot/lane, never another record.
uint checkCell(Cell value, uint bits, uint before, uint pairX, uint pairY)
{
    uint bad = uint(uint(value.before) != before);
    bad |= uint(uint(bit_cast<uint8_t>(value.a)) != (bits & 255)) << 1;
    bad |= uint(uint(bit_cast<uint8_t>(value.b)) != ((bits >> 8) ^ 0xa5)) << 2;
    bad |= uint(uint(bit_cast<uint16_t>(value.scalar)) != (bits ^ 0x6c39)) << 3;
    bad |= uint(uint(bit_cast<uint16_t>(value.pair.x)) != pairX) << 4;
    bad |= uint(uint(bit_cast<uint16_t>(value.pair.y)) != pairY) << 5;
    bad |= uint(uint(value.after) != (bits ^ 0xe17b)) << 6;
    return bad;
}

struct Inspection
{
    Cell selected;
    uint errors;
}

// The whole Pair crosses only an internal value-parameter boundary. Mutating this parameter
// changes the callee's own local copy; the caller's saved and live arrays must remain separate.
// The returned record uses the already-qualified nested record result ABI, not an array result.
[noinline]
Inspection inspectAndMutate(Pair values, uint bits, uint selected, uint lane)
{
    uint bad = checkCell(values[0], bits, bits ^ 0x1357, bits, bits ^ 65535);
    uint other = bits ^ 0x5a5a;
    bad |= checkCell(values[1], other, other ^ 0x1357, other, other ^ 65535);
    uint before = bits ^ 0x7777;
    uint replacement = bits ^ 0x5555;
    values[selected].before = uint16_t(before);
    values[selected].pair[lane] = bit_cast<BFloat16>(uint16_t(replacement));

    // Literal slots prevent a consistently wrong dynamic write/read pair from passing.
    bad |= checkCell(values[0], bits, selected == 0 ? before : bits ^ 0x1357,
        selected == 0 && lane == 0 ? replacement : bits,
        selected == 0 && lane == 1 ? replacement : bits ^ 65535);
    bad |= checkCell(values[1], other, selected == 1 ? before : other ^ 0x1357,
        selected == 1 && lane == 0 ? replacement : other,
        selected == 1 && lane == 1 ? replacement : other ^ 65535);
    for (uint k = 0; k < 2; ++k)
    {
        uint j = (k ^ selected) & 1;
        uint t = bits ^ (j * 0x5a5a);
        bad |= checkCell(values[j], t, j == selected ? before : t ^ 0x1357,
            j == selected && lane == 0 ? replacement : t,
            j == selected && lane == 1 ? replacement : t ^ 65535);
    }
    Inspection result;
    result.selected = values[selected];
    result.errors = bad;
    return result;
}

//TEST_INPUT:ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=results
RWStructuredBuffer<uint> results;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint seed = inputs[0];
    uint order = inputs[1];
    uint badCallee = 0;
    uint badSaved = 0;
    uint badLive = 0;
    uint badReturned = 0;
    uint completed = 0;
    for (uint iteration = 0; iteration < 65536; ++iteration)
    {
        uint bits = (iteration ^ seed) & 65535;
        Pair values;
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
        }
        let saved = values;
        uint selected = (bits ^ order) & 1;
        uint lane = (bits >> 1) & 1;
        uint liveBefore = bits ^ 0x4444;
        uint liveReplacement = bits ^ 0x3333;
        values[selected].before = uint16_t(liveBefore);
        values[selected].pair[lane] = bit_cast<BFloat16>(uint16_t(liveReplacement));

        Inspection report = inspectAndMutate(saved, bits, selected, lane);
        badCallee |= report.errors;
        uint tSelected = bits ^ (selected * 0x5a5a);
        badReturned |= checkCell(report.selected, tSelected, bits ^ 0x7777,
            lane == 0 ? bits ^ 0x5555 : tSelected,
            lane == 1 ? bits ^ 0x5555 : tSelected ^ 65535);

        uint other = bits ^ 0x5a5a;
        badSaved |= checkCell(saved[0], bits, bits ^ 0x1357, bits, bits ^ 65535);
        badSaved |= checkCell(saved[1], other, other ^ 0x1357, other, other ^ 65535);
        badLive |= checkCell(values[0], bits, selected == 0 ? liveBefore : bits ^ 0x1357,
            selected == 0 && lane == 0 ? liveReplacement : bits,
            selected == 0 && lane == 1 ? liveReplacement : bits ^ 65535);
        badLive |= checkCell(values[1], other, selected == 1 ? liveBefore : other ^ 0x1357,
            selected == 1 && lane == 0 ? liveReplacement : other,
            selected == 1 && lane == 1 ? liveReplacement : other ^ 65535);
        for (uint k = 0; k < 2; ++k)
        {
            uint j = (k ^ order) & 1;
            uint t = bits ^ (j * 0x5a5a);
            badSaved |= checkCell(saved[j], t, t ^ 0x1357, t, t ^ 65535);
            badLive |= checkCell(values[j], t, j == selected ? liveBefore : t ^ 0x1357,
                j == selected && lane == 0 ? liveReplacement : t,
                j == selected && lane == 1 ? liveReplacement : t ^ 65535);
        }
        ++completed;
    }
    results[0] = badCallee;
    results[1] = badSaved;
    results[2] = badLive;
    results[3] = badReturned;
    results[4] = completed;
    // The final word is an untouched guard.
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 65536{{$}}
// CHECK-NEXT: 3735928559{{$}}
