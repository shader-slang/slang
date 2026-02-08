// Pointers to interface types break circular dependencies in dynamic dispatch.
//
// When a struct conforms to a dyn/anyValueSize interface, it must be packable
// into a fixed-size tagged union for dynamic dispatch. Self-referential structs
// containing the interface type directly would create infinite recursion during
// packing. However, pointers have a fixed size regardless of their target type,
// so using `INode*` instead of `INode` breaks the cycle and allows the struct
// to have a finite, packable size.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=output
RWStructuredBuffer<float> output;

[anyValueSize(32)]
interface INode
{
    float getValue();
}

struct LeafNode : INode
{
    float value;
    float getValue() { return value; }
}

struct TreeNode : INode
{
    float value;
    INode* left;   // Pointer breaks the cycle - fixed size regardless of target
    INode* right;
    float getValue() { return value; }
}

INode makeNode(uint id)
{
    if (id == 0)
    {
        LeafNode leaf;
        leaf.value = 42.0;
        return leaf;
    }
    else
    {
        TreeNode tree;
        tree.value = 100.0;
        tree.left = nullptr;
        tree.right = nullptr;
        return tree;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    INode node = makeNode(0);
    output[0] = node.getValue();
}

// CHECK: 42.0
