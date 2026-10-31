// An `out` parameter of a non-inlined function points at the caller's storage,
// so its debug variable is declared on the OpFunctionParameter itself instead
// of on a shadow variable that each store into the parameter would update.
//
// The pattern below avoids spelling the shadow-variable name literally because
// -g2 embeds this source file in an OpSource string.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=NOSHADOW): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir

RWStructuredBuffer<float> outputBuffer;

[noinline]
void produce(out float result, int seed)
{
    result = float(seed) * 2.0;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float value;
    produce(value, int(tid.x));
    outputBuffer[0] = value;
}

// CHECK: [[R_STR:%[0-9]+]] = OpString "result"
// CHECK: [[R_DBG:%[A-Za-z0-9_]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[R_STR]]
// CHECK: [[R_PARAM:%result[_0-9]*]] = OpFunctionParameter %_ptr_Function_float
// CHECK: DebugDeclare [[R_DBG]] [[R_PARAM]]
// CHECK-NOT: DebugValue [[R_DBG]]

// NOSHADOW-NOT: %{{_}}dbgvar_result
