// A `__target_switch` without a case for the current target has no code to generate. When that
// code is still reachable (here because `-ignore-capabilities` disables the capability check that
// would otherwise reject the entry point), the compiler reports E41037 instead of silently
// emitting a function that falls off its end.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry userFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target ptx -entry userFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry userFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry userFunc -stage compute -ignore-capabilities

//DIAGNOSTIC_TEST:SIMPLE(diag=VOID):-target cuda -entry userVoidFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=VOID):-target cpp -entry userVoidFunc -stage compute -ignore-capabilities

//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC):-target cuda -entry userGenericFunc -stage compute -ignore-capabilities

// On SPIR-V the switch below has no `spirv` case. E41037 replaces the generic missing-return error
// E41009, which would point at no source location and call the void function "non-void".
//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV):-target spirv -entry spirvVoidFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV):-target wgsl -entry spirvVoidFunc -stage compute -ignore-capabilities

RWStructuredBuffer<float> output;

float twice(float x)
{
    __target_switch
/*CHECK:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'twice' has no implementation for target
*/
    {
    case hlsl:
        __intrinsic_asm "($0*2)";
    case spirv:
        return x * 2;
    }
}

[numthreads(1, 1, 1)]
void userFunc()
{
    output[0] = twice(1.0);
//CHECK:             ^ see call to 'twice'
}

void setToTwo(inout float x)
{
    __target_switch
/*VOID:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'setToTwo' has no implementation for target
*/
    {
    case hlsl:
        __intrinsic_asm "($0 = 2)";
    case spirv:
        x = 2;
    }
}

[numthreads(1, 1, 1)]
void userVoidFunc()
{
    float v = 1;
    setToTwo(v);
//VOID:     ^ see call to 'setToTwo'
    output[0] = v;
}

T twiceGeneric<T : __BuiltinFloatingPointType>(T x)
{
    __target_switch
/*GENERIC:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'twiceGeneric' has no implementation for target
*/
    {
    case hlsl:
        __intrinsic_asm "($0*2)";
    case spirv:
        return x * T(2);
    }
}

[numthreads(1, 1, 1)]
void userGenericFunc()
{
    output[0] = twiceGeneric(1.0f);
//GENERIC:                  ^ see call to 'twiceGeneric'
}

void metalOnlySet(inout float x)
{
    __target_switch
/*SPIRV:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'metalOnlySet' has no implementation for target
*/
    {
    case metal:
        x = 2;
    }
}

[numthreads(1, 1, 1)]
void spirvVoidFunc()
{
    float v = 1;
    metalOnlySet(v);
//SPIRV:        ^ see call to 'metalOnlySet'
    output[0] = v;
}
