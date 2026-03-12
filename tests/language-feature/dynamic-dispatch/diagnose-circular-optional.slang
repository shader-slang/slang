// Optional<IInterface> does not break circular dependencies for dynamic dispatch.
//
// Unlike pointers, Optional<T> contains the value T directly (with a validity
// flag). When T is an interface type, packing the Optional still requires
// packing the interface value, creating the same circular dependency as a
// direct interface field.
//
// This is a subtle distinction: Optional<SomeConcreteType*> would work (pointer
// breaks cycle), but Optional<IInterface> does not because the interface value
// must still be packed into the AnyValue representation.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

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

// CHECK: error[E41014]: type cannot be packed for dynamic dispatch
struct LinkedNode : INode
{
    float value;
    Optional<INode> next;  // Optional contains the interface value - no cycle break
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
