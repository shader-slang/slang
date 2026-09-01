// Test that decimal float literal parsing produces correctly rounded
// values for float/half literals. Rounding mode is the standard IEEE 754
// round to nearest, ties to even.

//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    ////// regular positive floats -- round to nearest, ties to even //////

    float f;

    printf("0x%08x\n", bit_cast<uint32_t>(8388608.4999999999999999999f));
    //CHECK: 0x4b000000

    printf("0x%08x\n", bit_cast<uint32_t>(8388608.5f)); // even is towards zero
    //CHECK: 0x4b000000

    printf("0x%08x\n", bit_cast<uint32_t>(8388608.5000000000000000001f));
    //CHECK: 0x4b000001

    printf("0x%08x\n", bit_cast<uint32_t>(8388609.4999999999999999999f));
    //CHECK: 0x4b000001

    printf("0x%08x\n", bit_cast<uint32_t>(8388609.5f)); // even is away from zero
    //CHECK: 0x4b000002

    printf("0x%08x\n", bit_cast<uint32_t>(8388609.5000000000000000001f));
    //CHECK: 0x4b000002


    ////// regular positive half-floats -- round to nearest, ties to even //////

    half h;

    printf("0x%04x\n", uint32_t(bit_cast<uint16_t>(61440.hf)));
    //CHECK: 0x7b80

    printf("0x%04x\n", uint32_t(bit_cast<uint16_t>(61456.hf))); // round to even = towards zero
    //CHECK: 0x7b80

    printf("0x%04x\n", uint32_t(bit_cast<uint16_t>(61456.0001hf)));
    //CHECK: 0x7b81

    printf("0x%04x\n", uint32_t(bit_cast<uint16_t>(61472.hf)));
    //CHECK: 0x7b81

    printf("0x%04x\n", uint32_t(bit_cast<uint16_t>(61488.hf))); // round to even = away from zero
    //CHECK: 0x7b82

    printf("0x%04x\n", uint32_t(bit_cast<uint16_t>(61504.hf)));
    //CHECK: 0x7b82
}
