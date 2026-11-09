//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct Pair
{
    uint first;
    uint second;
}

class Counter
{
    uint value;

    __init() {}

    [mutating]
    void set(uint newValue)
    {
        value = newValue;
    }

    uint get()
    {
        return value;
    }
}

class Node
{
    uint value;
    RuntimeArray<Node> children;

    __init() {}
}

[host]
void main()
{
    uint one = hgbc_asm { result : $$uint = OpIntLit 1; };
    uint two = hgbc_asm { result : $$uint = OpIntLit 2; };
    uint five = hgbc_asm { result : $$uint = OpIntLit 5; };
    uint eight = hgbc_asm { result : $$uint = OpIntLit 8; };
    uint thirteen = hgbc_asm { result : $$uint = OpIntLit 13; };

    Counter first = new Counter();
    first.set(five);
    RuntimeArray<Counter> counters = RuntimeArray<Counter>::alloc(two);
    counters[0] = first;
    first = new Counter();
    first.set(thirteen);
    counters[1] = first;

    RuntimeArray<Counter> retainedCounters = counters;
    counters = RuntimeArray<Counter>::alloc(one);
    print(retainedCounters[0].get());
    print(retainedCounters[1].get());

    RuntimeArray<Pair> pairs = RuntimeArray<Pair>::alloc(one);
    pairs[0].first = five;
    pairs[0].second = eight;
    print(pairs[0].first + pairs[0].second);

    RuntimeArray<uint> inner = RuntimeArray<uint>::alloc(two);
    inner[1] = thirteen;
    RuntimeArray<RuntimeArray<uint>> outer =
        RuntimeArray<RuntimeArray<uint>>::alloc(one);
    outer[0] = inner;
    inner = RuntimeArray<uint>::alloc(one);
    print(outer[0][1]);

    Node root = new Node();
    root.children = RuntimeArray<Node>::alloc(one);
    Node child = new Node();
    child.value = eight;
    root.children[0] = child;
    child = new Node();
    print(root.children[0].value);
}

// CHECK: 5
// CHECK-NEXT: 13
// CHECK-NEXT: 13
// CHECK-NEXT: 13
// CHECK-NEXT: 8
