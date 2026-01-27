//TEST:SIMPLE(filecheck=CHECK): -target spirv

// There was a bug in lowerEnumTypes where if there is already a:
// %lit0 = OpIntLit : %MyEnum (1)

// Then when lowering enum types from %MyEnum to %int, we would just replace %lit10's type with
// %int, resulting:
// %lit0 = OpIntLit : %int (1)
// without checking if there is already an OpIntLit of type %int with value 0.
// and without inserting the new value to the IRModule's constant deduplication map.
// This can result in creating duplicated array types later, because we now have two
// int lit insts that represent the same value!

// This test checks for any regression of this bug by ensuring that the generated SPIR-V
// only has one OpTypeArray instruction, which indicate the deduplication is handled correctly
// across enum-lowering.

// CHECK: OpEntryPoint
// CHECK-COUNT-1: OpTypeArray

enum class MyEnum
{
    ValueA = 0,
    ValueB = 1,
    ValueC = 2
}

struct MyStruct<MyEnum e, int rank>
{
    int data[rank-1];
}

uniform int* output;

[numthreads(1,1,1)]
void computeMain()
{
    MyStruct<MyEnum.ValueB, 2> myStructInstance;
    output[0] = myStructInstance.data[0];
}
