//TEST:SIMPLE(filecheck=CHECK):-target spirv

// This test verifies that a generic method defined in an extension can be found.

// CHECK: OpEntryPoint
struct Test {
    void genericMethod<int u>(float2 x) {}
    __subscript<T:__BuiltinIntegerType>(T idx1, T idx2) -> int
    {
        get { return 0; }
    }
}

extension Test {
    void genericMethod<int u>() {}
    __subscript<T:__BuiltinIntegerType>(T idx1) -> int
    {
        get { return 1; }
    }
}

[shader("compute")]
void main()
{
    Test t;
    t.genericMethod<3>();    // Should work - calls extension method
    int a = t[0];            // Should work - calls extension method
}
