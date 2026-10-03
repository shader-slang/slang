//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry vertMain -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry vertMain -stage vertex -emit-spirv-via-glsl

// The varying inputs and outputs of an entry point live in the pipeline interface, which SPIR-V
// does not allow to hold a pointer in a logical storage class, so they are rejected like other
// memory supplied from or returned to outside the shader.

struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

struct VIn
{
    float4 position : POSITION;
    Tagged tagged : TEXCOORD0;
}

struct VOut
{
    float4 position : SV_Position;
    Tagged tagged : TEXCOORD0;
}

// GLSL-style global varyings are part of the interface too.
layout(location = 4) in Tagged globalInput;
//CHECK:                       ^^^^^^^^^^^ logical pointer in shared or external memory
//CHECK:                       ^^^^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
layout(location = 4) out Tagged globalOutput;
//CHECK:                        ^^^^^^^^^^^^ logical pointer in shared or external memory
//CHECK:                        ^^^^^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables

[shader("vertex")]
VOut vertMain(VIn input, out Tagged extra : TEXCOORD1)
/*CHECK:
     ^^^^^^^^ logical pointer in shared or external memory
     ^^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
                  ^^^^^ logical pointer in shared or external memory
                  ^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
                                    ^^^^^ logical pointer in shared or external memory
                                    ^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
*/
{
    VOut output;
    output.position = input.position;
    output.tagged.tag = input.tagged.tag;
    extra.tag = 1;
    globalOutput.tag = globalInput.tag;
    return output;
}
