// Test for slang-test infrastructure: data=[bufferName] syntax.
//
// Verifies that the data=[bufferName] syntax in TEST_INPUT correctly embeds
// a named buffer's device address into another buffer's data, enabling
// construction of serialized existentials with pointer fields.
//
// Uses two separate interfaces (IValue and INode) to avoid circularity:
// IndirectNode : INode has an IValue* field (different interface, no recursion).
//
// Scenario:
//   valueBuf  -> serialized Literal{val=10} existential for IValue
//   nodeBuf   -> serialized IndirectNode{source=&valueBuf, offset=5} for INode
//   nodeBuf.evaluate() dispatches IndirectNode::evaluate() which calls
//   source.get() (Literal::get() = 10) + offset (5) = 15

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type

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

// Serialized Literal existential for IValue.
// Layout: RTTI(uint2) + WitnessTableID(uint2) + AnyValue(8 bytes)
// AnyValue: [val=10, pad]
//TEST_INPUT: set valueBuf = ubuffer(data=[0 0 1 0 10 0], stride=4)
uniform IValue* valueBuf;

// Serialized IndirectNode existential for INode.
// Layout: RTTI(uint2) + WitnessTableID(uint2) + AnyValue(16 bytes)
// AnyValue: [source.lo, source.hi, offset=5, pad]
// 'valueBuf' in data=[] is replaced with the device address of valueBuf.
//TEST_INPUT: set nodeBuf = ubuffer(data=[0 0 2 0 valueBuf 5 0], stride=4)
uniform INode* nodeBuf;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Direct: Literal.get() = val = 10
    // CHECK: 10
    outputBuffer[0] = valueBuf.get();

    // Indirect via data=[bufferName]:
    // IndirectNode.evaluate() = source.get() + offset = 10 + 5 = 15
    // CHECK: 15
    outputBuffer[1] = nodeBuf.evaluate();

    // LeafNode for comparison (not using data=[bufferName], just static construction)
    LeafNode leaf;
    leaf.value = 99;
    INode leafInterface = leaf;
    // CHECK: 99
    outputBuffer[2] = leafInterface.evaluate();
}
