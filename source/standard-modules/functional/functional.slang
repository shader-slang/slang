/**
Experimental higher-order operations for finite builtin aggregate types.

@remarks Importers must enable experimental features. The API is intentionally outside `core` so
its names and signatures can evolve while practical experience is gathered.
*/
[ExperimentalModule]
module functional;

/// Adds higher-order element operations to fixed-size arrays.
public extension<T, let N : int> Array<T, N>
{
    /// Applies `function` to every element and returns the results in an array of the same length.
    [ForceInline]
    public Array<U, N> map<U, F : IFunc<U, T>>(F function)
    {
        static_assert(N != int.maxValue, "map requires an array with a statically known length");
        Array<U, N> result;
        for (int index = 0; index < N; ++index)
            result[index] = function(this[index]);
        return result;
    }

    /// Applies `function` to every element and returns the results in an array of the same length.
    [ForceInline]
    public Array<U, N> map<U>(functype(T) -> U function)
    {
        static_assert(N != int.maxValue, "map requires an array with a statically known length");
        Array<U, N> result;
        for (int index = 0; index < N; ++index)
            result[index] = function(this[index]);
        return result;
    }

    /// Generates an array by evaluating `elementAt` once for every element index.
    [ForceInline]
    public static This generate<F : IFunc<T, int>>(F elementAt)
    {
        static_assert(
            N != int.maxValue,
            "generate requires an array with a statically known length");
        This result;
        for (int index = 0; index < N; ++index)
            result[index] = elementAt(index);
        return result;
    }

    /// Initializes an array by evaluating `elementAt` once for every element index.
    [ForceInline]
    public __init<F : IFunc<T, int>>(F elementAt)
    {
        this = This.generate(elementAt);
    }
}

/// Adds higher-order element operations to builtin vectors.
public extension<T, let N : int> vector<T, N>
{
    /// Applies `function` to every element and returns the results in a vector of the same length.
    [ForceInline]
    public vector<U, N> map<U, F : IFunc<U, T>>(F function)
    {
        vector<U, N> result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = function(this[index]);
        return result;
    }

    /// Applies `function` to every element and returns the results in a vector of the same length.
    [ForceInline]
    public vector<U, N> map<U>(functype(T) -> U function)
    {
        vector<U, N> result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = function(this[index]);
        return result;
    }

    /// Generates a vector by evaluating `elementAt` once for every element index.
    [ForceInline]
    public static This generate<F : IFunc<T, int>>(F elementAt)
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = elementAt(index);
        return result;
    }

    /// Initializes a vector by evaluating `elementAt` once for every element index.
    [ForceInline]
    public __init<F : IFunc<T, int>>(F elementAt)
    {
        this = This.generate(elementAt);
    }
}

/// Adds higher-order element operations to builtin matrices.
public extension<T, let R : int, let C : int, let L : int> matrix<T, R, C, L>
{
    /// Applies `function` to every element and preserves the matrix dimensions and layout.
    [ForceInline]
    public matrix<U, R, C, L> map<U, F : IFunc<U, T>>(F function)
    {
        matrix<U, R, C, L> result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = function(this[row][column]);
        }
        return result;
    }

    /// Applies `function` to every element and preserves the matrix dimensions and layout.
    [ForceInline]
    public matrix<U, R, C, L> map<U>(functype(T) -> U function)
    {
        matrix<U, R, C, L> result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = function(this[row][column]);
        }
        return result;
    }

    /// Generates a matrix by evaluating `elementAt` once for every row and column index pair.
    [ForceInline]
    public static This generate<F : IFunc<T, int, int>>(F elementAt)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = elementAt(row, column);
        }
        return result;
    }

    /// Initializes a matrix by evaluating `elementAt` once for every row and column index pair.
    [ForceInline]
    public __init<F : IFunc<T, int, int>>(F elementAt)
    {
        this = This.generate(elementAt);
    }
}
