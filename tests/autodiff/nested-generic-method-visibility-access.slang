//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// An internal generic method nested in several generic containers stays inaccessible from an
// importing module, both directly and as the operand of `fwd_diff`.

import "nested-generic-method-visibility-lib";

void test()
{
    fwd_diff(Outer<float>.Inner<int>.cubeInternal)(diffPair(2.0, 1.0), 0);
    //CHECK: declaration not accessible
    //CHECK: 'cubeInternal' is not accessible from the current context.

    Outer<float>.Inner<int>.cubeInternal(2.0, 0);
    //CHECK: declaration not accessible
    //CHECK: 'cubeInternal' is not accessible from the current context.
}
