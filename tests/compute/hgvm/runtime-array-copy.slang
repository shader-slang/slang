//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

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

[host]
void main()
{
    RuntimeArray<uint> source = RuntimeArray<uint>::alloc(4);
    source[0] = 5;
    source[1] = 8;
    source[2] = 13;
    source[3] = 21;
    RuntimeArray<uint> destination = RuntimeArray<uint>::alloc(4);
    copy(destination, 1, source, 0, 3);
    copy(destination, 0, destination, 0, 4);
    copy(destination, 4, source, 4, 0);
    print(destination[0]);
    print(destination[1]);
    print(destination[2]);
    print(destination[3]);

    RuntimeArray<Counter> counters = RuntimeArray<Counter>::alloc(2);
    RuntimeArray<Counter> copiedCounters = RuntimeArray<Counter>::alloc(2);
    Counter first = new Counter();
    first.set(34);
    Counter second = new Counter();
    second.set(55);
    Counter replaced = new Counter();
    replaced.set(1);
    counters[0] = first;
    counters[1] = second;
    copiedCounters[0] = replaced;
    copiedCounters[1] = replaced;
    copy(copiedCounters, 0, counters, 0, 2);
    counters = RuntimeArray<Counter>::alloc(0);
    first = new Counter();
    second = new Counter();
    replaced = new Counter();
    print(copiedCounters[0].get());
    print(copiedCounters[1].get());

    RuntimeArray<uint> inner = RuntimeArray<uint>::alloc(1);
    inner[0] = 89;
    RuntimeArray<RuntimeArray<uint>> nestedSource =
        RuntimeArray<RuntimeArray<uint>>::alloc(1);
    RuntimeArray<RuntimeArray<uint>> nestedDestination =
        RuntimeArray<RuntimeArray<uint>>::alloc(1);
    nestedSource[0] = inner;
    copy(nestedDestination, 0, nestedSource, 0, 1);
    nestedSource = RuntimeArray<RuntimeArray<uint>>::alloc(0);
    inner = RuntimeArray<uint>::alloc(0);
    print(nestedDestination[0][0]);
}

// CHECK: standard output = {
// CHECK-NEXT: 0
// CHECK-NEXT: 5
// CHECK-NEXT: 8
// CHECK-NEXT: 13
// CHECK-NEXT: 34
// CHECK-NEXT: 55
// CHECK-NEXT: 89
