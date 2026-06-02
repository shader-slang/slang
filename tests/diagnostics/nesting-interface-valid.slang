//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// All declaration kinds allowed inside an interface.

interface IFoo
{
    associatedtype T;
    void method();
    __subscript(int i) -> int { get; }
    property int p { get; }
    static const int sVar = 0;
    ;
}
