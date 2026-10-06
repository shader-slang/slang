//TEST:EXECUTABLE:

// https://github.com/shader-slang/slang/issues/12485

class Counter
{
    int v;
    __init(int start) { v = start * 2; }
    __init(int a, int b) { v = a + b; }
    int get() { return v; }
}

// Overloads that differ only by argument type, to exercise candidate ranking on
// the overload-resolution path that single-arg class construction uses.
class Widget
{
    int v;
    __init(int x) { v = x; }
    __init(float x) { v = (int)x + 100; }
    int get() { return v; }
}

// A single-argument constructor taking the class itself, so `new Box(existing)`
// must invoke this constructor rather than fold into an identity no-op.
class Box
{
    int v;
    __init(int x) { v = x; }
    __init(Box other) { v = other.v + 1000; }
    int get() { return v; }
}

export __extern_cpp int main()
{
    Counter one = new Counter(4);
    printf("one=%d\n", one.get());

    Counter scalar = new Counter(int(3.0));
    printf("scalar=%d\n", scalar.get());

    Counter two = new Counter(4, 5);
    printf("two=%d\n", two.get());

    Counter list = new Counter({7});
    printf("list=%d\n", list.get());

    Widget wi = new Widget(5);
    printf("wi=%d\n", wi.get());

    Widget wf = new Widget(2.5f);
    printf("wf=%d\n", wf.get());

    // `new Box(existing)` is construction: it must run `__init(Box)`, not return
    // the argument unchanged.
    Box orig = new Box(5);
    Box copy = new Box(orig);
    printf("copy=%d\n", copy.get());

    // A same-type cast of a class value is an identity coercion (a no-op),
    // distinct from construction: it must not be rerouted through overload
    // resolution and rejected.
    Counter castParen = (Counter)one;
    printf("castParen=%d\n", castParen.get());

    Counter castCall = Counter(one);
    printf("castCall=%d\n", castCall.get());

    return 0;
}
