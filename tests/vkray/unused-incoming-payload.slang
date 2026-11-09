// An incoming payload is part of the shader interface even if the body never reads or writes it.
// Empty payloads use synthesized globals; nonempty parameters become ordinary global parameters.
// Both carry an entry-point DependsOn decoration that direct SPIR-V emission must honor.

//TEST:SIMPLE(filecheck=RAY): -entry emptyMiss -target spirv -emit-spirv-directly -fvk-use-entrypoint-name -validate-ir
//TEST:SIMPLE(filecheck=RAY): -entry nonemptyMiss -target spirv -emit-spirv-directly -fvk-use-entrypoint-name -validate-ir
//TEST:SIMPLE(filecheck=CALLABLE): -entry emptyCallable -target spirv -emit-spirv-directly -fvk-use-entrypoint-name -validate-ir
//TEST:SIMPLE(filecheck=CALLABLE): -entry nonemptyCallable -target spirv -emit-spirv-directly -fvk-use-entrypoint-name -validate-ir
//TEST:SIMPLE(filecheck=MULTI): -entry emptyMiss -entry nonemptyCallable -target spirv -emit-spirv-directly -fvk-use-entrypoint-name -validate-ir

struct EmptyPayload
{
}

struct NonemptyPayload
{
    uint value;
}

[shader("miss")]
void emptyMiss(inout EmptyPayload payload)
{
}

[shader("miss")]
void nonemptyMiss(inout NonemptyPayload payload)
{
}

[shader("callable")]
void emptyCallable(inout EmptyPayload payload)
{
}

[shader("callable")]
void nonemptyCallable(inout NonemptyPayload payload)
{
}

// The interface operand must name the actual incoming variable, not merely a declared type.
// RAY: OpEntryPoint MissKHR %{{[A-Za-z0-9_]+}} "{{[A-Za-z0-9_]+}}" %[[payload:[A-Za-z0-9_]+]]{{$}}
// RAY: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// RAY: %[[payloadType:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// RAY: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer IncomingRayPayloadKHR %[[payloadType]]
// RAY: %[[payload]] = OpVariable %[[pointer]] IncomingRayPayloadKHR
// RAY: = OpFunction
// RAY-NOT: OpLoad
// RAY-NOT: OpStore
// RAY: OpFunctionEnd

// CALLABLE: OpEntryPoint CallableKHR %{{[A-Za-z0-9_]+}} "{{[A-Za-z0-9_]+}}" %[[payload:[A-Za-z0-9_]+]]{{$}}
// CALLABLE: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// CALLABLE: %[[payloadType:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// CALLABLE: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer IncomingCallableDataKHR %[[payloadType]]
// CALLABLE: %[[payload]] = OpVariable %[[pointer]] IncomingCallableDataKHR
// CALLABLE: = OpFunction
// CALLABLE-NOT: OpLoad
// CALLABLE-NOT: OpStore
// CALLABLE: OpFunctionEnd

// Each entry point must list only its own incoming interface, even when both globals are emitted.
// MULTI-DAG: OpEntryPoint MissKHR %{{[A-Za-z0-9_]+}} "emptyMiss" %[[rayPayload:[A-Za-z0-9_]+]]{{$}}
// MULTI-DAG: OpEntryPoint CallableKHR %{{[A-Za-z0-9_]+}} "nonemptyCallable" %[[callablePayload:[A-Za-z0-9_]+]]{{$}}
// MULTI-DAG: %[[rayPayload]] = OpVariable %{{[A-Za-z0-9_]+}} IncomingRayPayloadKHR
// MULTI-DAG: %[[callablePayload]] = OpVariable %{{[A-Za-z0-9_]+}} IncomingCallableDataKHR
