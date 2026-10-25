//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute

// Regression test for #13948. A struct holding pointers is copied into a local
// from a buffer, and a pointer field of that local copy is passed to a function.
// The pointee types need a storage layout type (an array field, a bool field),
// so the call is specialized to take the storage-typed pointer. The pointer
// fields are reached directly, through a nested struct and through an array,
// and one pointee is passed by value.

// SPIRV: OpEntryPoint GLCompute

struct View
{
    float4 planes[3];
}

struct FlagView
{
    float4 value;
    bool enabled;
}

struct Holder
{
    View* view;
}

struct Root
{
    View* view;
    FlagView* flag;
    Holder holder;
    View* views[2];
    uint count;
}

//TEST_INPUT: set viewA = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0], stride=4)
uniform View* viewA;

//TEST_INPUT: set viewB = ubuffer(data=[13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0 24.0], stride=4)
uniform View* viewB;

//TEST_INPUT: set flagView = ubuffer(data=[100.0 200.0 300.0 400.0 1 0 0 0], stride=4)
uniform FlagView* flagView;

//TEST_INPUT: set root = ubuffer(data=[viewA flagView viewB viewA viewB 3 0], stride=4)
uniform Root* root;

//TEST_INPUT: set rootBuffer = ubuffer(data=[viewB flagView viewA viewB viewA 5 0], stride=48)
StructuredBuffer<Root> rootBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

float readPlane(View* v, uint i)
{
    return v.planes[i].y;
}

float readPlaneByValue(View v, uint i)
{
    return v.planes[i].z;
}

float readFlag(FlagView* f)
{
    return f.enabled ? f.value.z : -1.0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Root r = root[0];
    Root s = rootBuffer[0];

    // CHECK: type: float
    // CHECK-NEXT: {{^}}10.0
    outputBuffer[0] = readPlane(r.view, 2);
    // CHECK-NEXT: {{^}}300.0
    outputBuffer[1] = readFlag(r.flag);
    // CHECK-NEXT: {{^}}18.0
    outputBuffer[2] = readPlane(r.holder.view, 1);
    // CHECK-NEXT: {{^}}14.0
    outputBuffer[3] = readPlane(r.views[1], 0);
    // CHECK-NEXT: {{^}}18.0
    outputBuffer[4] = readPlane(s.view, 1);
    // CHECK-NEXT: {{^}}7.0
    outputBuffer[5] = readPlaneByValue(*r.view, 1);
    // CHECK-NEXT: {{^}}8.0
    outputBuffer[6] = float(r.count + s.count);
    // CHECK-NEXT: {{^}}2.0
    outputBuffer[7] = readPlane(viewA, 0);
}
