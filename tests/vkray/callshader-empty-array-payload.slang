// Arrays whose elements have no physical representation disappear during type legalization too.
// A zero cast lowers directly to MakeStruct with a MakeArrayFromElement operand before payload
// legalization. The ray-tracing pass must leave this logical constructor and type unchanged;
// it supplies the required physical payload object before type legalization removes the array.
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

// The original EmptyArrayPayload contains only Empty values[2]. General type legalization erases
// it, but the CallShader argument and callable entry-point parameter are required ABI slots.
// Those slots receive target-valid dummy objects. The resulting HLSL has this shape:
//
//     struct EmptyArrayPayloadCarrier { uint _slang_dummy; };
//     void rgen()
//     {
//         EmptyArrayPayloadCarrier carrier = { 0 };
//         CallShader(0U, carrier);
//     }
//     void callableMain(inout EmptyArrayPayloadCarrier payload) {}
//
// Check the one-field representation in both independently compiled entry points without
// depending on the compiler-generated type name. No nested empty logical type survives.
// HLSL_CALLER: struct [[carrier:[A-Za-z0-9_]+]]
// HLSL_CALLER: {
// HLSL_CALLER-NEXT: uint _slang_dummy{{.*}};
// HLSL_CALLER-NEXT: };
// HLSL_CALLER: CallShader(0U, {{[A-Za-z0-9_]+}});
// HLSL_CALLEE: struct [[carrier:[A-Za-z0-9_]+]]
// HLSL_CALLEE: {
// HLSL_CALLEE-NEXT: uint _slang_dummy{{.*}};
// HLSL_CALLEE-NEXT: };
// HLSL_CALLEE: void callableMain(inout [[carrier]] {{[A-Za-z0-9_]+}})
// DXIL: define void @

// The logical constructor retains its original array operand and must not gain an ABI dummy
// field. Successful final emission alone can hide a malformed intermediate
// constructor because its empty fields are eventually discarded.
// IR: AFTER legalizeRayTracingPayloads
// IR: let %[[elements:[0-9A-Za-z_]+]] : Array({{.*}}) = makeArrayFromElement(
// IR: : %EmptyArrayPayload{{[0-9]*}} = makeStruct(%[[elements]])

// SPIRV: %[[payload:[0-9A-Za-z_]+]] = OpVariable %{{[0-9A-Za-z_]+}} CallableDataKHR
// SPIRV: OpExecuteCallableKHR %{{[0-9A-Za-z_]+}} %[[payload]]
// GLSL-DAG: callableDataEXT
// GLSL: executeCallableEXT(

// CUDA_CALLER-NOT: _slang_dummy
// CUDA_CALLER-LABEL: void __raygen__rgen()
// CUDA_CALLER: optixDirectCall<void>(0U);
// CUDA_CALLEE-NOT: _slang_dummy
// CUDA_CALLEE: void __direct_callable__callableMain()
