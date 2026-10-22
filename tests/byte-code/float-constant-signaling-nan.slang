//TEST:INTERPRET(filecheck=CHECK):

// A folded `bit_cast<float>` of a signaling NaN is a `float` constant that keeps its bits. The
// byte-code emitter must write those bits, not narrow the IR's double with `(float)`, which would
// quiet the NaN (0x7F800001 would become 0x7FC00001).
//
// The helper keeps the constant out of reach of constant folding, so the interpreter reads it
// from the constant section at run time.
uint bitsOf(float f)
{
    return bit_cast<uint>(f);
}

void main()
{
    //CHECK: 0x7f800001
    printf("0x%08x\n", bitsOf(bit_cast<float>(0x7F800001u)));
    //CHECK: 0xff800001
    printf("0x%08x\n", bitsOf(bit_cast<float>(0xFF800001u)));
    //CHECK: 0x7fc12345
    printf("0x%08x\n", bitsOf(bit_cast<float>(0x7FC12345u)));
    //CHECK: 0x3f800000
    printf("0x%08x\n", bitsOf(1.0));
}
