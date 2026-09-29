//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O1
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly -Xslang -O1

// Array variant of https://github.com/shader-slang/slang/issues/13206.
//
// A local array of structs, each carrying a logical-storage-class pointer, is indexed by a
// constant and written through the pointer. Type legalization turns the array of structs into an
// array of the split-out pointers, and element-address access to it builds no composite that
// carries a pointer. The same holds for a struct field that is itself an array of pointers. The
// checks run at -O0 and at -O1, where the SPIRV-Tools optimizer runs.

struct View<let A : AddressSpace>
{
    Ptr<float, Access.ReadWrite, A> data;
    __subscript(int i) -> float
    {
        get { return data[i]; }
        [nonmutating] set { data[i] = newValue; }
    }
}

struct Pair
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> ptrs[2];
    float tag;
}

groupshared float gsA[64];
groupshared float gsB[64];

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// CHECK: OpEntryPoint
// CHECK-NOT: OpTypeStruct {{.*}}%_ptr_Workgroup
// CHECK-NOT: OpCompositeExtract %_ptr_Workgroup

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    View<AddressSpace.GroupShared> views[2];
    views[0].data = __getAddress(gsA[0]);
    views[1].data = __getAddress(gsB[0]);
    views[0][0] = 3.0f;
    views[1][0] = 4.0f;
    // Read back from groupshared directly, verifying each element's pointer addresses the right
    // array.
    // BUF: 3
    outputBuffer[0] = gsA[0];
    // BUF: 4
    outputBuffer[1] = gsB[0];

    Pair pair;
    pair.ptrs[0] = __getAddress(gsA[1]);
    pair.ptrs[1] = __getAddress(gsB[1]);
    pair.tag = 2.0f;
    pair.ptrs[0][0] = pair.tag;
    pair.ptrs[1][0] = pair.tag + 3.0f;
    // BUF: 7
    outputBuffer[2] = gsA[1] + gsB[1];
}
