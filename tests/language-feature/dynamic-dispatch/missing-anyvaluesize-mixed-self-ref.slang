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
//TEST:SIMPLE(filecheck=SIZE):-target spirv-asm -stage compute -entry computeMain -dump-ir-after inferAnyValueSizeWhereNecessary

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// No [anyValueSize] -- compiler infers using two-pass algorithm.
interface INode
{
    int getValue();
}

// Non-self-referential: 4 bytes (1 int)
struct LeafNode : INode
{
    int value;
    int getValue() { return value; }
}

// Non-self-referential: 12 bytes (3 ints) -- drives the base size
struct DataNode : INode
{
    int x;
    int y;
    int z;
    int getValue() { return x + y + z; }
}

// Self-referential: contains INode* (pointer to interface).
// Its AnyValue storage is computed in the second inference pass,
// after the base size is determined from LeafNode/DataNode.
struct LinkedNode : INode
{
    int value;
    INode* next;
    int getValue() { return value; }
}

INode createNode(uint id)
{
    INode n;
    if (id == 0)
    {
        LeafNode leaf;
        leaf.value = 10;
        n = leaf;
    }
    else if (id == 1)
    {
        DataNode data;
        data.x = 1;
        data.y = 2;
        data.z = 3;
        n = data;
    }
    else if (id == 2)
    {
        LinkedNode linked;
        linked.value = 42;
        linked.next = nullptr;
        n = linked;
    }
    else
    {
        LeafNode fallback;
        fallback.value = -1;
        n = fallback;
    }
    return n;
}

[numthreads(3, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    INode n = createNode(dispatchThreadID.x);
    outputBuffer[dispatchThreadID.x] = n.getValue();
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 3 possible types:
}
// Verify inferred AnyValueSize and struct sizes via IR dump.
// SIZE: AnyValueSize(16 : Int)
// SIZE: SizeAndAlignment({{.*}}, 4 : Int64, {{.*}})
// SIZE: struct %LeafNode
// SIZE: SizeAndAlignment({{.*}}, 12 : Int64, {{.*}})
// SIZE: struct %DataNode
// SIZE: SizeAndAlignment({{.*}}, 16 : Int64, {{.*}})
// SIZE: struct %LinkedNode
// LeafNode: value = 10
// DataNode: x+y+z = 1+2+3 = 6
// LinkedNode: value = 42 (next is nullptr, not dereferenced)
// CHECK: 10
// CHECK: 6
// CHECK: 42
