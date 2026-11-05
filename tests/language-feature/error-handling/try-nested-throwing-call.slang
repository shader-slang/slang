//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A `try` covers only the call it wraps. A throwing call inside that call's operands needs its
// own `try`, as in `try g(try f())` (#13489). Apart from `nonThrowingOuterCall`, every `try` here
// wraps a throwing call, so the only diagnostics are for the uncovered inner calls.

enum MyError
{
    Failed
}

int f() throws MyError { throw MyError.Failed; }
int g(int x) throws MyError { return x; }
int g2(int a, int b) throws MyError { return a + b; }
float gf(float x) throws MyError { return x; }
uint gu(uint x) throws MyError { return x; }
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

int siblingAfterExplicitTry() throws MyError
{
    return try g2(try f(), f());
//CHECK:                    ^ callee may throw, use 'try'
//CHECK:                    ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int subscriptIndex() throws MyError
{
    S s;
    s.x = 1;
    return try g(s[f()]);
//CHECK:            ^ callee may throw, use 'try'
//CHECK:            ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int subscriptBase() throws MyError
{
    return try g(mkS()[0]);
//CHECK:            ^ callee may throw, use 'try'
//CHECK:            ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

float castConversion() throws MyError
{
    return try gf((float)f());
//CHECK:                  ^ callee may throw, use 'try'
//CHECK:                  ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

float constructorConversion() throws MyError
{
    return try gf(float(f()));
//CHECK:                 ^ callee may throw, use 'try'
//CHECK:                 ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

uint unsignedConversion() throws MyError
{
    return try gu(uint(f()));
//CHECK:                ^ callee may throw, use 'try'
//CHECK:                ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int identityOverIndex() throws MyError
{
    int a[2] = { 3, 4 };
    return try g(int(a[f() & 1]));
//CHECK:                ^ callee may throw, use 'try'
//CHECK:                ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int identityOverOperator() throws MyError
{
    return try g(int(f() + 1));
//CHECK:              ^ callee may throw, use 'try'
//CHECK:              ^ the callee may throw an error, and therefore must be called within a 'try' clause
}

int identityOverMember() throws MyError
{
    return try g(int(mk().v));
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
