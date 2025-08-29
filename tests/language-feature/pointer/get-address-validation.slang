//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests for invalid/valid use of `__getAddress`

struct DeviceStruct
{
    int data1;
    int data2;
}

struct StructPtrInStruct
{
    DeviceStruct* ptr;
}

uniform int* bufferUserPointer;
RWStructuredBuffer<int> bufferStorage;
groupshared int bufferGroupShared[100];
uniform DeviceStruct* bufferUserPointerStruct;
uniform int2* bufferUserPointerVector;

int* output;

typealias GroupSharedPtr<T> = Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>;

GroupSharedPtr<T> paramGroupShared<T : __BuiltinIntegerType>(out groupshared T[100] ptr)
{
    // CHECK: ([[# @LINE+1]]): error 30019
    T* ptr1 = __getAddress(ptr[5]);

    // CHECK-NOT: ([[# @LINE+1]]): error
    GroupSharedPtr<T> ptr2 = __getAddress(ptr[5]);

    return ptr2;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // CHECK: ([[# @LINE+1]]): error 31160
    int* ptr1 = __getAddress(bufferStorage[id.x]);

    // CHECK ([[# @LINE+1]]): error
    int[100]* ptr2 = __getAddress(bufferGroupShared);

    // CHECK: ([[# @LINE+1]]): error
    int* ptr3 = __getAddress(bufferGroupShared[id.x]);

    // CHECK-NOT: ([[# @LINE+1]]): error
    int* ptr4 = __getAddress(bufferUserPointer[id.x]);

    // CHECK-NOT: ([[# @LINE+1]]): error
    GroupSharedPtr<int[100]> ptr5 = __getAddress(bufferGroupShared);

    // CHECK-NOT: ([[# @LINE+1]]): error
    GroupSharedPtr<int> ptr6 = __getAddress(bufferGroupShared[id.x]);

    // CHECK-NOT: ([[# @LINE+1]]): error
    GroupSharedPtr<int> ptr7 = paramGroupShared(bufferGroupShared);

    // CHECK-NOT: ([[# @LINE+1]]): error
    int* ptr8 = __getAddress(bufferUserPointerStruct.data1);

    StructPtrInStruct structPtrInStruct;
    structPtrInStruct.ptr = bufferUserPointerStruct;
    // CHECK-NOT: ([[# @LINE+1]]): error
    int* ptr9 = __getAddress(structPtrInStruct.ptr[id.x].data1);

    // CHECK-NOT: ([[# @LINE+1]]): error
    int* ptr10 = __getAddress(bufferUserPointerVector[0].x);

    output[id] = ptr1[id];
    output[id] = ptr2[id][0];
    output[id] = ptr3[id];
    output[id] = ptr4[id];
    output[id] = ptr5[id];
    output[id] = ptr6[id];
    output[id] = ptr7[id];
    output[id] = ptr8[id];
    output[id] = ptr9[id];
    output[id] = ptr10[id];
}
