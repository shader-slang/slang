// Test ambiguous overload case

func test<T : __BuiltinFloatingPointType>(T x) -> T { return x; }
func test<T : __BuiltinIntegerType>(T x) -> T { return x; }

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int value = 42;

    // This should potentially be ambiguous if both overloads are considered
    int result = test(value);
}