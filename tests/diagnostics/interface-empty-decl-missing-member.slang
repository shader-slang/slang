//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Ignoring empty declarations must not ignore actual requirements.
interface IProperty
{
    ;
    property int value { get; };
//CHECK:         ^^^^^ see declaration of 'value'
}
struct MissingProperty : IProperty {}
//CHECK:                 ^^^^^^^^^ E38100
//CHECK:                 ^^^^^^^^^ type 'MissingProperty' does not provide required interface member 'value'

interface IMethod
{
    ;
    void method();
//CHECK: ^^^^^^ see declaration of 'method'
    ;
}
struct MissingMethod : IMethod {}
//CHECK:               ^^^^^^^ E38100
//CHECK:               ^^^^^^^ type 'MissingMethod' does not provide required interface member 'method'
