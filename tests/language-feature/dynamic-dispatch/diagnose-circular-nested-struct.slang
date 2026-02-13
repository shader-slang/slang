// Circular dependencies through nested structs are also detected.
//
// The circularity check must trace through struct fields recursively. If a
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
    INode node;  // Contains the interface
}

// CHECK: error {{[0-9]+}}
struct OuterNode : INode
{
    InnerData inner;  // Cycle via nested struct: OuterNode -> InnerData -> INode
    float get() { return 0; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    INode n = createDynamicObject<INode>(0, 5);
    outputBuffer[0] = n.get();
}
