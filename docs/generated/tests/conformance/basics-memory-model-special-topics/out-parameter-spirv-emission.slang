//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`out` parameter: SPIRV emits the argument as a Function-storage-class pointer; the callee issues at least one OpStore (write) to it during the function call.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: the function may write the argument passed to the
// out-parameter any number of times during the function call.
// In SPIRV, `out` arguments are passed as Function-storage-class pointers
// (OpTypePointer Function), and the callee writes to it via OpStore.
// The doc allows multiple writes; here we verify the pointer-passing mechanism
// that enables writes inside the callee (at least one OpStore observed).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void writeOut(out int result, int factor)
{
    // write the out parameter
    result = factor * 2;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    int f = output[0];
    writeOut(r, f);
    output[0] = r;
}

// CHECK: %_ptr_Function_int = OpTypePointer Function %int
// CHECK: %writeOut = OpFunction %void DontInline
// CHECK: %result = OpFunctionParameter %_ptr_Function_int
// CHECK: OpStore %result
