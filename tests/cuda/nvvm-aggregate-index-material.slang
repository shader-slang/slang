//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):name=inputs
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputs
// CHECK: 1
// CHECK-NEXT: 26
// CHECK-NEXT: 83
// CHECK-NEXT: 108
// CHECK-NEXT: 1
// CHECK-NEXT: 26
// CHECK-NEXT: 83
// CHECK-NEXT: 108

// Preserve the material reduction's initialized aggregate, receiver snapshot, and nested copy.
struct Frame
{
    float3 tangent;
    float3 bitangent;
    float3 normal;
};
struct Surface
{
    float3 incoming;
    float3 normal;
    Frame frame;
    uint front;
};
struct Stack
{
    float weights[2];
    float3 normals[2];
    float3 tangents[2];
    float unused[20];
    uint root;
};
struct Graph
{
    uint handles[2];
    Surface si;
    uint hints;
    float lod;
    float matrices[50];
    int counters[2];
    Stack stack;
    float outputMetadata[4];
};

// Initialize every field, including the unused arrays copied with the receiver.
Graph initialize(float x, float y, uint tag)
{
    Graph graph = {};
    graph.handles[0] = 17;
    graph.handles[1] = 23;
    graph.si.incoming = float3(x, y, 1);
    graph.si.normal = float3(0.25, 0.5, 1);
    graph.si.frame.tangent = float3(1, 0, 0);
    graph.si.frame.bitangent = float3(0, 1, 0);
    graph.si.frame.normal = float3(0, 0, 1);
    graph.si.front = 1;
    graph.hints = tag;
    return graph;
}

// Read the saved surface and select either the normal or tangent contribution.
float adjust(Graph receiver, float x)
{
    float value = x * receiver.si.normal.x
        + receiver.si.incoming.y * receiver.si.normal.y + receiver.si.normal.z;
    return (receiver.hints & 2) != 0
        ? value + receiver.si.frame.normal.z : value - receiver.si.frame.tangent.x;
}

// Capture the receiver before updating the selected lobe of its nested stack.
void prepare(inout Graph graph, uint slot, float x)
{
    Graph snapshot = graph;
    float adjusted = adjust(snapshot, x);
    graph.stack.weights[slot] = adjusted;
    graph.stack.normals[slot] = graph.si.incoming;
    graph.stack.tangents[slot] = graph.si.frame.tangent;
    graph.counters[0] = int(slot + 1);
    graph.counters[1] = 1;
    graph.stack.root = slot;
}

// Read all three fields written to the selected stack element.
float evaluate(Stack stack, uint slot)
{
    return stack.weights[slot] + stack.normals[slot].x
        + 2 * stack.normals[slot].y + stack.tangents[slot].x;
}

// Exercise a runtime index through both the nested mutation and the final value copy.
float evaluateBounded(float x, float y, uint tag)
{
    Graph graph = initialize(x, y, tag);
    uint slot = tag & 1;
    prepare(graph, slot, x);
    Stack material = graph.stack;
    return evaluate(material, slot);
}

// Keep a distinct constant-index control with the same initialization and copy operations.
float evaluateLiteral(float x, float y, uint tag)
{
    Graph graph = initialize(x, y, tag);
    prepare(graph, 0, x);
    Stack material = graph.stack;
    return evaluate(material, 0);
}

StructuredBuffer<uint> inputs;
RWStructuredBuffer<uint> outputs;
[numthreads(1, 1, 1)]
void computeMain()
{
    for (uint i = 0; i < 4; ++i)
    {
        uint tag = inputs[i];
        float x = (float(tag) + 1) * 0.25;
        float y = (float(tag) - 1) * 0.5;
        // The scalar result is 1.25*x + 2.5*y + ((tag & 2) ? 3 : 1).
        // Scaling these exact dyadic cases by 16 gives the integer checks above.
        outputs[i] = uint(16 * evaluateBounded(x, y, tag));
        outputs[4 + i] = uint(16 * evaluateLiteral(x, y, tag));
    }
}
