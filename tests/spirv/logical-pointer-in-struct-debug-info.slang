//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -g -O0
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -g -O1
//TEST:SIMPLE(filecheck=DBG):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -g -O0

// With debug info, every local variable gets a debug variable whose type is legalized along with
// the variable. When type legalization splits a struct holding a logical-storage-class pointer,
// the debug variable and the address projections that feed its debug values must be split the
// same way. A split variable keeps its debug variable when every part of it can be described,
// as for `views` and `o`.

struct View
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
}

struct Outer
{
    View view;
}

struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

groupshared float gs[4];

RWStructuredBuffer<float> outputBuffer;

// The source text is embedded in the debug info, so these patterns are written not to match
// themselves.
// CHECK: OpEntryPoint
// CHECK-NOT: = OpCompositeExtract %{{[_a-zA-Z0-9]*}}_ptr_Workgroup
// CHECK-NOT: = OpCompositeConstruct %{{View|Tagged|Outer}}{{ }}

// DBG-DAG: %views = OpExtInst %void %{{[0-9]+}} DebugLocalVariable
// DBG-DAG: %o = OpExtInst %void %{{[0-9]+}} DebugLocalVariable

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 gtid : SV_GroupThreadID)
{
    View views[2];
    views[0].data = __getAddress(gs[0]);
    views[1].data = __getAddress(gs[1]);
    views[gtid.x].data[0] = 1.0f;

    Outer o;
    o.view.data = __getAddress(gs[2]);
    o.view.data[0] = 2.0f;

    Tagged t;
    t.data = __getAddress(gs[3]);
    t.tag = 3.0f;
    t.data[0] = t.tag;

    GroupMemoryBarrierWithGroupSync();
    outputBuffer[gtid.x] = gs[gtid.x];
}
