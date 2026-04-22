//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test: __func_extension naming an unknown function should diagnose

__func_extension fwd_diff(nonexistent)(DifferentialPair<float> x) -> DifferentialPair<float>
//CHECK:                  ^^^^^^^^^^^ undefined identifier
//CHECK:                  ^^^^^^^^^^^ undefined identifier 'nonexistent'.
//CHECK:                  ^^^^^^^^^^^ expected a function
//CHECK:                  ^^^^^^^^^^^ expected a function, got 'error'
{
    return x;
}
