//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Skipping an unrelated sibling constraint during an in-progress inheritance query must not
// suppress its real conformance error. Normal declaration checking must still diagnose both
// invalid dependent bounds below.

interface IContext {}

interface IConsumer<T> where T : IContext {}

interface IInvalidLayout
{
    associatedtype Context;
    associatedtype First : IConsumer<Context>;
/*CHECK:
                           ^^^^^^^^^ E38029
                           ^^^^^^^^^ type argument 'Context' does not conform to the required interface 'IContext'
*/
    associatedtype Second : IConsumer<Context>;
/*CHECK:
                            ^^^^^^^^^ E38029
                            ^^^^^^^^^ type argument 'Context' does not conform to the required interface 'IContext'
*/
}
