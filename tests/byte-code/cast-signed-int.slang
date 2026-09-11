//TEST:INTERPRET(filecheck=CHECK):

// Regression for the signed-integer cast bug in the byte-code interpreter (shader-slang/slang#12871):
// the cast dispatchers in slang-vm-inst-impl.cpp routed signed-int casts through unsigned C++ types,
// so float->signed truncation of a negative value gave 0 on aarch64 (fcvtzu saturates) and signed->float
// read the operand as unsigned on every platform.
//
// The helpers keep the values out of reach of constant folding, so the casts are really performed by
// the interpreter at run time (a folded `(int)(-9.0)` prints -9 even on a broken build).
int negInt(int a, int b)
{
    return a * b;
}
float negFloat(float a, float b)
{
    return a * b;
}

void main()
{
    int i = negInt(3, -3);
    float f = negFloat(3.0, -3.0);

    //CHECK: float->int -9
    printf("float->int %d\n", (int)f);
    //CHECK: float->int64 -9
    printf("float->int64 %lld\n", (int64_t)f);
    //CHECK: int->float -9.000000
    printf("int->float %f\n", (float)i);
    //CHECK: int->int16->float -9.000000
    printf("int->int16->float %f\n", (float)(int16_t)i);
    //CHECK: positive 9 9
    printf("positive %d %u\n", (int)negFloat(3.0, 3.0), (uint)negFloat(3.0, 3.0));
}
