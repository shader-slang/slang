// Pointer types break dependency cycles for dynamic dispatch.
//
// A struct conforming to an interface may contain a pointer to that interface
// without creating a circular dependency, because pointers have fixed size and
// do not embed the pointee inline.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

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

INode createNode()
{
    LeafNode leaf;
    leaf.value = 42.0;
    return leaf;
}

RWStructuredBuffer<float> outputBuffer;

// CHECK-NOT: error[E
[numthreads(1, 1, 1)]
void computeMain()
{
    INode n = createNode();
    outputBuffer[0] = n.get();
}
