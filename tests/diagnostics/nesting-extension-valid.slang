//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// All declaration kinds allowed inside an extension.

struct Base { int x; }

extension Base
{
    struct Inner { int y; }
    enum E { A, B }
    void method() {}
    static void staticMethod() {}
    __init() {}
    __subscript(int i) -> int { get { return 0; } }
    property int p { get { return 0; } }
    static int sVar;
    typealias MyInt = int;
    ;
}
