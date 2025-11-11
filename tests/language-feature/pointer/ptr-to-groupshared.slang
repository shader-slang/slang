//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// Tests if we handle passing groupshared address-space pointers correctly to a function
// when that data-type needs legalization (Data -> Data_natural due to `lower-buffer-element-type`).
// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 0

struct Data
{
    int value1;
    int value2;
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
uniform int* outputBuffer;
groupshared Data shared;

void foo(Ptr<Data, Access::ReadWrite, AddressSpace::GroupShared> ptr)
{
    outputBuffer[0] = ptr.value1;
    outputBuffer[1] = ptr.value2;
}

[numthreads(3, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    spirv_asm { OpCapability VariablePointers; };
    spirv_asm { OpExtension "SPV_KHR_variable_pointers"; };

    shared = Data(1, 2);
    foo(__getAddress(shared));
}
