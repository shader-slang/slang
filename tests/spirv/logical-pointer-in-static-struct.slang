//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O1
//TEST:SIMPLE(filecheck=LAYOUT):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// A `static` global lives in private memory, where SPIR-V allows a logical pointer, so a `static`
// struct holding a logical-storage-class pointer is split by type legalization rather than
// rejected as non-local memory. A `static` with an initializer keeps its initial value, and a later
// initializer that reads it still runs after it.

struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

groupshared float gs[2];
static Tagged st;

Tagged makeTagged()
{
    Tagged t;
    t.data = __getAddress(gs[1]);
    t.tag = 2.0f;
    return t;
}

static Tagged initialized = makeTagged();
static float afterInitialized = initialized.tag + 1.0f;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// CHECK-NOT: error[E
// CHECK-NOT: OpTypeStruct {{.*}}%_ptr_Workgroup
// CHECK-NOT: OpCompositeExtract %_ptr_Workgroup

// LAYOUT: %Tagged = OpTypeStruct %float
// LAYOUT: %st = OpVariable %_ptr_Private_Tagged Private

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    st.data = __getAddress(gs[0]);
    st.tag = 2.0f;
    st.data[0] = st.tag;
    initialized.data[0] = 3.0f;
    // BUF: 2
    outputBuffer[0] = gs[0];
    // BUF: 5
    outputBuffer[1] = initialized.tag + gs[1];
    // BUF: 3
    outputBuffer[2] = afterInitialized;
}
