// Inner and Outer are both independent callable payloads, and Inner is also a field of Outer.
// Collect emptiness before padding either carrier: padding Inner first must not invalidate the
// decision to preserve Outer. Both source orders must compile with both payload operands present.

//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=HLSL_REVERSED): -stage raygeneration -entry rgenReversed -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry rgen -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry rgenReversed -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry rgen -target spirv -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry rgen -target glsl -validate-ir

struct Inner
{
}

struct Outer
{
    Inner inner;
}

[shader("raygeneration")]
void rgen()
{
    Inner inner;
    Outer outer;
    CallShader(0, inner);
    CallShader(1, outer);
}

[shader("raygeneration")]
void rgenReversed()
{
    Outer outer;
    Inner inner;
    CallShader(1, outer);
    CallShader(0, inner);
}

// HLSL: CallShader(0U, {{[A-Za-z0-9_]+}});
// HLSL: CallShader(1U, {{[A-Za-z0-9_]+}});
// HLSL_REVERSED: CallShader(1U, {{[A-Za-z0-9_]+}});
// HLSL_REVERSED: CallShader(0U, {{[A-Za-z0-9_]+}});
// DXIL: define void @

// SPIRV-COUNT-2: = OpVariable %{{[0-9A-Za-z_]+}} CallableDataKHR
// SPIRV-COUNT-2: OpExecuteCallableKHR %{{[0-9A-Za-z_]+}} %{{[0-9A-Za-z_]+}}
// GLSL: callableDataEXT
// GLSL: callableDataEXT
// GLSL: executeCallableEXT(
// GLSL: executeCallableEXT(
