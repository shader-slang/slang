// Auto-inference of AnyValue size with a mix of self-referential and
// non-self-referential implementations.
//
// The inference algorithm in inferAnyValueSizeWhereNecessary() has two passes:
// 1. Compute max size from non-self-referential implementations (base case).
// 2. Set the AnyValue size, then compute sizes of self-referential
//    implementations that contain INode* (pointer to interface).
//
// LeafNode and DataNode are non-self-referential (they set the base size),
// while LinkedNode is self-referential (it contains INode*). This extends
// coverage beyond interface-ptr-in-impl.slang which only has two implementations.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// No [anyValueSize] -- compiler infers using two-pass algorithm.
interface INode
{
    float getValue();
}

// Non-self-referential: 4 bytes (1 float)
struct LeafNode : INode
{
    float value;
    float getValue() { return value; }
}

// Non-self-referential: 12 bytes (3 floats) -- drives the base size
struct DataNode : INode
{
    float x;
    float y;
    float z;
    float getValue() { return x + y + z; }
}

// Self-referential: contains INode* (pointer to interface).
// The pointer has fixed size (8 bytes), but its AnyValue storage is computed
// from the base size set by LeafNode/DataNode in the first pass.
struct LinkedNode : INode
{
    float value;
    INode* next;
    float getValue() { return value; }
}

float getNodeValue(uint id)
{
    INode n;
    if (id == 0)
    {
        LeafNode leaf;
        leaf.value = 10.0;
        n = leaf;
    }
    else if (id == 1)
    {
        DataNode data;
        data.x = 1.0;
        data.y = 2.0;
        data.z = 3.0;
        n = data;
    }
    else
    {
        LinkedNode linked;
        linked.value = 42.0;
        linked.next = nullptr;
        n = linked;
    }

    return n.getValue();
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 3 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // LeafNode: value = 10.0
    outputBuffer[0] = getNodeValue(0);
    // CHECK: 10.0

    // DataNode: x + y + z = 1 + 2 + 3 = 6.0
    outputBuffer[1] = getNodeValue(1);
    // CHECK: 6.0

    // LinkedNode: value = 42.0 (next is nullptr, not dereferenced)
    outputBuffer[2] = getNodeValue(2);
    // CHECK: 42.0
}
