// Optional<IInterface> does not break circular dependencies for dynamic dispatch.
//
// Unlike pointers, Optional<T> contains the value T directly (with a validity
// flag). When T is an interface type, packing the Optional still requires
// packing the interface value, creating the same circular dependency as a
// direct interface field.
//
// The early circular conformance detection sees through Optional to find
// the INode dependency, so this now correctly produces E41003.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target glsl
//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK): -target cpp
//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK): -target wgsl

[anyValueSize(64)]
interface INode
{
    float getValue();
}

struct LeafNode : INode
{
    float value;
    float getValue() { return value; }
}

// CHECK: error[E41003]
// CHECK: LinkedNode
// CHECK: circular reference through interface
struct LinkedNode : INode
{
    float value;
    Optional<INode> next;
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
        // We construct a scenario with a true loop, because otherwise, the compiler
        // might be able to optimize away the dynamic dispatch and thus not trigger the
        // packing error.
        //

        LeafNode leaf;
        leaf.value = 42.0;

        LinkedNode link1;
        link1.value = 100.0;
        link1.next = {leaf};

        LinkedNode link2;
        link2.value = 100.0;
        link2.next = {link1};

        return link2;  // Dynamic dispatch triggers packing error
    }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    INode node = makeNode(0);
    outputBuffer[0] = node.getValue();
}
