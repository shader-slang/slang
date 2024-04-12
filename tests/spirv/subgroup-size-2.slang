// Test that using workgroup size from more than one entrypoint result in
// correct lowering into global variable.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -fvk-use-entrypoint-name -O0

RWStructuredBuffer<int> outputBuffer;

uint3 f() { return WorkgroupSize(); }

[shader("compute")]
[numthreads(1u, 2u, 3)]
void compute1()
{
    // CHECK-DAG: %[[VAR:[A-Za-z0-9_]+]] = OpVariable %_ptr_Private_v3int Private
    // CHECK: OpStore %[[VAR]]

    // CHECK-DAG: %[[CALL_RS:[A-Za-z0-9_]+]] = OpFunctionCall %v3uint %f
    // CHECK: OpCompositeExtract %uint %[[CALL_RS]] 0
    const int x = f().x;
    outputBuffer[0] = x;

    // CHECK-DAG: %[[PTR:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_int %outputBuffer %int_0 %uint_1
    // CHECK: OpStore %[[PTR]] %int_2
    outputBuffer[1] = WorkgroupSize().y;
}

[shader("compute")]
[numthreads(4, 5, 6)]
void compute2()
{
    const int x = f().x;
    outputBuffer[0] = x;
}