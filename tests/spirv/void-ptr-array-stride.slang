//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-directly -entry main -stage compute
//TEST:SIMPLE(filecheck=DEBUG):-target spirv-asm -emit-spirv-directly -entry main -stage compute -g2 -O0

// Check that void* in PhysicalStorageBuffer is emitted as uint* with
// ArrayStride 4 (required by OpPtrAccessChain), and that debug info still
// describes the original source-level void*.

// CHECK-DAG: OpDecorate %[[PTR:_ptr_PhysicalStorageBuffer_uint[A-Za-z0-9_]*]] ArrayStride 4
// CHECK-DAG: %uint = OpTypeInt 32 0
// CHECK-DAG: %[[PTR]] = OpTypePointer PhysicalStorageBuffer %uint
// CHECK: OpPtrAccessChain %[[PTR]]

// DEBUG-DAG: %[[VOID_STR:[A-Za-z_0-9]+]] = OpString "void"
// DEBUG-DAG: %[[P_STR:[A-Za-z_0-9]+]] = OpString "p"
// DEBUG-DAG: %[[VOID_TYPE:[A-Za-z_0-9]+]] = OpExtInst %void %{{[A-Za-z_0-9]+}} DebugTypeBasic %[[VOID_STR]] %uint_0 %uint_0
// DEBUG-DAG: %[[VOID_PTR_TYPE:[A-Za-z_0-9]+]] = OpExtInst %void %{{[A-Za-z_0-9]+}} DebugTypePointer %[[VOID_TYPE]]
// DEBUG-DAG: OpExtInst %void %{{[A-Za-z_0-9]+}} DebugTypeMember %[[P_STR]] %[[VOID_PTR_TYPE]]

cbuffer Params
{
    void* p;
    uint i;
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    void* q = p + i;
    outputBuffer[0] = reinterpret<uint64_t>(q) != 0;
}
