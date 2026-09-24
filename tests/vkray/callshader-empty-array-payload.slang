// Arrays whose elements have no physical representation disappear during type legalization too.
// A zero cast lowers directly to MakeStruct with a MakeArrayFromElement operand before payload
// legalization. Creating an ABI carrier must leave this logical constructor and type unchanged
// until type legalization removes the empty array.
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

// Consider the carrier created before type legalization (shown here as source-like code):
//
//     struct EmptyArrayPayloadCarrier
//     {
//         EmptyArrayPayload _slang_data;
//         int _slang_dummy;
//     };
//
// The original EmptyArrayPayload still contains only Empty values[2]. Type legalization removes
// that array and then the carrier's now-empty _slang_data field. The emitted HLSL is therefore
// equivalent to the following, with compiler-generated suffixes omitted:
//
//     struct EmptyArrayPayloadCarrier { int _slang_dummy; };
//     void rgen()
//     {
//         EmptyArrayPayloadCarrier carrier = { 0 };
//         CallShader(0U, carrier);
//     }
//     void callableMain(inout EmptyArrayPayloadCarrier payload) {}
//
// The carrier inherits the logical payload's name hint, so the emitted name starts with
// EmptyArrayPayload rather than literally ending in Carrier. The checks verify its exact
// one-field body in both independently compiled entry points; no nested empty type survives.
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

// The logical constructor retains its original array operand and must not gain the physical
// carrier's dummy field. Successful final emission alone can hide a malformed intermediate
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
