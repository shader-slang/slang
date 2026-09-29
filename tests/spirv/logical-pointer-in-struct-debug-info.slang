//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -g -O0
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -g -O1
//TEST:SIMPLE(filecheck=DBG):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -g -O0
//TEST:SIMPLE(filecheck=DBGT):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -g -O0

// With debug info, every local variable gets a debug variable whose type is legalized along with
// the variable. When type legalization splits a struct holding a logical-storage-class pointer,
// the debug variable and the address projections that feed its debug values must be split the
// same way. A split variable keeps one debug variable per split-out part, named after its field
// path, when every part can be described, as for `views`, `o` and `two`. The ordinary half of a
// struct that also has ordinary fields, like `Tagged`, cannot be described, so `t` has no debug
// variable.

struct View
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
}

struct Outer
{
    View view;
}

struct Two
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> a;
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> b;
}

struct Tagged
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

groupshared float gs[6];

RWStructuredBuffer<float> outputBuffer;

// The source text is embedded in the debug info, so these patterns are written not to match
// themselves.
// CHECK: OpEntryPoint
// CHECK-NOT: = OpCompositeExtract %{{[_a-zA-Z0-9]*}}_ptr_Workgroup
// CHECK-NOT: = OpCompositeConstruct %{{View|Tagged|Outer}}{{ }}

// DBG-DAG: %[[VIEWS:[0-9]+]] = OpString "views.data"
// DBG-DAG: %[[O:[0-9]+]] = OpString "o.view.data"
// DBG-DAG: %[[TWOA:[0-9]+]] = OpString "two.a"
// DBG-DAG: %[[TWOB:[0-9]+]] = OpString "two.b"
// DBG-DAG: DebugLocalVariable %[[VIEWS]] {{.*}}
// DBG-DAG: DebugLocalVariable %[[O]] {{.*}}
// DBG-DAG: DebugLocalVariable %[[TWOA]] {{.*}}
// DBG-DAG: DebugLocalVariable %[[TWOB]] {{.*}}

// DBGT-NOT: OpString "t"

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

    Two two;
    two.a = __getAddress(gs[4]);
    two.b = __getAddress(gs[5]);
    two.a[0] = two.b[0];

    Tagged t;
    t.data = __getAddress(gs[3]);
    t.tag = 3.0f;
    t.data[0] = t.tag;

    GroupMemoryBarrierWithGroupSync();
    outputBuffer[gtid.x] = gs[gtid.x];
}
