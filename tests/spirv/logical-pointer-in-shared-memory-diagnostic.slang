//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

// SPIR-V only allows a pointer in a logical storage class in a function-local or private
// variable. Type legalization splits such pointers out of local aggregates, but memory that is
// shared across invocations or supplied from outside the shader cannot be split, so a logical
// pointer stored there is rejected.

struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

StructuredBuffer<Tagged> sb;
//CHECK:                 ^^ logical pointer in shared or external memory
//CHECK:                 ^^ this memory holds a pointer to a logical address space, which SPIR-V only allows in function-local or private variables

ConstantBuffer<Tagged> cb;
//CHECK:               ^^ logical pointer in shared or external memory
//CHECK:               ^^ this memory holds a pointer to a logical address space, which SPIR-V only allows in function-local or private variables

groupshared Tagged gt;
//CHECK:           ^^ logical pointer in shared or external memory
//CHECK:           ^^ this memory holds a pointer to a logical address space, which SPIR-V only allows in function-local or private variables

// The pointee of a physical pointer lives in device memory, which is not function-local either.
uniform Ptr<Tagged> pt;

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    gt.tag = sb[0].tag + cb.tag + pt.tag;
//CHECK:                          ^^ logical pointer in shared or external memory
//CHECK:                          ^^ this memory holds a pointer to a logical address space, which SPIR-V only allows in function-local or private variables
    outputBuffer[0] = gt.tag;
}
