// A `__target_switch` without a case for the current target has no code to generate. When a
// function containing one would still be generated (here because `-ignore-capabilities` disables
// the capability check that would otherwise reject the entry point), the compiler reports E41037
// instead of emitting a function that falls off its end.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry userFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry userFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry userFunc -stage compute -ignore-capabilities

//DIAGNOSTIC_TEST:SIMPLE(diag=VOID):-target cuda -entry userVoidFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=VOID):-target cpp -entry userVoidFunc -stage compute -ignore-capabilities

//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC):-target cuda -entry userGenericFunc -stage compute -ignore-capabilities

// On every target, including those that reject missing returns (SPIR-V, GLSL, WGSL), a switch
// with no case reports E41037 and never the generic missing-return error E41009. E41037 is also
// reported when `-disable-non-essential-validations` skips the missing-return check.
//DIAGNOSTIC_TEST:SIMPLE(diag=METALONLY):-target spirv -entry metalOnlyVoidFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=METALONLY):-target wgsl -entry metalOnlyVoidFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=METALONLY):-target hlsl -entry metalOnlyVoidFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=METALONLY):-target spirv -entry metalOnlyVoidFunc -stage compute -ignore-capabilities -disable-non-essential-validations
//DIAGNOSTIC_TEST:SIMPLE(diag=METALONLY):-target cuda -entry metalOnlyVoidFunc -stage compute -ignore-capabilities -disable-non-essential-validations

// A function that returns a resource but has no case for the target. On SPIR-V, GLSL and WGSL
// E41037 stops compilation before `specializeResourceUsage`, which cannot handle such a function.
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE):-target spirv -entry resourceFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE):-target glsl -entry resourceFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE):-target wgsl -entry resourceFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE):-target cuda -entry resourceFunc -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE):-target cpp -entry resourceFunc -stage compute -ignore-capabilities

// The annotations above are shared by several targets, so the target named in the message is
// pinned separately for each target family. PTX is generated from CUDA source and reports 'cuda';
// its lane is a FileCheck test so that it only runs where NVRTC is available.
//TEST:SIMPLE(filecheck=PIN_CUDA):-target cuda -entry userFunc -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_CUDA):-target ptx -entry userFunc -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_CPP):-target cpp -entry userVoidFunc -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_METAL):-target metal -entry userFunc -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_HLSL):-target hlsl -entry metalOnlyVoidFunc -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_SPIRV):-target spirv -entry metalOnlyVoidFunc -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_GLSL):-target glsl -entry resourceFunc -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_WGSL):-target wgsl -entry metalOnlyVoidFunc -stage compute -ignore-capabilities
// PIN_CUDA: 'twice' contains a __target_switch with no case for target 'cuda'
// PIN_CPP: 'setToTwo' contains a __target_switch with no case for target 'cpp'
// PIN_METAL: 'twice' contains a __target_switch with no case for target 'metal'
// PIN_HLSL: 'metalOnlySet' contains a __target_switch with no case for target 'hlsl'
// PIN_SPIRV: 'metalOnlySet' contains a __target_switch with no case for target 'spirv'
// PIN_GLSL: 'pickTexture' contains a __target_switch with no case for target 'glsl'
// PIN_WGSL: 'metalOnlySet' contains a __target_switch with no case for target 'wgsl'

RWStructuredBuffer<float> output;

float twice(float x)
{
    __target_switch
/*CHECK:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'twice' contains a __target_switch with no case for target
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
    ^^^^^^^^^^^^^^^ 'setToTwo' contains a __target_switch with no case for target
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
    ^^^^^^^^^^^^^^^ 'twiceGeneric' contains a __target_switch with no case for target
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
/*METALONLY:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'metalOnlySet' contains a __target_switch with no case for target
*/
    {
    case metal:
        x = 2;
    }
}

[numthreads(1, 1, 1)]
void metalOnlyVoidFunc()
{
    float v = 1;
    metalOnlySet(v);
//METALONLY:    ^ see call to 'metalOnlySet'
    output[0] = v;
}

Texture2D<float4> tex;
RWStructuredBuffer<float4> output4;

Texture2D<float4> pickTexture()
{
    __target_switch
/*RESOURCE:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'pickTexture' contains a __target_switch with no case for target
*/
    {
    case metal:
        return tex;
    }
}

[numthreads(1, 1, 1)]
void resourceFunc()
{
    output4[0] = pickTexture().Load(int3(0));
//RESOURCE:                 ^ see call to 'pickTexture'
}
