//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
// Both distinct record elements transport every FP8 and BF16 encoding through runtime indices.
// TEST_INPUT:ubuffer(data=[9321 1], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;
// TEST_INPUT:ubuffer(data=[3735928559 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Cell
{
    uint16_t before;
    FloatE4M3 a;
    FloatE5M2 b;
    BFloat16 scalar;
    uint16_t after;
};

[numthreads(1, 1, 1)]
void computeMain()
{
    uint seed = inputBuffer[0];
    uint order = inputBuffer[1];
    uint badDestination = 0;
    uint completed = 0;
    for (uint i = 0; i < 65536; ++i)
    {
        uint bits = (i ^ seed) & 65535;
        Cell values[2];
        for (uint k = 0; k < 2; ++k)
        {
            uint t = (bits ^ (k * 0x5a5a)) & 65535;
            values[k].before = uint16_t(t ^ 0x1357);
            values[k].a = bit_cast<FloatE4M3>(uint8_t(t & 255));
            values[k].b = bit_cast<FloatE5M2>(uint8_t((t >> 8) ^ 0xa5));
            values[k].scalar = bit_cast<BFloat16>(uint16_t(t ^ 0x6c39));
            values[k].after = uint16_t(t ^ 0xe17b);
        }
        for (uint k = 0; k < 2; ++k)
        {
            uint j = (k ^ order) & 1;
            uint t = (bits ^ (j * 0x5a5a)) & 65535;
            if (uint(values[j].before) != (t ^ 0x1357)) badDestination |= 1;
            if (uint(bit_cast<uint8_t>(values[j].a)) != (t & 255)) badDestination |= 2;
            if (uint(bit_cast<uint8_t>(values[j].b)) != ((t >> 8) ^ 0xa5)) badDestination |= 4;
            if (uint(bit_cast<uint16_t>(values[j].scalar)) != (t ^ 0x6c39)) badDestination |= 8;
            if (uint(values[j].after) != (t ^ 0xe17b)) badDestination |= 16;
        }
        completed += 1;
    }
    outputBuffer[0] = badDestination;
    outputBuffer[1] = completed;
}
// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 65536{{$}}
