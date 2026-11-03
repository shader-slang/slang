//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir

// Regression test for https://github.com/shader-slang/slang/issues/13841
// Address-space specialization clones `bumpValue` for its `groupshared` argument and gives the
// clone's parameter the Workgroup storage class. The SPIR-V emitter declares a debug variable
// only on Function-storage memory, so the clone's `value` keeps its `_dbgvar_` copy, while the
// original, called with a local, declares `value` on its parameter.

groupshared float sharedValue;
RWStructuredBuffer<float> outputBuffer;

[noinline]
void bumpValue(inout float value)
{
    value += 1;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float local = outputBuffer[0];
    bumpValue(local);
    sharedValue = outputBuffer[1];
    GroupMemoryBarrierWithGroupSync();
    bumpValue(sharedValue);
    outputBuffer[0] = local + sharedValue;
}

// CHECK-LABEL: %bumpValue = OpFunction
// CHECK: [[FUNCTION_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK: DebugDeclare {{%[A-Za-z_0-9]+}} [[FUNCTION_PARAM]] {{%[0-9]+}}{{$}}
// CHECK-LABEL: %bumpValue_0 = OpFunction
// CHECK: OpFunctionParameter %_ptr_Workgroup_float
// CHECK: DebugDeclare {{%[A-Za-z_0-9]+}} %_dbgvar_value {{%[0-9]+}}{{$}}
