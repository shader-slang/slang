//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive):-stage compute -entry computeMain -target spirv

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
    T* ptr1 = __getAddress(ptr[5]);
//diag:       ^^^^^^^^^^^^ expected an expression of type 'Ptr<T, Access.ReadWrite, AddressSpace.Device>', got 'Ptr<T, Access.ReadWrite, AddressSpace.GroupShared>'

    GroupSharedPtr<T> ptr2 = __getAddress(ptr[5]);

    return ptr2;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    int* ptr1 = __getAddress(bufferStorage[id.x]);
//diag:         ^^^^^^^^^^^^ invalid __getAddress usage
//diag:         ^^^^^^^^^^^^ '__getAddress' only supports groupshared variables and members of groupshared/device memory.

    int[100]* ptr2 = __getAddress(bufferGroupShared);
//diag:              ^^^^^^^^^^^^ expected an expression of type 'Ptr<int[100], Access.ReadWrite, AddressSpace.Device>', got 'Ptr<int[100], Access.ReadWrite, AddressSpace.GroupShared>'

    int* ptr3 = __getAddress(bufferGroupShared[id.x]);
//diag:         ^^^^^^^^^^^^ expected an expression of type 'Ptr<int, Access.ReadWrite, AddressSpace.Device>', got 'Ptr<int, Access.ReadWrite, AddressSpace.GroupShared>'

    int* ptr4 = __getAddress(bufferUserPointer[id.x]);

    GroupSharedPtr<int[100]> ptr5 = __getAddress(bufferGroupShared);

    GroupSharedPtr<int> ptr6 = __getAddress(bufferGroupShared[id.x]);

    GroupSharedPtr<int> ptr7 = paramGroupShared(bufferGroupShared);

    int* ptr8 = __getAddress(bufferUserPointerStruct.data1);

    StructPtrInStruct structPtrInStruct;
    structPtrInStruct.ptr = bufferUserPointerStruct;
    int* ptr9 = __getAddress(structPtrInStruct.ptr[id.x].data1);

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
