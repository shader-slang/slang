//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O1
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly -Xslang -O1

// Dynamic-index variant of https://github.com/shader-slang/slang/issues/13206.
//
// An array of structs carrying a logical-storage-class pointer is indexed by a runtime value
// (`SV_GroupThreadID`) rather than a constant. A constant index is a module-scope literal that is
// available everywhere, so it short-circuits the availability check in the pass; a runtime index
// exercises the dominance branch (`operandAvailableAt` -> `dominates`) that decides whether the
// element read can be projected to an address. The pointer must still be read through an address.

struct View
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
}

groupshared float gs[64];

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// CHECK: OpEntryPoint
// CHECK-NOT: OpCompositeExtract %{{.*}}_ptr_Workgroup
// CHECK-NOT: OpCompositeConstruct %View

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 gtid : SV_GroupThreadID)
{
    View views[2];
    views[0].data = __getAddress(gs[0]);
    views[1].data = __getAddress(gs[1]);
    views[gtid.x].data[0] = float(gtid.x + 1);
    GroupMemoryBarrierWithGroupSync();
    // BUF: 1
    // BUF: 2
    outputBuffer[gtid.x] = gs[gtid.x];
}
