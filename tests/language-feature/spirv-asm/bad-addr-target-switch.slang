//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

// E29103 is reported during lowering, before target-specific code is chosen,
// so a bad address-of operand in a `case spirv:` arm is an error even when
// compiling for HLSL.

RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    __target_switch
    {
    case spirv:
        spirv_asm { OpDecorate &buf RelaxedPrecision; };
//CHECK:                        ^^^ error E29103
    default:
        break;
    }
    buf[0] = 1;
}
