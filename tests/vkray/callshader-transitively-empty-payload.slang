// A payload whose only field is itself empty is erased by type legalization. Targets that need
// physical storage must use a separate carrier. Compile caller and callee separately so the D3D
// entry-point parameter cannot rely on discovering a CallShader call in the same module. OptiX permits
// the logical payload to contribute no caller argument or callee parameter.

//TEST:SIMPLE(filecheck=HLSL_CALLER): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=HLSL_CALLEE): -stage callable -entry callableMain -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry rgen -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage callable -entry callableMain -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry rgen -target spirv -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry rgen -target glsl -validate-ir
//TEST:SIMPLE(filecheck=CUDA_CALLER): -stage raygeneration -entry rgen -target cuda -validate-ir
//TEST:SIMPLE(filecheck=CUDA_CALLEE): -stage callable -entry callableMain -target cuda -validate-ir

struct Empty
{
}

struct TransitivelyEmpty
{
    Empty value;
}

[shader("callable")]
void callableMain(inout TransitivelyEmpty data)
{
}

[shader("raygeneration")]
void rgen()
{
    TransitivelyEmpty data;
    CallShader(0, data);
}

// Type legalization removes the original nested logical types. Required D3D payload slots then
// receive one-int objects in both independently compiled callers and callees.
// HLSL_CALLER: struct [[carrier:[A-Za-z0-9_]+]]
// HLSL_CALLER: {
// HLSL_CALLER-NEXT: int _slang_dummy{{.*}};
// HLSL_CALLER-NEXT: };
// HLSL_CALLER: [[carrier]] [[payload:[A-Za-z0-9_]+]];
// HLSL_CALLER: CallShader(0U, [[payload]]);
// HLSL_CALLEE: struct [[carrier:[A-Za-z0-9_]+]]
// HLSL_CALLEE: {
// HLSL_CALLEE-NEXT: int _slang_dummy{{.*}};
// HLSL_CALLEE-NEXT: };
// HLSL_CALLEE: void callableMain(inout [[carrier]] {{[A-Za-z0-9_]+}})
// DXIL: define void @

// SPIRV: %[[payload:[0-9A-Za-z_]+]] = OpVariable %{{[0-9A-Za-z_]+}} CallableDataKHR
// SPIRV: OpExecuteCallableKHR %{{[0-9A-Za-z_]+}} %[[payload]]
// GLSL-DAG: callableDataEXT
// GLSL: executeCallableEXT(

// CUDA_CALLER-NOT: _slang_dummy
// CUDA_CALLER-LABEL: void __raygen__rgen()
// CUDA_CALLER: optixDirectCall<void>(0U);
// CUDA_CALLEE-NOT: _slang_dummy
// CUDA_CALLEE: void __direct_callable__callableMain()
