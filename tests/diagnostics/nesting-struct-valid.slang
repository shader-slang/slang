//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// All declaration kinds that ARE allowed inside a struct.
// No nesting-validation diagnostics (E31400) should fire.

struct S
{
    struct Inner { int x; }
    enum E { A, B }
    void method() {}
    static void staticMethod() {}
    __subscript(int i) -> int { get { return 0; } }
    property int p { get { return 0; } }
    int field;
    static int sVar;
    typealias MyInt = int;
    typedef int MyInt2;
    ;
}
