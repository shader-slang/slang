//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-via-glsl

// SPIR-V only allows a pointer in a logical storage class in a function-local or private
// variable. Type legalization splits such pointers out of local aggregates, but memory that is
// shared across invocations or supplied from outside the shader cannot be split, so a logical
// pointer stored there is rejected.

struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

struct Outer
{
    Tagged inner;
}

StructuredBuffer<Tagged> sb;
//CHECK:                 ^^ logical pointer in shared or external memory
//CHECK:                 ^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

RWStructuredBuffer<Tagged> rwsb;
//CHECK:                   ^^^^ logical pointer in shared or external memory
//CHECK:                   ^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

// A buffer type with no declaration of its own, like the element type of this array, is
// reported at a use.
StructuredBuffer<Outer> sbs[2];

ConstantBuffer<Tagged> cb;
//CHECK:               ^^ logical pointer in shared or external memory
//CHECK:               ^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

ConstantBuffer<Outer> cbo;
//CHECK:              ^^^ logical pointer in shared or external memory
//CHECK:              ^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

ParameterBlock<Tagged> pb;
//CHECK:               ^^ logical pointer in shared or external memory
//CHECK:               ^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

groupshared Tagged gt;
//CHECK:           ^^ logical pointer in shared or external memory
//CHECK:           ^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

groupshared Tagged gts[2];
//CHECK:           ^^^ logical pointer in shared or external memory
//CHECK:           ^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

// Each groupshared variable is reported, even when another has the same type.
groupshared Tagged gt2;
//CHECK:           ^^^ logical pointer in shared or external memory
//CHECK:           ^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

// A `__global` variable is one variable shared by all invocations, with or without an initializer.
__global Tagged actualGlobal;
//CHECK:        ^^^^^^^^^^^^ logical pointer in shared or external memory
//CHECK:        ^^^^^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
__global Tagged initializedActualGlobal = {};
//CHECK:        ^^^^^^^^^^^^^^^^^^^^^^^ logical pointer in shared or external memory
//CHECK:        ^^^^^^^^^^^^^^^^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

// The pointee of a physical pointer lives in device memory, which is not function-local either.
uniform Ptr<Tagged> pt;
//CHECK:            ^^ logical pointer in shared or external memory
//CHECK:            ^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

// Global and entry-point `uniform` parameters are collected into implicit parameter groups.
uniform Tagged ut;
//CHECK:       ^^ logical pointer in shared or external memory
//CHECK:       ^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

// The pointee of a groupshared pointer is groupshared memory, so a self-referential list node
// cannot live there even when the nodes themselves are local.
struct Node
/*CHECK:
       ^^^^ logical pointer in shared or external memory
       ^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
*/
{
    Ptr<Node, Access.ReadWrite, AddressSpace.GroupShared> next;
    float value;
}

// A logical pointer held directly, rather than in a struct field, is rejected the same way.
typealias GroupSharedFloatPtr = Ptr<float, Access.ReadWrite, AddressSpace.GroupShared>;
StructuredBuffer<GroupSharedFloatPtr> sbDirect;
//CHECK:                              ^^^^^^^^ logical pointer in shared or external memory
//CHECK:                              ^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
ConstantBuffer<GroupSharedFloatPtr> cbDirect;
//CHECK:                            ^^^^^^^^ logical pointer in shared or external memory
//CHECK:                            ^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
uniform GroupSharedFloatPtr uniformDirect;
//CHECK:                    ^^^^^^^^^^^^^ logical pointer in shared or external memory
//CHECK:                    ^^^^^^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
groupshared GroupSharedFloatPtr gsDirect;
//CHECK:                        ^^^^^^^^ logical pointer in shared or external memory
//CHECK:                        ^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
uniform Ptr<GroupSharedFloatPtr> ptDirect;
//CHECK:                         ^^^^^^^^ logical pointer in shared or external memory
//CHECK:                         ^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

ByteAddressBuffer bab;

RWByteAddressBuffer rwbab;

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform Tagged u)
//CHECK:                        ^ logical pointer in shared or external memory
//CHECK:                        ^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
{
    Node node;
    node.value = 1;

    Tagged loaded = bab.Load<Tagged>(0);
//CHECK:                            ^ logical pointer in shared or external memory
//CHECK:                            ^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
    rwbab.Store<Tagged>(0, loaded);
//CHECK:               ^ logical pointer in shared or external memory
//CHECK:               ^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

    gt.tag = sb[0].tag + rwsb[0].tag + sbs[1][0].inner.tag + cb.tag + cbo.inner.tag + pb.tag + pt.tag;
//CHECK:                                  ^ logical pointer in shared or external memory
//CHECK:                                  ^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
    gt2.tag = ut.tag + u.tag;
    actualGlobal.tag = gt2.tag;
    initializedActualGlobal.tag = initializedActualGlobal.tag + 1;
    gts[1].tag = loaded.tag + node.value;
    gsDirect = uniformDirect;
    gt2.tag += sbDirect[0][0] + cbDirect[0] + (*ptDirect)[0] + gsDirect[0];

    // A pointer to an element of a groupshared array is a pointer into groupshared memory that
    // holds a logical pointer, so it is reported as well.
    outputBuffer[0] = gt.tag + gts[1].tag + gt2.tag;
//CHECK:                          ^ logical pointer in shared or external memory
//CHECK:                          ^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
}
