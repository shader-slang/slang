// Simple CoopVec test with integers

func testCoopVecInt<T : __BuiltinIntegerType, let N : int>(CoopVec<T, N> x) -> CoopVec<T, N>
{
    return x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<int, 4> vec = CoopVec<int, 4>(1, 2, 3, 4);

    // This should work - int satisfies __BuiltinIntegerType
    CoopVec<int, 4> result = testCoopVecInt(vec);
}