// Regression test for issue #7152: calling an inaccessible single-parameter
// constructor of a struct defined in a separate module used to segfault.
// It must now report an accessibility error rather than crash.
//
// `B` uses the implicit (internal) default visibility — matching the issue's
// reproducer — and `C` uses an explicit `private` modifier, so both
// visibility-resolution paths are covered.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

import "private-init-cross-module-helper";

[shader("vertex")]
float4 foo() : SV_Position
{
    B bar = B(0.0);
//CHECK:     ^ declaration not accessible
//CHECK:     ^ 'B.init' is not accessible from the current context.
//CHECK:      ^ type mismatch in expression
//CHECK:      ^ expected an expression of type 'B', got 'float'

    C baz = C(0.0);
//CHECK:     ^ declaration not accessible
//CHECK:     ^ 'C.init' is not accessible from the current context.
//CHECK:      ^ type mismatch in expression
//CHECK:      ^ expected an expression of type 'C', got 'float'
    return float4(1, 1, 1, 1);
}
