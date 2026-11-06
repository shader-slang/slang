//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Check that suggested constraint names resolve at the type parameter declaration.

interface IBase {}
interface IDerived : IBase {}
void need<U : IDerived>(U value) {}
void need(int2 value) {}

// Check that a type parameter shadowing the required interface suppresses the hint.
void shadowed<IDerived, T : IBase>(T value) { need(value); }
/*CHECK:
                                                   ^^^^^ type mismatch in expression
                                                   ^^^^^ expected an expression of type 'vector<int,2>', got 'T'
*/

// Check that method's shadow does not suppress a hint for Outer's T parameter.
// IDerived still resolves to the interface where T is declared.
struct Outer<T : IBase>
/*CHECK:
             ^ consider constraining 'T' as 'T : IDerived' to satisfy an overload's requirement 'IDerived'
*/
{
    void method<IDerived>(T value) { need(value); }
/*CHECK:
                                          ^^^^^ type mismatch in expression
                                          ^^^^^ expected an expression of type 'vector<int,2>', got 'T'
*/
}

namespace Left
{
    interface IShared {}
    void needShared<U : IShared>(U value) {}
    void needShared(int2 value) {}
}
namespace Right
{
    interface IShared {}
}
using namespace Left;
using namespace Right;

// Check that no hint is emitted when the required interface name is ambiguous.
void ambiguous<T>(T value) { needShared(value); }
/*CHECK:
                                        ^^^^^ type mismatch in expression
                                        ^^^^^ expected an expression of type 'vector<int,2>', got 'T'
*/
