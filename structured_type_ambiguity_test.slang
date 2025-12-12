// Test ambiguous overload case with structured types

struct MyContainer<T, let N : int>
{
    T data[N];
};

func test<T : __BuiltinFloatingPointType, let N : int>(MyContainer<T, N> x, MyContainer<T, N> y) -> MyContainer<T, N> { return x; }
func test<T : __BuiltinIntegerType, let N : int>(MyContainer<T, N> x, MyContainer<T, N> y) -> MyContainer<T, N> { return x; }

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    MyContainer<int, 4> containerA;
    MyContainer<int, 4> containerB;

    // This should potentially be ambiguous - similar to CoopVec case
    MyContainer<int, 4> result = test(containerA, containerB);
}