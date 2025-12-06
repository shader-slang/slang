// Test if int directly satisfies __BuiltinIntegerType

func testIntDirect<T : __BuiltinIntegerType>(T x) -> T { return x; }

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int value = 42;

    // This should work - int directly satisfies __BuiltinIntegerType
    int result = testIntDirect(value);
}