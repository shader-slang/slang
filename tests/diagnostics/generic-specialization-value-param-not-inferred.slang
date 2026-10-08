//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// An ordinary generic value parameter `N` that is not mentioned in any value
// parameter cannot be inferred from the call arguments, so the call should
// report which parameter could not be inferred.

void h<let N : int>(int x) {}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    h(3);
/*CHECK:
     ^ generic parameter could not be inferred
     ^ could not infer generic argument for parameter 'N'
*/
}
