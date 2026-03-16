//TEST:INTERPRET(filecheck=CHECK):

// Test mixed explicit/implicit binding: first type parameter is explicit,
// second type parameter is inferred from the function argument.

interface IValue
{
    int toInt();
    __init(int x);
}

struct IntBox : IValue
{
    int v;
    __init(int x) { v = x; }
    int toInt() { return v; }
}

struct FloatBox : IValue
{
    float v;
    __init(int x) { v = float(x); }
    int toInt() { return int(v); }
}

// Result is explicit, Input is inferred from the argument.
Result convert<Result : IValue, Input : IValue>(Input input)
{
    Result r = Result(input.toInt() * 10);
    return r;
}

// T is explicit and constrained to IValue, U is inferred.
T add<T : IValue, U : IValue>(U val)
{
    T result = T(val.toInt() + 1);
    return result;
}

void main()
{
    // Provide Result=FloatBox explicitly, infer Input=IntBox from argument.
    IntBox ib = IntBox(3);
    FloatBox fb = convert<FloatBox>(ib);
    printf("%d\n", fb.toInt());
    // CHECK: 30

    // Provide Result=IntBox explicitly, infer Input=FloatBox from argument.
    FloatBox fb2 = FloatBox(5);
    IntBox ib2 = convert<IntBox>(fb2);
    printf("%d\n", ib2.toInt());
    // CHECK: 50

    // Provide T=IntBox explicitly, infer U=FloatBox from argument.
    FloatBox fb3 = FloatBox(7);
    IntBox ib3 = add<IntBox>(fb3);
    printf("%d\n", ib3.toInt());
    // CHECK: 8
}
