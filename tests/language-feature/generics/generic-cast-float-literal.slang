//TEST:INTERPRET(filecheck=CHECK):

// (half is omitted here: the interpreter cannot execute `cast.f32.f16`.)

T castQuarter<T : IArithmetic>()
{
    return (T)0.25;
}

T castVal<T : IArithmetic>(float v)
{
    return T(v);
}

struct MyNumber : IArithmetic
{
    int value;

    bool equals(This other) { return value == other.value; }
    bool lessThan(This other) { return value < other.value; }
    bool lessThanOrEquals(This other) { return value <= other.value; }

    This add(This other) { MyNumber r; r.value = value + other.value; return r; }
    This sub(This other) { MyNumber r; r.value = value - other.value; return r; }
    This mul(This other) { MyNumber r; r.value = value * other.value; return r; }
    This div(This other) { MyNumber r; r.value = value / other.value; return r; }
    This mod(This other) { MyNumber r; r.value = value % other.value; return r; }
    This neg() { MyNumber r; r.value = -value; return r; }

    __init(int val) { value = val; }
    __init(This other) { value = other.value; }
}

void main()
{
    float f = castQuarter<float>();
    //CHECK: f=0.25
    printf("f=%f\n", f);

    int i = castQuarter<int>();
    //CHECK: i=0
    printf("i=%d\n", i);

    float g = castVal<float>(2.75f);
    //CHECK: g=2.75
    printf("g=%f\n", g);

    int j = castVal<int>(2.75f);
    //CHECK: j=2
    printf("j=%d\n", j);

    MyNumber m = castVal<MyNumber>(2.75f);
    //CHECK: m=2
    printf("m=%d\n", m.value);
}
