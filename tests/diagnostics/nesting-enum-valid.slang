//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// All declaration kinds allowed inside an enum.

enum E
{
    A,
    B,
}

// Enum with methods and nested types.
enum Rich
{
    X = 1,
    Y = 2,

    static int count() { return 2; }
    static int sVar;
    enum InnerEnum { P, Q }
    typealias MyInt = int;
    ;
}
