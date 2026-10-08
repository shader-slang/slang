//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A `try` covers only the call it wraps. A throwing call inside that call's operands needs its
// own `try`, as in `try g(try f())` (#13489).

enum MyError
{
    Failed
}

int f() throws MyError { throw MyError.Failed; }
int g(int x) throws MyError { return x; }
int k(int x) { return x; }

struct T
{
    int v;
    int get() throws MyError { return v; }
}
T mk() throws MyError { T t; t.v = 1; return t; }

struct S
{
    int x;
    __subscript(int i) -> int { get { return x + i; } }
}
S mkS() throws MyError { S s; s.x = 1; return s; }

struct Pair
{
    int a;
    int b;
}
int gp(Pair p) throws MyError { return p.a + p.b; }

int argument() throws MyError
{
    return try g(f());
//CHECK:          ^ callee may throw, use 'try'
//CHECK:          ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int argumentInDoCatch()
{
    do
    {
        return try g(f());
//CHECK:              ^ callee may throw, use 'try'
//CHECK:              ^ the callee may throw an error, and therefore must be called within a 'try' clause
    }
    catch (e : MyError)
    {
        return -1;
    }
}

int memberCallBase() throws MyError
{
    return try mk().get();
//CHECK:         ^ callee may throw, use 'try'
//CHECK:         ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int operatorOperand() throws MyError
{
    return try g(f() + 1);
//CHECK:          ^ callee may throw, use 'try'
//CHECK:          ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int nonThrowingOuterCall() throws MyError
{
    return try k(f());
//CHECK:          ^ callee may throw, use 'try'
//CHECK:          ^ the callee may throw an error, and therefore must be called within a 'try' clause
//CHECK:   ^^^ callee in 'try' does not throw
//CHECK:   ^^^ 'k' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
}

int subscriptIndex() throws MyError
{
    S s;
    s.x = 1;
    return try s[f()];
//CHECK:          ^ callee may throw, use 'try'
//CHECK:          ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int subscriptBase() throws MyError
{
    return try mkS()[0];
//CHECK:          ^ callee may throw, use 'try'
//CHECK:          ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

float conversionCast() throws MyError
{
    return try (float)f();
//CHECK:               ^ callee may throw, use 'try'
//CHECK:               ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int initializerListArgument() throws MyError
{
    return try gp({ f(), 1 });
//CHECK:             ^ callee may throw, use 'try'
//CHECK:             ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int arrayIndexArgument() throws MyError
{
    int a[2] = { 3, 4 };
    return try g(a[f() & 1]);
//CHECK:            ^ callee may throw, use 'try'
//CHECK:            ^ the callee may throw an error, and therefore must be called within a 'try' clause
}
