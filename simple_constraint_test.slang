// Simple test to understand constraint checking

func testFloat<T : __BuiltinFloatingPointType>(T x) -> T { return x; }
func testInt<T : __BuiltinIntegerType>(T x) -> T { return x; }

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int value = 42;

    // This should not be ambiguous - only testInt should match
    int result = testInt(value);

    // Let's try calling a generic function with ambiguous overloads
    int result2 = testFloat(value);  // This should fail or prefer testInt
}