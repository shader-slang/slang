//TEST:INTERPRET(filecheck=CHECK):

// Verify BFloat16 conversions produce correct results
// using the bytecode interpreter (no GPU required).

void main()
{
    // float -> BFloat16 -> float roundtrip
    float f = 3.0;
    BFloat16 bf = BFloat16(f);
    // CHECK: f2bf=3
    printf("f2bf=%d\n", int(float(bf)));

    // half -> BFloat16 -> float
    half h = half(2.0);
    BFloat16 bf2 = BFloat16(h);
    // CHECK: h2bf=2
    printf("h2bf=%d\n", int(float(bf2)));

    // int -> BFloat16 -> float
    BFloat16 bf3 = BFloat16(5);
    // CHECK: i2bf=5
    printf("i2bf=%d\n", int(float(bf3)));

    // BFloat16 -> half -> float
    BFloat16 bf4 = BFloat16(4.0);
    half h2 = half(bf4);
    // CHECK: bf2h=4
    printf("bf2h=%d\n", int(float(h2)));

    // BFloat16 -> double
    double d = double(bf4);
    // CHECK: bf2d=4
    printf("bf2d=%d\n", int(d));

    // MakeVectorFromScalar
    BFloat16 scalar = BFloat16(7.0);
    vector<BFloat16, 3> v = vector<BFloat16, 3>(scalar);
    // CHECK: splat=7,7,7
    printf("splat=%d,%d,%d\n", int(float(v[0])), int(float(v[1])), int(float(v[2])));

    // __floatAsInt for BFloat16 (constant folding)
    // BFloat16(1.0) has bit pattern 0x3F80 (sign=0, exp=01111111, mantissa=0000000)
    int16_t bits = __floatAsInt(BFloat16(1.0));
    // CHECK: bf16bits=16256
    printf("bf16bits=%d\n", int(bits));
}
