//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// Test: 'each' expression used outside of 'expand' produces error 30104.

void badEachOutsideExpand<each T>(expand each T values)
{
    // Using 'each' without surrounding 'expand' is invalid.
    let x = each values;
//CHECK:    ^^^^ 'each' expression must be inside 'expand' expression
}
