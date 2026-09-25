// Inner and Outer are both independent callable payloads, and Inner is also a field of Outer.
// Each physical payload must have one uint, even though Outer contains Inner. Its layout
// must match independently compiled callable entries, and must not depend on caller source order.
// Separate boundary carriers leave both shared logical types unchanged.

//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=HLSL_REVERSED): -stage raygeneration -entry rgenReversed -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry rgen -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry rgenReversed -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry rgen -target spirv -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry rgen -target glsl -validate-ir
//TEST:SIMPLE(filecheck=HLSL_CALLEE): -stage callable -entry innerCallee -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=HLSL_CALLEE): -stage callable -entry outerCallee -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage callable -entry innerCallee -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage callable -entry outerCallee -target dxil -profile sm_6_7 -validate-ir

struct Inner
{
}

struct Outer
{
    Inner inner;
}

[shader("callable")]
void innerCallee(inout Inner payload)
{
}

[shader("callable")]
void outerCallee(inout Outer payload)
{
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

// Both logical types may share the same dummy type. No original inner field may gain storage.
// HLSL-NOT: inner_{{[0-9]+}};
// HLSL: struct {{[A-Za-z0-9_]+}}
// HLSL-NEXT: {
// HLSL-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// HLSL-NEXT: };
// HLSL-NOT: inner_{{[0-9]+}};
// HLSL: CallShader(0U, {{[A-Za-z0-9_]+}});
// HLSL: CallShader(1U, {{[A-Za-z0-9_]+}});
// HLSL-NOT: inner_{{[0-9]+}};
// HLSL_REVERSED-NOT: inner_{{[0-9]+}};
// HLSL_REVERSED: struct {{[A-Za-z0-9_]+}}
// HLSL_REVERSED-NEXT: {
// HLSL_REVERSED-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// HLSL_REVERSED-NEXT: };
// HLSL_REVERSED-NOT: inner_{{[0-9]+}};
// HLSL_REVERSED: CallShader(1U, {{[A-Za-z0-9_]+}});
// HLSL_REVERSED: CallShader(0U, {{[A-Za-z0-9_]+}});
// HLSL_REVERSED-NOT: inner_{{[0-9]+}};
// HLSL_CALLEE: struct [[carrier:[A-Za-z0-9_]+]]
// HLSL_CALLEE-NEXT: {
// HLSL_CALLEE-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// HLSL_CALLEE-NEXT: };
// HLSL_CALLEE: void {{innerCallee|outerCallee}}(inout [[carrier]] {{[A-Za-z0-9_]+}})
// DXIL: define void @

// SPIRV: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV: %[[carrier:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer CallableDataKHR %[[carrier]]
// SPIRV: = OpVariable %[[pointer]] CallableDataKHR
// SPIRV-COUNT-2: OpExecuteCallableKHR %{{[0-9A-Za-z_]+}} %{{[0-9A-Za-z_]+}}
// GLSL-NOT: inner_{{[0-9]+}};
// GLSL: struct {{[A-Za-z0-9_]+}}
// GLSL-NEXT: {
// GLSL-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// GLSL-NEXT: };
// GLSL-NOT: inner_{{[0-9]+}};
// GLSL: callableDataEXT
// GLSL: executeCallableEXT(
// GLSL: executeCallableEXT(
// GLSL-NOT: inner_{{[0-9]+}};
