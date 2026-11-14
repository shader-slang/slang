//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Namespace scope is treated like module scope.
// All module-level declarations should be valid inside a namespace.

namespace NS
{
    struct S { int x; }
    interface IFoo { void method(); }
    enum E { A, B }
    void freeFunc() {}
    int globalVar;
    static int staticGlobalVar;
    typealias MyInt = int;
    namespace Inner {}
    ;
}
