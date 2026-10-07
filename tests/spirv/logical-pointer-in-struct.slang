//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O1
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly -Xslang -O1

// Regression test for https://github.com/shader-slang/slang/issues/13206.
//
// A struct with a field that is a pointer in a logical SPIR-V storage class -- here
// `Ptr<float, Access.ReadWrite, GroupShared>`, which lowers to a `Workgroup` pointer -- is
// constructed as a local and written through its by-value subscript. Under logical addressing such
// a pointer may not be a member of a composite, so type legalization splits it out of the struct
// the same way it splits out resources, and no `OpCompositeExtract`/`OpCompositeConstruct` carries
// the pointer.
//
// The checks run at -O0, where the unsplit struct would be loaded and its pointer extracted, and
// at -O1, where the SPIRV-Tools optimizer would rebuild it as a composite. Metal allows a
// `threadgroup` pointer as a struct member, so there the struct is left intact.

struct View<let A : AddressSpace>
{
    Ptr<float, Access.ReadWrite, A> data;
    __subscript(int i) -> float
    {
        get { return data[i]; }
        [nonmutating] set { data[i] = newValue; }
    }
}

// A nested aggregate is split through more than one level.
struct Outer
{
    View<AddressSpace.GroupShared> view;
}

// A struct with an ordinary field alongside the pointer splits into an ordinary struct holding
// `tag` plus the pointer.
struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

groupshared float gsA[64];

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// No struct type has a `Workgroup` pointer member, so no composite value can carry one. These
// CHECK-NOT directives are the structural regression guard. The `-vk` COMPARE_COMPUTE
// additionally checks the runtime result; it is not itself a structural check, since a permissive
// driver could accept invalid-composite SPIR-V and still return the right value.
// CHECK: OpEntryPoint
// CHECK-NOT: OpTypeStruct {{.*}}%_ptr_Workgroup
// CHECK-NOT: OpCompositeExtract %_ptr_Workgroup

// METAL: struct View
// METAL-NEXT: {
// METAL-NEXT: float threadgroup* data

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    View<AddressSpace.GroupShared> a;
    a.data = __getAddress(gsA[0]);
    a[0] = 1.0f;
    // The value is read back from the groupshared array directly, verifying the write went through
    // the pointer.
    // BUF: 1
    outputBuffer[0] = gsA[0];

    Outer o;
    o.view.data = __getAddress(gsA[1]);
    o.view[0] = 2.0f;
    // BUF: 2
    outputBuffer[1] = gsA[1];

    // Both halves of the split `Tagged` are read.
    Tagged t;
    t.data = __getAddress(gsA[2]);
    t.tag = 5.0f;
    t.data[0] = t.tag;
    // BUF: 5
    outputBuffer[2] = gsA[2];
}
