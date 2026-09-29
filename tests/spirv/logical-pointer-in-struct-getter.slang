//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O1
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly -Xslang -O1

// A struct holding a logical-storage-class pointer is read through a `get` accessor, which takes
// `this` by value. Type legalization splits the pointer out of the struct, so wherever the
// by-value `this` still exists it is a plain pointer, and no composite carrying the pointer is
// built, with or without the SPIR-V optimizer.
// https://github.com/shader-slang/slang/issues/13206

struct View<let A : AddressSpace>
{
    Ptr<float, Access.ReadWrite, A> data;
    __subscript(int i) -> float
    {
        get { return data[i]; }
        [nonmutating] set { data[i] = newValue; }
    }
}

groupshared float gsA[4];

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// CHECK: OpEntryPoint
// CHECK-NOT: OpTypeStruct {{.*}}%_ptr_Workgroup
// CHECK-NOT: OpCompositeExtract %_ptr_Workgroup

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 gtid : SV_GroupThreadID)
{
    View<AddressSpace.GroupShared> a;
    a.data = __getAddress(gsA[0]);
    a[gtid.x] = float(gtid.x + 1);
    GroupMemoryBarrierWithGroupSync();
    // BUF: 1
    // BUF: 2
    outputBuffer[gtid.x] = a[gtid.x];
}
