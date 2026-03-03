// Arrays of interface types do not break circular dependencies.
//
// Unlike pointers, arrays embed their elements directly. An array of interface
// values still requires packing each element, so INode[N] in a conforming type
// creates the same circular dependency as a single INode field.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

[anyValueSize(128)]
interface INode
{
    float get();
}

// CHECK: error {{[0-9]+}}
struct TreeNode : INode
{
    INode children[2];  // Array embeds values - does not break the cycle
    float get() { return 0; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    INode n = createDynamicObject<INode>(0, 5);
    outputBuffer[0] = n.get();
}
