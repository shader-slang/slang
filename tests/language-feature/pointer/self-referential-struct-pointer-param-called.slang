// A self-referential struct whose only use is a function called from the entry point on a `Node*`
// emits on every source target (shader-slang/slang#13396); nothing reads `next`. The pointer comes
// from a uniform, and the call is guarded so the one-thread Vulkan run never dereferences it.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=VIA_GLSL): -target spirv -emit-spirv-via-glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target hlsl -profile lib_6_6 -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -output-using-type

struct Node
{
    float value;
    Node* next;
};

float readCalled(Node* p)
{
    return p.value;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name outBuf
RWStructuredBuffer<float> outBuf;

uniform Node* head;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = 1.0;
    if (tid.x == 1000)
        outBuf[tid.x] = readCalled(head);
}

// GLSL: layout(buffer_reference) buffer [[PTR:BufferPointer_Node_[0-9_]+]];
// GLSL: struct [[NODE:Node_[0-9]+]]
// GLSL: [[PTR]] next_0;

// VIA_GLSL: OpEntryPoint

// CLIKE: struct [[NODE:Node_[0-9]+]];
// CLIKE: struct [[NODE]]
// CLIKE-NEXT: {
// CLIKE: next_0;

// WGSL: struct Node_0{{$}}

// BUF: type: float
// BUF-NEXT: 1.0
