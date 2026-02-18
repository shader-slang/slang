//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -emit-spirv-directly -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_IGNORE_CAPS,non-exhaustive): -target spirv -emit-spirv-directly -entry main -stage compute -ignore-capabilities -skip-spirv-validation

// Test that capabilities can be declared on module.
[require(glsl)]
[require(spirv)]
module test;

RWStructuredBuffer<int> sideEffect;

[require(glsl, _sm_4_0)]
/*CHECK:
 ^^^^^^^ using internal capability name
*/
public void use1()
//CHECK:    ^^^^ dependencies not compatible on target
{
    __target_switch
    {
    case glsl:
//CHECK: ^^^^ conflicting capability requirement
        return;
    }
}

void use2Sub()
{
    __target_switch
    {
    case glsl:
        sideEffect[1] = 1;
    }
}
[require(spirv, spirv_1_0)]
public void use2()
//CHECK:    ^^^^ dependencies not compatible on target
{
    use2Sub();
}

void main()
{
    use1();
    use2();
}
