//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O1
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly -Xslang -O1

// Regression test for https://github.com/shader-slang/slang/issues/13206.
//
// A struct with a field that is a pointer in a logical SPIR-V storage class -- here
// `Ptr<float, Access.ReadWrite, GroupShared>`, which lowers to a `Workgroup` pointer -- is
// constructed as a local and written through its by-value subscript. The pointer field must be
// read through an address (`OpAccessChain`/`OpLoad`); loading the whole struct and extracting the
// pointer with `OpCompositeExtract`, or building the struct with `OpCompositeConstruct`, is invalid
// under logical addressing.
//
// The check runs at -O1 so the downstream SPIRV-Tools optimizer runs (slang-test compiles at -O0
// unless a level is given): the pointer must stay address-based through optimization, not be
// reconstructed into a composite.

struct View<let A : AddressSpace>
{
    Ptr<float, Access.ReadWrite, A> data;
    __subscript(int i) -> float
    {
        get { return data[i]; }
        [nonmutating] set { data[i] = newValue; }
    }
}

// A nested aggregate exercises projection-sinking through more than one level.
struct Outer
{
    View<AddressSpace.GroupShared> view;
}

// A struct with an ordinary (non-pointer) field alongside the pointer. Reading `tag` keeps the
// whole-aggregate load live unless every field read -- not just the pointer-typed one -- is made
// address-based; this is the case the "rewrite every read" design exists for.
struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

groupshared float gsA[64];

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// The logical pointer is read through an address, so it never becomes an operand of
// OpCompositeConstruct nor the result of OpCompositeExtract, to end of module. These CHECK-NOT
// directives are the structural regression guard. The `-vk` COMPARE_COMPUTE additionally checks the
// runtime result; it is not itself a structural check, since a permissive driver could accept
// invalid-composite SPIR-V and still return the right value.
// CHECK: OpEntryPoint
// CHECK-NOT: OpCompositeExtract
// CHECK-NOT: OpCompositeConstruct

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    View<AddressSpace.GroupShared> a;
    a.data = __getAddress(gsA[0]);
    a[0] = 1.0f;
    // The value is read back from the groupshared array directly, verifying the write went through
    // the pointer without materializing the pointer-bearing struct as a value.
    // BUF: 1
    outputBuffer[0] = gsA[0];

    Outer o;
    o.view.data = __getAddress(gsA[1]);
    o.view[0] = 2.0f;
    // BUF: 2
    outputBuffer[1] = gsA[1];

    // Read both the pointer field and the ordinary `tag` field of the same value: with only the
    // pointer read made address-based the whole-`Tagged` load would stay live for `tag` and the
    // optimizer could rebuild it as an OpCompositeConstruct, so the checks above exercise the
    // every-field rewrite here.
    Tagged t;
    t.data = __getAddress(gsA[2]);
    t.tag = 5.0f;
    t.data[0] = t.tag;
    // BUF: 5
    outputBuffer[2] = gsA[2];
}
