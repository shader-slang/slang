// GPU-side any-value pack of a pointer field, then unpack and dispatch.
//
// buffer-address-ref.slang only unpacks a host-written IValue* (uint2 -> pointer).
// This test constructs IndirectNode on the device, stores it as INode into a
// buffer (pointer -> uint2 pack), loads it back (uint2 -> pointer unpack), and
// dispatches. A truncated pack would drop the high 32 bits of the device
// address; on Metal those bits are typically nonzero, so evaluate() would
// fail to load Literal.val.
//
// LeafNode is stored in the same buffer so the load site cannot assume a
// single concrete type.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=MTL):-target metal -stage compute -entry computeMain -conformance "Literal:IValue=1" -conformance "LeafNode:INode=1" -conformance "IndirectNode:INode=2"

[anyValueSize(8)]
interface IValue
{
    int get();
}

struct Literal : IValue
{
    int val;
    int get() { return val; }
}

[anyValueSize(16)]
interface INode
{
    int evaluate();
}

struct LeafNode : INode
{
    int value;
    int evaluate() { return value; }
}

struct IndirectNode : INode
{
    IValue* source;
    int offset;
    int evaluate() { return source.get() + offset; }
}

//TEST_INPUT: type_conformance Literal:IValue = 1
//TEST_INPUT: type_conformance LeafNode:INode = 1
//TEST_INPUT: type_conformance IndirectNode:INode = 2

// Serialized Literal existential: RTTI(uint2) + Witness(uint2) + AnyValue[val=10, pad]
//TEST_INPUT: set valueBuf = ubuffer(data=[0 0 1 0 10 0], stride=4)
uniform IValue* valueBuf;

// Two INode existentials: RTTI(8) + Witness(8) + AnyValue(16) = 8 uints each.
//TEST_INPUT: set nodes = ubuffer(data=[0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<INode> nodes;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IndirectNode indirect;
    indirect.source = valueBuf;
    indirect.offset = 5;
    nodes[0] = indirect;

    LeafNode leaf;
    leaf.value = 99;
    nodes[1] = leaf;

    // Packed IndirectNode: Literal.get() + offset = 10 + 5
    // CHECK: 15
    outputBuffer[0] = nodes[0].evaluate();

    // Packed LeafNode, different witness id
    // CHECK: 99
    outputBuffer[1] = nodes[1].evaluate();

    // Reload IndirectNode after the LeafNode store so neither load is
    // a forwarding of a local.
    // CHECK: 15
    outputBuffer[2] = nodes[0].evaluate();

    // CHECK: 10
    outputBuffer[3] = valueBuf.get();
}

// Pack: pointer -> ulong -> uint2, then both lanes into the any-value payload.
// MTL: as_type<uint2>((ulong)(
// Unpack: both uint2 lanes -> ulong -> device pointer.
// MTL: as_type<ulong>(uint2(
// Direct vector-to-pointer casts are illegal in MSL.
// MTL-NOT: as_type<{{.*}}device{{.*}}*>
