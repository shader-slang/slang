//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

import "transitive-attribute-extensions-facade";

void test(Container container)
{
    container.operations.ordinaryExtension(2.0);
//CHECK:                 ^^^^^^^^^^^^^^^^^ member not found
//CHECK:                 ^^^^^^^^^^^^^^^^^ 'ordinaryExtension' is not a member of 'Operations'.
    fwd_diff(ExposedOperations.restricted)(container.operations, diffPair(2.0, 1.0));
/*CHECK:
    ^^^^^^^^ declaration not accessible
    ^^^^^^^^ 'fwd_diff' is not accessible from the current context.
*/
}
