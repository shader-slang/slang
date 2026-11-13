// A self-referential struct reached only through the parameter of an exported function, `Node* p`,
// emits on every source target (shader-slang/slang#13396). No global and no entry point refers to
// `Node`, and nothing reads `next`, so the walk enters the cycle at the parameter's pointer.

//TEST:SIMPLE(filecheck=CLIKE): -target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target hlsl -profile lib_6_6 -entry main -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target cpp -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute

struct Node
{
    float value;
    Node* next;
};

export float readNode(Node* p)
{
    return p.value;
}

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = 1.0;
}

// CLIKE: struct [[NODE:Node_[0-9]+]];
// CLIKE: struct [[NODE]]
// CLIKE: readNode

// GLSL: layout(buffer_reference) buffer [[PTR:BufferPointer_Node_[0-9_]+]];
// GLSL: [[PTR]] next_0;
// GLSL: readNode

// WGSL: struct Node_0{{$}}
// WGSL: readNode
