//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// An internal generic method nested in several generic containers, and its derivatives, stay
// inaccessible from an importing module.

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
