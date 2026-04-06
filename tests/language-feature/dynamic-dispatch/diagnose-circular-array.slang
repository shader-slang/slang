// Arrays of interface types do not break circular dependencies.
//
// Unlike pointers, arrays embed their elements directly. An array of interface
// values still requires packing each element, so INode[N] in a conforming type
// creates the same circular dependency as a single INode field.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target glsl
//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK): -target cpp
//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK): -target wgsl

[anyValueSize(128)]
interface INode
{
    float get();
}

// CHECK: error[E41003]
// CHECK: TreeNode
// CHECK: circular reference through interface
// CHECK: INode
struct TreeNode : INode
{
    INode children[2];
    float get() { return 0; }
}

INode createNode()
{
    TreeNode t;
    return t;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    INode n = createNode();
    outputBuffer[0] = n.get();
}
