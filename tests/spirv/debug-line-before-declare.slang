//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -emit-spirv-directly

// Minimal reproduction: any function with at least one parameter
// triggers the bug where DebugDeclare appears between DebugScope and DebugLine.

RWStructuredBuffer<int> outputBuffer;

void funcWithParam(int x)
{
    outputBuffer[0] = x;
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    funcWithParam(42);
}

// CHECK: ; Function funcWithParam
// CHECK: %funcWithParam = OpFunction
// CHECK: DebugFunctionDefinition {{%[0-9]+}} %funcWithParam
// CHECK-NEXT: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugScope
// CHECK-NEXT: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugLine
