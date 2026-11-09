// `try` applied to a call whose callee does not throw is diagnosed with E30091 whatever the
// kind of callee: constructors (user, synthesized, generic, interface, builtin), subscripts
// (user, buffer, interface, setter form) and values of function type. The error type of a
// generic callee is checked against the caller after substitution. Throwing callees reached
// through functions, methods, generic constraints and existentials are accepted.
//
// E30091 keeps the wording it has for functions. Its hint to mark the callee 'throws' cannot
// yet be followed for constructors and subscripts (#13492), nor for a `functype` value.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum MyError
{
    Failed
}

int throwing(int x) throws MyError
{
    if (x < 0)
        throw MyError.Failed;
    return x;
}

struct S
{
    int x;
    __init(int v) { x = v; }
    __subscript(int i)->int
    {
        get { return x + i; }
        set { x = newValue; }
    }
    int method(int v) throws MyError { return try throwing(v); }
}

struct Plain
{
    int x;
}

struct G<T>
{
    T x;
    __init(T v) { x = v; }
}

interface I
{
    __init(int v);
    __subscript(int i)->int { get; }
    int get(int v) throws MyError;
}

RWStructuredBuffer<int> buffer;

int userCtor() throws MyError
{
    S s = try S(1);
/*CHECK:
          ^^^ E30091
          ^^^ 'S.init' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
    return s.x;
}

int synthesizedDefaultCtor() throws MyError
{
    Plain p = try Plain();
/*CHECK:
              ^^^ E30091
              ^^^ 'Plain.init' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
    return p.x;
}

int genericCtor() throws MyError
{
    G<int> g = try G<int>(1);
/*CHECK:
               ^^^ E30091
               ^^^ 'G.init' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
    return g.x;
}

int interfaceCtor<T : I>() throws MyError
{
    T t = try T(1);
/*CHECK:
          ^^^ E30091
          ^^^ 'I.init' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
    return try t.get(0);
}

int builtinCtor() throws MyError
{
    return try int(3.0);
/*CHECK:
           ^^^ E30091
           ^^^ 'int.init' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
}

int userSubscript() throws MyError
{
    S s = S(1);
    return try s[0];
/*CHECK:
           ^^^ E30091
           ^^^ 'operator[]' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
}

int bufferSubscript() throws MyError
{
    return try buffer[0];
/*CHECK:
           ^^^ E30091
           ^^^ 'operator[]' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
}

int interfaceSubscript<T : I>(T t) throws MyError
{
    return try t[0];
/*CHECK:
           ^^^ E30091
           ^^^ 'operator[]' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
}

int subscriptSetter() throws MyError
{
    S s = S(1);
    try s[0] = 5;
/*CHECK:
    ^^^ E30091
    ^^^ 'operator[]' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
    return s.x;
}

int functionTypedParam(functype(int)->int f) throws MyError
{
    return try f(1);
/*CHECK:
           ^^^ E30091
           ^^^ 'f' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'
*/
}

enum OtherError
{
    Other
}

int genericThrowing<E>(E e, int x) throws E
{
    if (x < 0)
        throw e;
    return x;
}

int substitutedErrorTypeMismatch() throws OtherError
{
    return try genericThrowing<MyError>(MyError.Failed, 1);
/*CHECK:
           ^^^ E30095
           ^^^ the error type `MyError` of callee `genericThrowing` is not compatible with the caller's error type `OtherError`.
*/
}

// Positive controls: throwing callees reached through a function, a method, a generic
// constraint and an existential, a throwing call in a constructor argument, and a generic
// error type substituted to the caller's, with the error propagated or caught.
int throwingCallees<U : I>(U u, I i) throws MyError
{
    S s = S(try throwing(1));
    int a = try s.method(2);
    int b = try u.get(3);
    int c = try i.get(4);
    int d = 0;
    do
    {
        d = try throwing(-1);
    }
    catch (e: MyError)
    {
        d = 5;
    }
    int f = try genericThrowing<MyError>(MyError.Failed, 6);
    return s.x + a + b + c + d + f;
}
