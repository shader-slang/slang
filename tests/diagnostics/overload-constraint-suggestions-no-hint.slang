//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Check that no hints are emitted for synthesized This or ambiguous applicable overloads.

interface IRequired {}
void need<U : IRequired>(U value) {}
/*CHECK:
     ^^^^ see declaration of func need<U> -> void
*/

// Check that no constraint is suggested for the compiler-generated This parameter.
// The source fix would change interface inheritance.
interface IBase
{
    void f() { need(this); }
/*CHECK:
                   ^ type argument doesn't conform to interface
                   ^ type argument 'This' does not conform to the required interface 'IRequired'
*/
}

interface ILeft {}
interface IRight {}
void choose<U : ILeft>(U value) {}
/*CHECK:
     ^^^^^^ candidate: func choose<T>(T) -> void
*/
void choose<U : IRight>(U value) {}
/*CHECK:
     ^^^^^^ candidate: func choose<T>(T) -> void
*/
void choose<U : IRequired>(U value) {}

// Check that no third constraint is suggested when two overloads already apply.
void applicableAmbiguity<T : ILeft & IRight>(T value) { choose(value); }
/*CHECK:
                                                              ^ ambiguous call to 'choose' with arguments of type (T)
*/
