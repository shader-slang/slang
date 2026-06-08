// Test that decimal float literal parsing produces correct values

//TEST:INTERPRET(filecheck=CHECK):  -warnings-as-errors all
void main()
{
    half h;
    float f;
    double d;

    ////// half-floats //////
    h = 0.0hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0000'0000'0000'0000
    //CHECK: 0x0000

    h = 1#INFhf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0111'1100'0000'0000
    //CHECK-NEXT: 0x7c00

    h = -1#INFhf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 1111'1100'0000'0000
    //CHECK-NEXT: 0xfc00

    // smallest half-float (normal)
    h = .000061035hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0000'0100'0000'0000
    //CHECK-NEXT: 0x0400

    // smallest half-float (subnormal)
    h = .000000060hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0000'0000'0000'0001
    //CHECK-NEXT: 0x0001

    // smallest half-float (subnormal) / exponent
    h = 6.0e-8hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0000'0000'0000'0001
    //CHECK-NEXT: 0x0001

    // maximum half-float
    h = 65504.hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0111'1011'1111'1110
    //CHECK-NEXT: 0x7bff

    // maximum half-float / exponent
    h = 6.5504e4hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0111'1011'1111'1110
    //CHECK-NEXT: 0x7bff


    ////// floats //////
    f = 0.0f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x00000000

    f = 1#INFf;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0111'1111'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x7f800000

    f = -1#INFf;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 1111'1111'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0xff800000

    // smallest float (normal)
    f = .0000000000000000000000000000000000000117549435f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0000'0000'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x00800000

    // smallest float (normal) / exponent
    f = 1.17549435e-38f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0000'0000'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x00800000

    // smallest float (subnormal)
    f = .0000000000000000000000000000000000000000000014f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0001
    //CHECK-NEXT: 0x00000001

    // smallest float (subnormal) / exponent
    f = 1.4e-45f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0001
    //CHECK-NEXT: 0x00000001

    // maximum float
    f = 340282340000000000000000000000000000000.f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0111'1111'0111'1111'1111'1111'1111'1111
    //CHECK-NEXT: 0x7f7fffff

    // maximum float / exponent
    f = 3.4028234e+38f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0111'1111'0111'1111'1111'1111'1111'1111
    //CHECK-NEXT: 0x7f7fffff


    ////// doubles //////
    d = 0.0LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x0000000000000000

    d = 1#INFLF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0111'1111'1111'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x7ff0000000000000

    d = -1#INFLF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 1111'1111'1111'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0xfff0000000000000

    // smallest double (normal)
    d = .000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000022250738585072014LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0000'0000'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x0010000000000000

    // smallest double (normal) / exponent
    d = 2.2250738585072014e-308LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0000'0000'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x0010000000000000

    // smallest double (subnormal)
    d = .0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000049LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0001
    //CHECK-NEXT: 0x0000000000000001

    // smallest double (subnormal) / exponent
    d = 4.9e-324LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0001
    //CHECK-NEXT: 0x0000000000000001

    // maximum double
    d = 179769313486231570000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0111'1111'1110'1111'1111'1111'1111'1111
    //CHECK-NEXT: 0x7fefffff

    // maximum double / exponent
    d = 1.7976931348623157E+308LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0111'1111'1110'1111'1111'1111'1111'1111
    //CHECK-NEXT: 0x7fefffff
}
