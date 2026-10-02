// A struct that holds a pointer to its own type, reached through a pointer before the struct
// itself is needed (shader-slang/slang#13396). The source emitters order global declarations with
// a depth-first walk, and this test exercises its `Ptr(Node)` → `Node` → `Ptr(Node)` cycle.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=output
RWStructuredBuffer<float> output;

struct Node
{
    float value;
    Node* next;
};

// `head` is reached first, so the walk opens `Ptr(Node)` before `Node`.
cbuffer Params
{
    Node* head;
}

float sumList(Node* node)
{
    float sum = 0;
    while (node != nullptr)
    {
        sum += node->value;
        node = node->next;
    }
    return sum;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = (head == nullptr) ? 1.0 : 0.0;

    // We assign fields one by one: brace initialization synthesizes a `Node` constructor that
    // reaches `Node` before `head`, which hides the cycle on the CPU target.
    Node c;
    c.value = 4.0;
    c.next = nullptr;
    Node b;
    b.value = 2.0;
    b.next = &c;
    Node a;
    a.value = 1.0;
    a.next = &b;
    output[1] = sumList(&a);
    output[2] = a.next->next->value;
}

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 4.0
