// Arrays whose elements have no physical representation disappear during type legalization too.
// A zero cast lowers directly to MakeStruct with a MakeArrayFromElement operand before payload
// legalization. Adding the outer dummy field must preserve that original constructor operand
// until type legalization removes the empty array; replacing it prematurely invalidates the IR.
// Brace initialization in the callee also covers the ordinary aggregate constructor path.

//TEST:SIMPLE(filecheck=HLSL_CALLER): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=HLSL_CALLEE): -stage callable -entry callableMain -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry rgen -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -stage callable -entry callableMain -target dxil -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -stage raygeneration -entry rgen -target spirv -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -stage raygeneration -entry rgen -target glsl -validate-ir
//TEST:SIMPLE(filecheck=CUDA_CALLER): -stage raygeneration -entry rgen -target cuda -validate-ir
//TEST:SIMPLE(filecheck=CUDA_CALLEE): -stage callable -entry callableMain -target cuda -validate-ir
//TEST:SIMPLE(filecheck=IR): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7 -dump-ir-after legalizeRayTracingPayloads

struct Empty
{
}

struct EmptyArrayPayload
{
    Empty values[2];
}

[shader("callable")]
void callableMain(inout EmptyArrayPayload payload)
{
    payload = { { {}, {} } };
}

[shader("raygeneration")]
void rgen()
{
    EmptyArrayPayload payload = (EmptyArrayPayload)0;
    CallShader(0, payload);
}

// The array and its empty element type must disappear. Only the outer payload gets one int.
// HLSL_CALLER: struct EmptyArrayPayload
// HLSL_CALLER: {
// HLSL_CALLER-NEXT: int _slang_dummy{{.*}};
// HLSL_CALLER-NEXT: };
// HLSL_CALLER: CallShader(0U, {{[A-Za-z0-9_]+}});
// HLSL_CALLEE: struct EmptyArrayPayload
// HLSL_CALLEE: {
// HLSL_CALLEE-NEXT: int _slang_dummy{{.*}};
// HLSL_CALLEE-NEXT: };
// HLSL_CALLEE: void callableMain(inout EmptyArrayPayload{{.*}} {{[A-Za-z0-9_]+}})
// DXIL: define void @

// The intermediate constructor must still have an operand for every field, including the empty
// array, until type legalization removes that field. Successful final emission alone can hide a
// malformed intermediate constructor because its empty fields are eventually discarded.
// IR: AFTER legalizeRayTracingPayloads
// IR: let %[[elements:[0-9A-Za-z_]+]] : Array({{.*}}) = makeArrayFromElement(
// IR: : %EmptyArrayPayload = makeStruct(%[[elements]], 0 : Int)

// SPIRV: %[[payload:[0-9A-Za-z_]+]] = OpVariable %{{[0-9A-Za-z_]+}} CallableDataKHR
// SPIRV: OpExecuteCallableKHR %{{[0-9A-Za-z_]+}} %[[payload]]
// GLSL-DAG: callableDataEXT
// GLSL: executeCallableEXT(

// CUDA_CALLER-NOT: _slang_dummy
// CUDA_CALLER-LABEL: void __raygen__rgen()
// CUDA_CALLER: optixDirectCall<void>(0U);
// CUDA_CALLEE-NOT: _slang_dummy
// CUDA_CALLEE: void __direct_callable__callableMain()
