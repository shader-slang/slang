//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -g0

// By default slang-test uses `-g` and it requires `VariablePointers`, which
// doesn't produce the correct result due to the bug on the graphics driver.
// Tracked by github issue #9061
// Used GroupShared pointer paths now correctly require `VariablePointers` even with `-g0`,
// so keep the Vulkan runtime path disabled and check the generated SPIR-V instead.
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -g0
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// Tests if we handle passing groupshared address-space pointers correctly to a function
// when that data-type needs legalization (Data -> Data_natural due to `lower-buffer-element-type`).
// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 0
// SPIRV: OpCapability VariablePointers
// SPIRV: OpExtension "SPV_KHR_variable_pointers"
// SPIRV: %Data = OpTypeStruct %int %int
// SPIRV: %_ptr_Workgroup_Data = OpTypePointer Workgroup %Data
// SPIRV: %shared = OpVariable %_ptr_Workgroup_Data Workgroup
// SPIRV: {{%[0-9]+}} = OpAccessChain %_ptr_Workgroup_int %shared %int_0
// SPIRV-NEXT: {{%[0-9]+}} = OpLoad %int
// SPIRV: {{%[0-9]+}} = OpAccessChain %_ptr_Workgroup_int %shared %int_1
// SPIRV-NEXT: {{%[0-9]+}} = OpLoad %int

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
    shared = Data(1, 2);
    foo(__getAddress(shared));
}
