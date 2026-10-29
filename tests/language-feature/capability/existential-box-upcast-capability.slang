//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2025 -target spirv -emit-spirv-directly -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -target spirv -emit-spirv-directly -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 202c -target spirv -emit-spirv-directly -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -std 202c -DALLOW_NON_UNIFORM -target spirv -emit-spirv-directly -entry main -stage compute

interface IBase {}
interface IDerived : IBase {}
struct Concrete : IDerived {}

[require(spvShaderNonUniform)]
dyn IDerived makeDerived()
/*CHECK:
             ^^^^^^^^^^^ see definition of 'makeDerived'
*/
{
    return Concrete();
}

#ifdef ALLOW_NON_UNIFORM
[require(spirv, shaderclock, spvShaderNonUniform)]
#else
[require(spirv, shaderclock)]
#endif
public void main()
/*CHECK:
            ^^^^ undeclared capability used
            ^^^^ 'main' uses undeclared capability 'spvShaderNonUniform'
*/
{
    // The upcast binds the call result in a temporary before opening the box. Capability
    // checking must visit that temporary's initializer to find the call's requirement.
    dyn IBase value = makeDerived();
/*CHECK:
                      ^^^^^^^^^^^ see using of 'makeDerived'
*/
}

//SPIRV: OpEntryPoint GLCompute
