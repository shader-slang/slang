//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Verify backend generic specialization stops with a diagnostic when a
// recursive specialization chain exceeds the backend budget.

int recurse<let x : int>()
{
    return recurse<x + 1>();
//CHECK:                 ^ generic specialization exceeded maximum depth
//CHECK:                 ^ exceeded the recursive specialization budget (512)
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    recurse<0>();
}
