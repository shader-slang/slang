//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute

// Regression test for #13948. A struct holding pointers is copied into a local
// from a buffer, and a pointer field of that local copy is passed to a function.
// The pointee types need a storage layout type (an array field, a bool field),
// so the call is specialized to take the storage-typed pointer. The cases reach
// the pointer field directly (FieldExtract), through a nested struct, through an
// array element (GetElement), from a `StructuredBuffer` copy, from a copy made
// inside a specialized callee, and with the pointee passed by value.
// `r.count + s.count` reads a non-pointer field, and `readPlane(viewA, 0)` calls
// the same function on a global pointer, without a local copy.
//
// The SIMPLE line needs no GPU: it fails if the pass asserts in
// `emitFieldAddress`, or, with SLANG_RUN_SPIRV_VALIDATION=1, if the SPIR-V is
// invalid.
//
// Each buffer name in `data=[...]` fills two 32-bit slots with its device
// address. `Root` holds view (offset 0), flag (8), holder.view (16), views[0]
// (24), views[1] (32) and count (40), padded to 48 bytes.

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

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4)
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

// The local copy is made inside a callee that is itself specialized to take
// the storage-typed `Root` pointer.
float readPlaneThroughRoot(Root* rp)
{
    Root rr = *rp;
    return readPlane(rr.holder.view, 2);
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
    // CHECK-NEXT: {{^}}6.0
    outputBuffer[4] = readPlane(s.holder.view, 1);
    // CHECK-NEXT: {{^}}7.0
    outputBuffer[5] = readPlaneByValue(*r.view, 1);
    // CHECK-NEXT: {{^}}8.0
    outputBuffer[6] = float(r.count + s.count);
    // CHECK-NEXT: {{^}}2.0
    outputBuffer[7] = readPlane(viewA, 0);
    // CHECK-NEXT: {{^}}22.0
    outputBuffer[8] = readPlaneThroughRoot(root);
}
