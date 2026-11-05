//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A conversion of a value to the type it already has resolves to that value itself, so a `try` on
// an identity conversion of a throwing call covers the call, as in `try int(f())` where `f`
// returns `int` (#13489). None of these functions is diagnosed. A conversion to a different type
// is a separate call, whose throwing operand needs its own `try` (see
// `try-nested-throwing-call.slang`).

enum MyError
{
    Failed
}

int f() throws MyError { throw MyError.Failed; }
T gf<T>(T x) throws MyError { return x; }
typedef int MyInt;

struct V
{
    int v;
    int get() throws MyError { return v; }
}
V mkV() throws MyError { V v; v.v = 1; return v; }

int constructorStyle() throws MyError { return try int(f()); }
int castStyle() throws MyError { return try (int)f(); }
int nested() throws MyError { return try int(int(f())); }
int mixed() throws MyError { return try (int)int(f()); }
int throughTypedef() throws MyError { return try MyInt(f()); }
int ofMethod(V v) throws MyError { return try int(v.get()); }
V ofStruct() throws MyError { return try V(mkV()); }
T generic<T>(T x) throws MyError { return try T(gf<T>(x)); }
T genericCast<T>(T x) throws MyError { return try (T)gf<T>(x); }
int genericAtInt() throws MyError { return try generic<int>(1) + try genericCast<int>(2); }
