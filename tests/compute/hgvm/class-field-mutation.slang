//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

interface ICounter
{
    void set(int value);
    int get();
}

struct Pair
{
    int first;
    int second;
}

class Counter : ICounter
{
    int value;
    Pair pair;

    __init() { value = 0; pair = Pair(0, 0); }

    [noinline]
    void set(int newValue)
    {
        value = newValue;
        this.pair.first = newValue + 1;
    }

    int get() { return value; }

    property doubled : int
    {
        get { return value * 2; }
        set { value = newValue / 2; }
    }

    void setViaProperty(int newValue) { doubled = newValue; }

    [mutating]
    void legacySet(int newValue) { value = newValue; }
}

extension Counter
{
    void setSecond(int newValue) { this.pair.second = newValue; }
}

class Box<T>
{
    T value;
    __init(T initialValue) { value = initialValue; }
    void set(T newValue) { value = newValue; }
}

struct Holder
{
    Counter counter;
    property object : Counter { get { return counter; } }
}

[noinline]
void setGeneric<T : ICounter>(T counter, int value)
{
    counter.set(value);
}

[noinline]
void setInterface(ICounter counter, int value)
{
    counter.set(value);
}

[noinline]
Counter identity(Counter counter) { return counter; }

[host]
void main()
{
    let counter = new Counter();
    let alias = counter;
    counter.set(7);
    print(alias.get());
    print(alias.pair.first);
    counter.pair.second = 9;
    print(alias.pair.second);
    identity(counter).value = 11;
    print(alias.get());
    counter.doubled = 26;
    print(alias.get());
    counter.setViaProperty(30);
    print(alias.get());
    counter.setSecond(17);
    print(alias.pair.second);
    counter.legacySet(19);
    print(alias.get());

    setGeneric(counter, 21);
    print(alias.get());
    setInterface(counter, 23);
    print(alias.get());
    let objects = RuntimeArray<Counter>::alloc(1);
    objects[0] = counter;
    setGeneric(objects[0], 25);
    print(alias.get());
    setInterface(objects[0], 27);
    print(alias.get());

    let box = new Box<int>(29);
    let boxAlias = box;
    box.set(31);
    print(boxAlias.value);

    // An immutable value may still contain a handle to mutable object storage.
    let holder = Holder(counter);
    holder.counter.value = 33;
    print(alias.get());
    holder.object.pair.second = 35;
    print(alias.pair.second);
}

// CHECK: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 9
// CHECK-NEXT: 11
// CHECK-NEXT: 13
// CHECK-NEXT: 15
// CHECK-NEXT: 17
// CHECK-NEXT: 19
// CHECK-NEXT: 21
// CHECK-NEXT: 23
// CHECK-NEXT: 25
// CHECK-NEXT: 27
// CHECK-NEXT: 31
// CHECK-NEXT: 33
// CHECK-NEXT: 35
