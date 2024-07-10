//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly -O0

// Test that we can pass arguments in different address space to an `inout` parameter, and have
// the callee specialized to the address space of the argument.
// If successful, we should generate SPIRV that passes validation.

static int gArray0[2];
groupshared int gArray1[2];

// CHECK: %array = OpFunctionParameter %_ptr_Private__arr_int_int_2
// CHECK: %array_0 = OpFunctionParameter %_ptr_Workgroup__arr_int_int_2

void modify(inout int array[2])
{
    array[0] = 1;
    array[1] = 2;
}

void atomicOp(inout int array[2])
{
    InterlockedAdd(array[0], 1);
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    modify(gArray0);
    modify(gArray1);
    atomicOp(gArray1);
    output[0] = gArray0[0] + gArray1[1];
}
