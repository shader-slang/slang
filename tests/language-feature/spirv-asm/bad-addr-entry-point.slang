//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -stage compute -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

// Taking the address of an operand that has no address is an error. The asm
// block sits in an entry point so that the IR passes and emitters after
// lowering would see the operand; bad-addr.slang only reaches lowering.

RWStructuredBuffer<float> buf;

float load(const float c)
{
    return spirv_asm { result:$$float = OpLoad &c; };
//CHECK:                                        ^ error E29103
}

[numthreads(1, 1, 1)]
void main()
{
    float4 v = 0;
    spirv_asm
    {
        OpDecorate &buf RelaxedPrecision;
//CHECK:            ^^^ error E29103
        OpDecorate &v.xy RelaxedPrecision;
//CHECK:              ^^ error E29103
    };
    buf[0] = v.x + load(buf[1]);
}
