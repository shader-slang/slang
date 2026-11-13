//DIAGNOSTIC_TEST:SIMPLE(diag=NAME):-target spirv -entry nameOnly -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=NAME):-target spirv -entry nameOnly -stage compute -Wno-using-uninitialized-variable -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE):-target spirv -entry typeOnly -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE):-target spirv -entry typeOnly -stage compute -Wno-using-uninitialized-variable -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=FETCH):-target spirv -entry unannotatedFetch -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=FETCH):-target spirv -entry unannotatedFetch -stage compute -Wno-using-uninitialized-variable -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=COPY):-target spirv -entry unannotatedCopy -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=COPY):-target spirv -entry unannotatedCopy -stage compute -Wno-using-uninitialized-variable -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=SIBLING):-target spirv -entry unannotatedSibling -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=SIBLING):-target spirv -entry unannotatedSibling -stage compute -Wno-using-uninitialized-variable -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CONTROL,exit-code=0):-target spirv -entry definedSibling -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CONTROL,exit-code=0):-target spirv -entry definedSibling -stage compute -Wno-using-uninitialized-variable -O0

// Check that an unannotated asm block naming an undefined handle reports E56003,
// even if it accesses the resource. An annotated block using the handle only in a
// type operand, such as __imageType(missing), also reports E56003.
// The uninitialized-variable warning is suppressed so each case checks only that error.
Texture2D<float4> texture;
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void nameOnly()
{
    Texture2D<float4> missing;
    uint value = spirv_asm
//NAME:          ^^^^^^^^^ E56003
//NAME:          ^^^^^^^^^ use of uninitialized opaque handle 'Texture2D'.
    {
        OpName $missing "unusedHandle";
        result:$$uint = OpConstant 123;
    };
    outputBuffer[0] = float4(value);
}

[numthreads(1, 1, 1)]
void typeOnly()
{
    Texture2D<float4> missing;
    int2 coordinates = 0;
    int lod = 0;
    float4 value = spirv_asm [__assumeValidResourceOperands]
//TYPE:            ^^^^^^^^^ E56003
//TYPE:            ^^^^^^^^^ use of uninitialized opaque handle 'Texture2D'.
    {
        %unused = OpTypeSampledImage __imageType(missing);
        result:$$float4 = OpImageFetch $texture $coordinates Lod $lod;
    };
    outputBuffer[0] = value;
}

[numthreads(1, 1, 1)]
void unannotatedFetch()
{
    Texture2D<float4> missing;
    int2 coordinates = 0;
    int lod = 0;
    float4 value = spirv_asm
//FETCH:           ^^^^^^^^^ E56003
//FETCH:           ^^^^^^^^^ use of uninitialized opaque handle 'Texture2D'.
    {
        result:$$float4 = OpImageFetch $missing $coordinates Lod $lod;
    };
    outputBuffer[0] = value;
}

[numthreads(1, 1, 1)]
void unannotatedCopy()
{
    Texture2D<float4> missing;
    Texture2D<float4> copied = spirv_asm
//COPY:                        ^^^^^^^^^ E56003
//COPY:                        ^^^^^^^^^ use of uninitialized opaque handle 'Texture2D'.
    {
        result:$$Texture2D<float4> = OpCopyObject $missing;
    };
    outputBuffer[0] = copied.Load(int3(0));
}

// Check that annotating one block does not authorize removing its unannotated sibling,
// including when both blocks are cloned to specialize an undefined argument.
[noinline]
uint useSiblingAsmBlocks(Texture2D<float4> image)
{
    int2 coordinates = 0;
    int lod = 0;
    float4 value = spirv_asm [__assumeValidResourceOperands]
    {
        result:$$float4 = OpImageFetch $image $coordinates Lod $lod;
    };
    outputBuffer[0] = value;
    return spirv_asm
//SIBLING: ^^^^^^^^^ E56003
//SIBLING: ^^^^^^^^^ use of uninitialized opaque handle 'Texture2D'.
    {
        OpName $image "siblingHandle";
        result:$$uint = OpConstant 123;
    };
}

[numthreads(1, 1, 1)]
void unannotatedSibling()
{
    Texture2D<float4> missing;
    outputBuffer[1] = float4(useSiblingAsmBlocks(missing));
}

[numthreads(1, 1, 1)]
void definedSibling()
{
    outputBuffer[1] = float4(useSiblingAsmBlocks(texture));
}
