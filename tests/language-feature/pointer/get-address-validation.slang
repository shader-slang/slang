//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests for invalid/valid use of `__getAddress`

int* bufferUserPointer;
RWStructuredBuffer<int> bufferStorage;
groupshared int bufferGroupShared[100];
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
    int* ptr1 = __getAddress(bufferUserPointer[id.x]);

    // CHECK: ([[# @LINE+1]]): error 31160
    int* ptr2 = __getAddress(bufferStorage[id.x]);

    // CHECK: ([[# @LINE+1]]): error 30019
    int[100]* ptr3 = __getAddress(bufferGroupShared);

    // CHECK: ([[# @LINE+1]]): error 30019
    int* ptr4 = __getAddress(bufferGroupShared[id.x]);

    // CHECK-NOT: ([[# @LINE+1]]): error
    GroupSharedPtr<int[100]> ptr5 = __getAddress(bufferGroupShared);

    // CHECK-NOT: ([[# @LINE+1]]): error
    GroupSharedPtr<int> ptr6 = __getAddress(bufferGroupShared[id.x]);

    // CHECK-NOT: ([[# @LINE+1]]): error
    GroupSharedPtr<int> ptr7 = paramGroupShared(bufferGroupShared);

    output[id] = ptr1[id];
    output[id] = ptr2[id];
    output[id] = ptr3[id][0];
    output[id] = ptr4[id];
    output[id] = ptr5[id][0];
    output[id] = ptr6[id];
    output[id] = ptr7[id];
}
