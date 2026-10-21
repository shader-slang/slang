//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -stage compute -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

// Taking the address of an operand that has no address is error E29103:
// `&buf` is a global resource, `&v.xy` is a multi-element swizzle, and `&c`
// is a `const` parameter of a helper that `main` calls. These asm blocks are
// reachable from `main`, so before E29103 was an error they reached the back
// end: SPIR-V emission crashed and HLSL reported an internal error.

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
