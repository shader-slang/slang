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

class Holder
{
    Counter counter;
    Pair pair;
    RWStructuredBuffer<uint> buffer;

    __init() {}
}

class Node
{
    uint value;
    Node next;

    __init() {}
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeValue(RWStructuredBuffer<uint> output, uint value)
{
    output[0] = value;
}

[noinline]
Holder makeHolder(Counter counter, Pair pair, RWStructuredBuffer<uint> buffer)
{
    Holder result = new Holder();
    result.counter = counter;
    result.pair = pair;
    result.buffer = buffer;
    return result;
}

[host]
void main()
{
    uint three = hgbc_asm { result : $$uint = OpIntLit 3; };
    uint five = hgbc_asm { result : $$uint = OpIntLit 5; };
    uint eight = hgbc_asm { result : $$uint = OpIntLit 8; };
    uint thirteen = hgbc_asm { result : $$uint = OpIntLit 13; };

    Counter child = new Counter();
    child.set(three);
    Pair pair;
    pair.first = five;
    pair.second = eight;
    RWStructuredBuffer<uint> originalBuffer = RWStructuredBuffer<uint>::alloc(1);
    Holder holder = makeHolder(child, pair, originalBuffer);

    child = new Counter();
    child.set(thirteen);
    holder.counter.set(eight);
    holder.pair.first = holder.pair.first + holder.pair.second;
    dispatch(writeValue, uint3(1, 1, 1))(holder.buffer, holder.pair.first);

    Counter retainedChild = holder.counter;
    holder = makeHolder(child, pair, RWStructuredBuffer<uint>::alloc(1));

    print(retainedChild.get());
    print(readBack(originalBuffer, 0));
    print(holder.counter.get());

    Node tail = new Node();
    tail.value = thirteen;
    Node head = new Node();
    head.next = tail;
    tail = new Node();
    print(head.next.value);
}

// CHECK: 8
// CHECK-NEXT: 13
// CHECK-NEXT: 13
// CHECK-NEXT: 13
