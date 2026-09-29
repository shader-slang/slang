//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0

// A `static` global lives in private memory, where SPIR-V allows a logical pointer, so a `static`
// struct holding a logical-storage-class pointer is split by type legalization rather than
// rejected as memory shared across invocations.

struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

groupshared float gs[2];
static Tagged st;

RWStructuredBuffer<float> outputBuffer;

// CHECK-NOT: error[E
// CHECK: OpEntryPoint
// CHECK-NOT: OpTypeStruct {{.*}}%_ptr_Workgroup
// CHECK: %Tagged = OpTypeStruct %float
// CHECK: %st = OpVariable %_ptr_Private_Tagged Private

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    st.data = __getAddress(gs[0]);
    st.tag = 2.0f;
    st.data[0] = st.tag;
    outputBuffer[0] = gs[0];
}
