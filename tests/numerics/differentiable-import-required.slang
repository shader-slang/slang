//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature

import slang.numerics;

T differentiableSine<T : IDifferentiableTrigonometricFunctions>(T value)
/*CHECK:
                         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ undefined identifier 'IDifferentiableTrigonometricFunctions'.
*/
{
    return value;
}
