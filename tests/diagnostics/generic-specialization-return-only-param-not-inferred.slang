//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// An ordinary generic type parameter `T` that appears only in return position
// is not constrained by any value argument, so it cannot be inferred from the
// call and should report which parameter could not be inferred.

T pick<T>()
{
    T v;
    return v;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    pick();
/*CHECK:
        ^ generic parameter could not be inferred
        ^ could not infer generic argument for parameter 'T'
*/
}
