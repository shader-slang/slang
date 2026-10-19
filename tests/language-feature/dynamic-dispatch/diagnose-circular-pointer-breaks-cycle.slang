// Pointer types break dependency cycles for dynamic dispatch.
//
// A struct conforming to an interface may contain a pointer to that interface
// without creating a circular dependency, because pointers have fixed size and
// do not embed the pointee inline.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=NOPTR): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target glsl
//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK): -target cpp
//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=NOPTR): -target wgsl

[anyValueSize(16)]
interface INode
{
    float get();
}

struct PtrNode : INode
{
    INode* child;
    float get() { return 0; }
}

struct LeafNode : INode
{
    float value;
    float get() { return value; }
}

INode createPtrNode()
{
    PtrNode p;
    return p;
}

INode createLeaf()
{
    LeafNode leaf;
    leaf.value = 42.0;
    return leaf;
}

RWStructuredBuffer<float> outputBuffer;

// CHECK-NOT: error[E

// HLSL and WGSL have no pointers to memory addresses, so the pointer field is reported there
// instead of being emitted. That check runs after the circularity check, which passes.
// NOPTR-NOT: error[E41003]
// NOPTR: error[E55219]
[numthreads(1, 1, 1)]
void computeMain()
{
    INode n = createPtrNode();
    INode m = createLeaf();
    outputBuffer[0] = n.get() + m.get();
}
