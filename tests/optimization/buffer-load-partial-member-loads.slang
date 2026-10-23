//TEST:SIMPLE(filecheck=INDEX): -target spirv -entry indexAfterStore -stage compute
//TEST:SIMPLE(filecheck=GUARD): -target spirv -entry guardedIndex -stage compute
//TEST:SIMPLE(filecheck=CONST): -target spirv -entry guardedConstantIndex -stage compute
//TEST:SIMPLE(filecheck=REPEAT): -target spirv -entry repeatedMember -stage compute

// A load of a composite through a pointer whose value is only taken apart is replaced by loads
// of the members that are used, all at the point of the original load. An element whose index
// is not known there, or that is read only under a condition with an index that is not a
// constant within bounds, must not be loaded there: these cases keep loading the whole array
// (before the aliasing store) and index the copy.

struct Table
{
    float values[1024];

    float getAtStored()
    {
        *gOther = 100;
        return values[*gOther];
    }

    float getChecked(int i)
    {
        float r = 0;
        *gOther = 100;
        if (i < 16)
            r = values[i];
        return r;
    }

    float getConstChecked(int i)
    {
        float r = 0;
        *gOther = 100;
        if (i < 16)
            r = values[3];
        return r;
    }

    float getSameElementTwice(int i)
    {
        float r;
        *gOther = 100;
        if (i < 16)
            r = values[5] + 1.0;
        else
            r = values[5] - 1.0;
        return r;
    }
}

uniform Table* gTable;
uniform int* gOther;
RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<int> indices;

// The index is read after the store, so it is not available where the table is loaded.
// INDEX-LABEL: %Table_getAtStored = OpFunction
// INDEX: OpLoad %{{[_a-zA-Z]*}}float{{[_a-z]*}}1024
// INDEX: OpStore %[[COPY:[0-9]+]] %{{[0-9]+}}
// INDEX: OpStore %{{[0-9]+}} %int_100
// INDEX: %[[I:[0-9]+]] = OpLoad %int
// INDEX: %[[A:[0-9]+]] = OpAccessChain %{{[_a-zA-Z0-9]+}} %[[COPY]]
// INDEX: OpAccessChain %_ptr_Function_float %[[A]] %[[I]]
// INDEX: OpFunctionEnd
[shader("compute")]
[numthreads(1, 1, 1)]
void indexAfterStore(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = gTable.getAtStored();
}

// The element is read only when `i < 16`, so it is not loaded before the check.
// GUARD-LABEL: %Table_getChecked = OpFunction
// GUARD: %[[I:[0-9]+]] = OpFunctionParameter %int
// GUARD: OpLoad %{{[_a-zA-Z]*}}float{{[_a-z]*}}1024
// GUARD: OpStore %[[COPY:[0-9]+]] %{{[0-9]+}}
// GUARD: OpStore %{{[0-9]+}} %int_100
// GUARD-NOT: OpLoad %float
// GUARD: OpBranchConditional
// GUARD: %[[A:[0-9]+]] = OpAccessChain %{{[_a-zA-Z0-9]+}} %[[COPY]]
// GUARD: OpAccessChain %_ptr_Function_float %[[A]] %[[I]]
// GUARD: OpFunctionEnd
[shader("compute")]
[numthreads(1, 1, 1)]
void guardedIndex(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = gTable.getChecked(indices[tid.x]);
}

// A constant index within the array is in bounds on every path, so the element is loaded before
// the store and the check, as the program would have read it there.
// CONST-LABEL: %Table_getConstChecked = OpFunction
// CONST-NOT: OpLoad %{{[_a-zA-Z]*}}float{{[_a-z]*}}1024
// CONST: %[[P:[0-9]+]] = OpAccessChain %_ptr_PhysicalStorageBuffer_float
// CONST: OpLoad %float %[[P]]
// CONST: OpStore %{{[0-9]+}} %int_100
// CONST-NOT: OpAccessChain %_ptr_PhysicalStorageBuffer_float
// CONST: OpFunctionEnd
[shader("compute")]
[numthreads(1, 1, 1)]
void guardedConstantIndex(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = gTable.getConstChecked(indices[tid.x]);
}

// An element read in both branches is loaded once, before the store, and the rest of the array is
// not loaded.
// REPEAT-LABEL: %Table_getSameElementTwice = OpFunction
// REPEAT-NOT: OpLoad %{{[_a-zA-Z]*}}float{{[_a-z]*}}1024
// REPEAT: %[[P:[0-9]+]] = OpAccessChain %_ptr_PhysicalStorageBuffer_float
// REPEAT: OpLoad %float %[[P]]
// REPEAT-NOT: OpLoad %float %[[P]]
// REPEAT: OpStore %{{[0-9]+}} %int_100
// REPEAT-NOT: OpAccessChain %_ptr_PhysicalStorageBuffer_float
// REPEAT: OpFunctionEnd
[shader("compute")]
[numthreads(1, 1, 1)]
void repeatedMember(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = gTable.getSameElementTwice(indices[tid.x]);
}
