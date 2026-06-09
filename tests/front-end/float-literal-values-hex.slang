// Test that hexadecimal float literal parsing produces correct values

//TEST:INTERPRET(filecheck=CHECK):  -warnings-as-errors all
void main()
{
    half h;
    float f;
    double d;

    ////// half-floats //////
    h = 0x0p0hf;
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
    h = 0x1p-14hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0000'0100'0000'0000
    //CHECK-NEXT: 0x0400

    // smallest half-float (subnormal)
    h = 0x1p-24hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0000'0000'0000'0001
    //CHECK-NEXT: 0x0001

    // smallest half-float (subnormal) / exponent
    h = 0x4p-26hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0000'0000'0000'0001
    //CHECK-NEXT: 0x0001

    // maximum half-float
    h = 0x1.ffcp15hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0111'1011'1111'1110
    //CHECK-NEXT: 0x7bff

    // maximum half-float / exponent
    h = 0xffe0p0hf;
    printf("0x%04x\n", (uint32_t)bit_cast<uint16_t>(h));
    // Note: 0111'1011'1111'1110
    //CHECK-NEXT: 0x7bff


    ////// floats //////
    f = 0x0p0f;
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
    f = 0x1p-126f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0000'0000'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x00800000

    // smallest float (normal) / exponent
    f = 0x4p-128f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0000'0000'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x00800000

    // smallest float (subnormal)
    f = 0x1p-149f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0001
    //CHECK-NEXT: 0x00000001

    // smallest float (subnormal) / exponent
    f = 0x4p-151f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0001
    //CHECK-NEXT: 0x00000001

    // maximum float
    f = 0x1.fffffep127f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0111'1111'0111'1111'1111'1111'1111'1111
    //CHECK-NEXT: 0x7f7fffff

    // maximum float / exponent
    f = 0xffffffp104f;
    printf("0x%08x\n", bit_cast<uint32_t>(f));
    // Note: 0111'1111'0111'1111'1111'1111'1111'1111
    //CHECK-NEXT: 0x7f7fffff


    ////// doubles //////
    d = 0x0p0LF;
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
    d = 0x1p-1022LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0000'0000'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x0010000000000000

    // smallest double (normal) / exponent
    d = 0x4p-1024LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0000'0000'1000'0000'0000'0000'0000'0000
    //CHECK-NEXT: 0x0010000000000000

    // smallest double (subnormal)
    d = 0x1p-1074LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0001
    //CHECK-NEXT: 0x0000000000000001

    // smallest double (subnormal) / exponent
    d = 0x4p-1076LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0000'0000'0000'0000'0000'0000'0000'0001
    //CHECK-NEXT: 0x0000000000000001

    // maximum double
    d = 0x1.fffffffffffffp1023LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0111'1111'1110'1111'1111'1111'1111'1111
    //CHECK-NEXT: 0x7fefffffffffffff

    // maximum double / exponent
    d = 0x1fffffffffffffp971LF;
    printf("0x%016llx\n", bit_cast<uint64_t>(d));
    // Note: 0111'1111'1110'1111'1111'1111'1111'1111
    //CHECK-NEXT: 0x7fefffffffffffff
}
