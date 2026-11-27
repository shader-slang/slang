//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

// An identity cast preserves the operand's storage and its address space.
groupshared int sharedValue;

void consume(groupshared int value)
{
}

[numthreads(1, 1, 1)]
void computeMain()
{
    consume(int(sharedValue));
    Ptr<int, Access::ReadWrite, AddressSpace::GroupShared> pointer =
        __getAddress(int(sharedValue));

    // The cast must not make a group-shared address convertible to a device pointer.
    int* invalidPointer = __getAddress(int(sharedValue));
//CHECK: type mismatch in expression
//CHECK: expected an expression of type 'Ptr<int, Access.ReadWrite, AddressSpace.Device, DefaultDataLayout>', got 'Ptr<int, Access.ReadWrite, AddressSpace.GroupShared, DefaultDataLayout>'
}
