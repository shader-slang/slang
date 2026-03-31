// Circular dependencies through nested structs are also detected.
//
// The circularity check traces through struct fields recursively. If a
// conforming type contains a struct that contains the interface type, this
// creates the same cycle as a direct interface field.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

[anyValueSize(64)]
interface INode
{
    float get();
}

struct InnerData
{
    INode node;
}

// CHECK: error[E41003]
// CHECK: OuterNode
// CHECK: circular reference through interface
// CHECK: INode
struct OuterNode : INode
{
    InnerData inner;
    float get() { return 0; }
}

INode createNode()
{
    OuterNode o;
    return o;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    INode n = createNode();
    outputBuffer[0] = n.get();
}
