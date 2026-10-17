//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Check that specialized interface constraints and erroneous bounds produce no hints.

// Check that IValue<V> is not suggested for Outer's T parameter.
// method's V is not in scope where T is declared.
interface IValue<V> {}
void dependentOp<V, U : IValue<V>>(V other, U value) {}
/*CHECK:
     ^^^^^^^^^^^ see declaration of func dependentOp<V, U> -> void
*/
struct Outer<T>
{
    void method<V>(T value, V other) { dependentOp(other, value); }
/*CHECK:
                                                  ^ type argument doesn't conform to interface
                                                  ^ type argument 'T' does not conform to the required interface 'IValue<V>'
*/
}

// Check that specialized constraints are skipped even when the argument is a visible type.
void concreteOp<U : IValue<float>>(U value) {}
/*CHECK:
     ^^^^^^^^^^ see declaration of func concreteOp<U> -> void
*/
void concreteArgument<T>(T value) { concreteOp(value); }
/*CHECK:
                                              ^ type argument doesn't conform to interface
                                              ^ type argument 'T' does not conform to the required interface 'IValue<float>'
*/

interface IRequired {}
void need<U : IRequired>(U value) {}
/*CHECK:
     ^^^^ see declaration of func need<U> -> void
*/
void invalidBound<T : Missing>(T value) { need(value); }
/*CHECK:
                      ^^^^^^^ undefined identifier
                      ^^^^^^^ undefined identifier 'Missing'.
                                              ^ type argument doesn't conform to interface
                                              ^ type argument 'T' does not conform to the required interface 'IRequired'
*/
