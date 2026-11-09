//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default
//TEST:SIMPLE(filecheck=DEBUG): -target hgbc -entry main -g1 -validate-ir

// Generic methods can demand-lower global field keys. Those keys must not inherit the
// method's lexical debug scope, which can depend on local generic parameters.
// DEBUG: OpDebugSource
// DEBUG: OpDebugLine
// DEBUG: OpPrint
// DEBUG: OpScalarTypeCast

interface IBox<T>
{
    T get();
    int getCount();
}

class Box<T, let N : int> : IBox<T> where T : IArithmetic
{
    T value;
    Array<T, N> values;

    __init() {}

    [mutating]
    void set(T newValue)
    {
        value = newValue;
        for (int i = 0; i < N; i++)
            values[i] = newValue;
    }

    T get()
    {
        return value;
    }

    int getCount()
    {
        return N;
    }
}

[host]
void main()
{
    Box<int, 2> integers = new Box<int, 2>();
    integers.set(7);

    Box<float, 3> floats = new Box<float, 3>();
    floats.set(2.5);

    print(integers.get());
    print(integers.getCount());
    print(int(floats.get() * 2.0));
    print(floats.getCount());
}

// CHECK: 7
// CHECK-NEXT: 2
// CHECK-NEXT: 5
// CHECK-NEXT: 3
