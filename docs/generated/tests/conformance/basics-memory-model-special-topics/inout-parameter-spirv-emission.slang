//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`inout` parameter: SPIRV emits the argument as a Function-storage-class pointer, allowing multiple reads and writes within the function body.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: the function may read and write the argument passed to
// the inout-parameter any number of times during the function call.
// In SPIRV, Slang implements this by passing inout arguments as
// Function-storage-class pointers (OpTypePointer Function), enabling
// the callee to issue multiple OpLoad (read) and OpStore (write) operations.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void multiReadWrite(inout int val)
{
    // read val twice (two reads of the inout parameter)
    int r1 = val;
    int r2 = val;
    // write val once (first write)
    val = r1 + r2;
    // read val again (third read after first write)
    int r3 = val;
    // write val again (second write)
    val = r3 * 2;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = output[0];
    multiReadWrite(x);
    output[0] = x;
}

// CHECK: %_ptr_Function_int = OpTypePointer Function %int
// CHECK: %multiReadWrite = OpFunction %void DontInline
// CHECK: %val = OpFunctionParameter %_ptr_Function_int
// CHECK: OpLoad %int %val
// CHECK: OpLoad %int %val
// CHECK: OpStore %val
