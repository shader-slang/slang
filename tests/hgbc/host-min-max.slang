//TEST:INTERPRET(hgvm):
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

void check(bool condition)
{
    if (!condition)
    {
        let message = "Incorrect host min/max result";
        hgbc_asm { OpFail $message; };
    }
}

[noinline]
T floatingMin<T : IFloat>(T a, T b) { return min(a, b); }
[noinline]
T integerMax<T : IInteger>(T a, T b) { return max(a, b); }

// Keep inputs dynamic so these tests execute bytecode instead of folding constants.
[noinline]
void checkInteger<T : __BuiltinIntegerType>(T low, T high)
{
    check(min(low, high) == low);
    check(min(high, low) == low);
    check(max(low, high) == high);
    check(max(high, low) == high);
    check(min(low, low) == low);
    check(max(high, high) == high);
    let a = vector<T, 3>(low, high, low);
    let b = vector<T, 3>(high, low, low);
    let lo = min(a, b);
    let hi = max(a, b);
    let genericHi = integerMax(a, b);
    for (uint i = 0; i < 3; ++i)
    {
        check(lo[i] == low);
        check(hi[i] == (i == 2 ? low : high));
        check(genericHi[i] == hi[i]);
    }
}

[noinline]
void checkFloat<T : __BuiltinFloatingPointType>(T low, T high)
{
    check(low < high);
    check(min(low, high) == low);
    check(min(high, low) == low);
    check(max(low, high) == high);
    check(max(high, low) == high);
    let a = vector<T, 3>(low, high, low);
    let b = vector<T, 3>(high, low, low);
    let lo = min(a, b);
    let hi = max(a, b);
    for (uint i = 0; i < 3; ++i)
    {
        check(lo[i] == low);
        check(hi[i] == (i == 2 ? low : high));
    }
    // Generate special values at runtime without relying on host bitcast/isnan support.
    T zero = low - low;
    T nan = zero / zero;
    T inf = T(1) / zero;
    check(min(nan, low) == low && min(low, nan) == low);
    check(max(nan, high) == high && max(high, nan) == high);
    check(min(nan, nan) != min(nan, nan));
    check(max(nan, nan) != max(nan, nan));
    check(min(-inf, low) == -inf && max(inf, high) == inf);
    check(min(zero, -zero) == zero && max(-zero, zero) == zero);
}

[noinline]
void checkMatrices<let L : MatrixLayoutMode>(
    matrix<int, 2, 3, L> a,
    matrix<int, 2, 3, L> b,
    matrix<float, 2, 3, L> x,
    matrix<float, 2, 3, L> y)
{
    let loInt = min(a, b);
    let hiInt = max(a, b);
    let loFloat = min(x, y);
    let hiFloat = max(x, y);
    let genericLoFloat = floatingMin(x, y);
    for (uint row = 0; row < 2; ++row)
        for (uint col = 0; col < 3; ++col)
        {
            int expected = int(row * 3 + col + 1);
            check(genericLoFloat[row][col] == -float(expected));
            check(loInt[row][col] == -expected && hiInt[row][col] == expected);
            check(loFloat[row][col] == -float(expected) && hiFloat[row][col] == float(expected));
        }
}

[noinline]
T comparableMin<T : IComparable>(T a, T b) { return min(a, b); }
[noinline]
T comparableMax<T : IComparable>(T a, T b) { return max(a, b); }

[host]
void main()
{
    // Exercise the numeric types currently emitted by HGBC (int/uint/half/float).
    checkInteger<int>(-2147483647 - 1, 2147483647);
    checkInteger<uint>(0, 0xffffffffu);
    checkFloat<half>(-2.5h, 3.5h);
    checkFloat<float>(-2.5f, 3.5f);
    checkMatrices(int2x3(-1, 2, -3, 4, -5, 6), int2x3(1, -2, 3, -4, 5, -6),
        float2x3(-1, 2, -3, 4, -5, 6), float2x3(1, -2, 3, -4, 5, -6));
    // Keep non-default matrix layout through both the shaped overload and IFloat dispatch.
    typealias ColumnInt = matrix<int, 2, 3, MatrixLayoutMode.ColumnMajor>;
    typealias ColumnFloat = matrix<float, 2, 3, MatrixLayoutMode.ColumnMajor>;
    checkMatrices(ColumnInt(-1, 2, -3, 4, -5, 6), ColumnInt(1, -2, 3, -4, 5, -6),
        ColumnFloat(-1, 2, -3, 4, -5, 6), ColumnFloat(1, -2, 3, -4, 5, -6));
    check(comparableMin<int>(-3, 7) == -3);
    check(comparableMax<float>(-3.0f, 7.0f) == 7.0f);
}

// CHECK-DAG: OpMin
// CHECK-DAG: OpMax
