// NVRTC's known nested-array copy failure remains a differential control in slice285.
// clang-format off
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
// clang-format on
// TEST_INPUT:ubuffer(data=[0], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;
// TEST_INPUT:ubuffer(data=[1 1 1 1], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// A Cell occupies 12 bytes: its fields start at offsets 0, 4, and 8.
// Whole array stores must preserve the padding before each child record.
struct Child
{
    uint16_t first;
    uint last;
};
struct Cell
{
    uint16_t first;
    Child child;
};
typedef Cell Payload[3];

[noinline] void initialize(out Payload p, uint bits)
{
    for (uint k = 0; k < 3; ++k)
    {
        uint value = bits ^ (k * 0x1111);
        p[k].first = uint16_t(value);
        p[k].child.first = uint16_t(value ^ 0x5555);
        p[k].child.last = 0x12340000 | value;
    }
}

    [noinline] Payload choose(Payload a, Payload b, bool first)
{
    Payload result = b;
    if (first)
        result = a;
    return result;
}

[noinline] void assignOut(out Payload destination, Payload source) { destination = source; }

    [numthreads(1, 1, 1)] void computeMain()
{
    uint seed = inputBuffer[0];
    uint bad = 0;
    for (uint i = 0; i < 65536; ++i)
    {
        uint bits = (i ^ seed) & 65535;
        Payload source;
        Payload other;
        Payload destination;
        initialize(source, bits);
        initialize(other, bits ^ 65535);

        // Check a fresh-source out copy against integers, before any return or saved copy.
        initialize(destination, bits ^ 65535);
        assignOut(destination, source);
        for (uint k = 0; k < 3; ++k)
        {
            uint expected = (bits) ^ (k * 0x1111);
            bad |= uint(
                       uint(destination[k].first) != expected ||
                       uint(destination[k].child.first) != (expected ^ 0x5555) ||
                       destination[k].child.last != (0x12340000 | expected)) *
                   1;
        }
        for (uint k = 0; k < 3; ++k)
        {
            uint expected = (bits) ^ (k * 0x1111);
            bad |= uint(
                       uint(source[k].first) != expected ||
                       uint(source[k].child.first) != (expected ^ 0x5555) ||
                       source[k].child.last != (0x12340000 | expected)) *
                   2;
        }

        // The saved value must survive mutation of every field in its source storage.
        let saved = source;
        initialize(source, bits ^ 65535);
        initialize(destination, bits ^ 65535);
        assignOut(destination, saved);
        for (uint k = 0; k < 3; ++k)
        {
            uint expected = (bits) ^ (k * 0x1111);
            bad |= uint(
                       uint(destination[k].first) != expected ||
                       uint(destination[k].child.first) != (expected ^ 0x5555) ||
                       destination[k].child.last != (0x12340000 | expected)) *
                   4;
        }
        for (uint k = 0; k < 3; ++k)
        {
            uint expected = (bits) ^ (k * 0x1111);
            bad |= uint(
                       uint(saved[k].first) != expected ||
                       uint(saved[k].child.first) != (expected ^ 0x5555) ||
                       saved[k].child.last != (0x12340000 | expected)) *
                   8;
        }
        for (uint k = 0; k < 3; ++k)
        {
            uint expected = (bits ^ 65535) ^ (k * 0x1111);
            bad |= uint(
                       uint(source[k].first) != expected ||
                       uint(source[k].child.first) != (expected ^ 0x5555) ||
                       source[k].child.last != (0x12340000 | expected)) *
                   16;
        }

        // Both choices return a whole value, with independently known field expectations.
        for (uint flag = 0; flag < 2; ++flag)
        {
            let selected = choose(saved, other, flag == 0);
            uint selectedBits = flag == 0 ? bits : bits ^ 65535;
            for (uint k = 0; k < 3; ++k)
            {
                uint expected = (selectedBits) ^ (k * 0x1111);
                bad |= uint(
                           uint(selected[k].first) != expected ||
                           uint(selected[k].child.first) != (expected ^ 0x5555) ||
                           selected[k].child.last != (0x12340000 | expected)) *
                       32;
            }
        }
        for (uint k = 0; k < 3; ++k)
        {
            uint expected = (bits ^ 65535) ^ (k * 0x1111);
            bad |= uint(
                       uint(other[k].first) != expected ||
                       uint(other[k].child.first) != (expected ^ 0x5555) ||
                       other[k].child.last != (0x12340000 | expected)) *
                   64;
        }
    }
    outputBuffer[0] = bad;
    outputBuffer[1] = 123;
    outputBuffer[2] = seed;
    outputBuffer[3] = 456;
}
// CHECK: type: uint
// CHECK-NEXT: 0
// CHECK-NEXT: 123
// CHECK-NEXT: 0
// CHECK-NEXT: 456
