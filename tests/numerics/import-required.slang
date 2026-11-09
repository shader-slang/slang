//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):

T addValues<T : IAdditive>(T left, T right)
/*CHECK:
                ^^^^^^^^^ undefined identifier 'IAdditive'.
*/
{
    return left + right;
}
