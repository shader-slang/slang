//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute -Wno-120
#version 450

// A GLSL buffer block is device memory, which SPIR-V does not allow to hold a pointer in a logical
// storage class, so a block holding one is rejected like a structured buffer.
// `#version` selects GLSL input for this file, and `-Wno-120` silences the warning that it
// overrides the language implied by the `.slang` extension.

struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
};

layout(std430, binding = 0) buffer Block
//CHECK:                           ^^^^^ logical pointer in shared or external memory
//CHECK:                           ^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
{
    Tagged tagged;
    float result;
} block;

layout(local_size_x = 1) in;
void main()
{
    block.result = block.tagged.tag;
}
