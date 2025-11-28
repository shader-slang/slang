//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly

// Test for debug info generation for generic struct types
// This test ensures that DebugLocalVariable and DebugTypeComposite are properly
// generated for specialized generic struct types.

struct GenericStruct<T>
{
    T value;
};

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    GenericStruct<int> gs_int;
    gs_int.value = 1;

    GenericStruct<float> gs_float;
    gs_float.value = 2.0f;

    outputBuffer[0] = gs_int.value * int(gs_float.value);
}

// Verify that debug info is generated for both specializations
// CHECK-DAG: DebugTypeMember
// CHECK-DAG: DebugTypeComposite

// Verify DebugLocalVariable for gs_int and gs_float
// CHECK-DAG: gs_int{{.*}}DebugLocalVariable
// CHECK-DAG: gs_float{{.*}}DebugLocalVariable

// Verify DebugDeclare instructions associate debug variables with actual variables
// CHECK-DAG: DebugDeclare

// Verify the function ends properly  
// CHECK: OpFunctionEnd
