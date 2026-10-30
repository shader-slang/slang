//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A `where optional T : I` constraint does not prove `T : I` outside an `if (T is I)` check,
// and so it does not prove `T` conforms to the interfaces `I` inherits either. Each function
// below needs a conformance the optional constraint only inherits: `add` is an `IArithmetic`
// requirement, and `+`, `-`, `%`, `*` and `/` resolve to core-module overloads constrained on
// `IArithmetic` or `__BuiltinArithmeticType`. Each use must be rejected the same way a use
// through `where optional T : IArithmetic` is (shader-slang/slang#13011, shader-slang/slang#13061).

interface IA { This plus(This o); }
interface IB : IA {}

interface IX { int get(); }
interface IFoo { associatedtype A : IX; A make(); }
interface IBar : IFoo {}

T addFloat<T>(T a, T b) where optional T : IFloat { return a + b; }
//CHECK:                                                     ^ no overload for '+'
T addFloatMember<T>(T a, T b) where optional T : IFloat { return a.add(b); }
//CHECK:                                                           ^^^ the constraint providing 'add' is optional
T subMarker<T>(T a, T b) where optional T : __BuiltinFloatingPointType { return a - b; }
//CHECK:                                                                          ^ no overload for '-'
T modInteger<T>(T a, T b) where optional T : __BuiltinIntegerType { return a % b; }
//CHECK:                                                                     ^ no overload for '%'
T mulInt64<T>(T a, T b) where optional T : __BuiltinInt64Type { return a * b; }
//CHECK:                                                                 ^ no overload for '*'
T plusUser<T>(T a, T b) where optional T : IB { return a.plus(b); }
//CHECK:                                                 ^^^^ the constraint providing 'plus' is optional
int makeAssoc<T>(T t) where optional T : IBar { return t.make().get(); }
//CHECK:                                                 ^^^^ the constraint providing 'make' is optional

// An associated type's conformance is looked up through the constraint too, so the direct form
// needs the check for it as well.
int getAssoc<T>(T.A a) where optional T : IFoo { return a.get(); }
//CHECK:                                                  ^^^ the constraint providing 'get' is optional

// The same holds when the inherited conformance is needed to call a generic function.
U twice<U : IArithmetic>(U x) { return x.add(x); }
T twiceFloat<T>(T a) where optional T : IFloat { return twice(a); }
//CHECK:                                                     ^ type argument doesn't conform to interface

// Only a check of the optional constraint itself counts, because the operator would be looked
// up through the `T : IFloat` witness, which a check of `IArithmetic` does not establish. (The
// condition `T is IArithmetic` itself is folded to `true` here, a known limitation.)
T divGuardedOnSuper<T>(T a, T b) where optional T : IFloat
{
    if (T is IArithmetic)
        return a / b;
//CHECK:         ^ no overload for '/'
    return a;
}

// An initializer is looked up through the constraint as well, so `T(1)` and `(T)1` need the same
// check, whether the constraint names `IArithmetic` (which declares `__init(int)`) or inherits it.
T makeDirect<T>(T a) where optional T : IArithmetic { return T(1); }
//CHECK:                                                     ^ expected a function
T castDirect<T>(T a) where optional T : IArithmetic { return (T)1; }
//CHECK:                                                      ^ expected a function
T makeFloat<T>(T a) where optional T : IFloat { return T(1); }
//CHECK:                                               ^ expected a function
T castFloat<T>(T a) where optional T : IFloat { return (T)1; }
//CHECK:                                                ^ expected a function

// Two optional constraints that both lead to `IArithmetic`: the facet keeps the proof through the
// first one written, so only a check of that constraint makes `add` usable. This is a known
// limitation; writing the constraints in the other order makes this body compile.
T addTwoOptional<T>(T a, T b) where optional T : IFloat where optional T : IInteger
{
    if (T is IInteger)
        return a.add(b);
//CHECK:         ^^^ the constraint providing 'add' is optional
    return a;
}

// Extensions are applied when a type's conformances are computed, with no enclosing statements,
// so an extension that needs a conformance an optional constraint only inherits never applies to
// `T`, even inside `if (T is IFloat)`. Naming the extension's interface directly in an optional
// constraint behaves the same way.
interface IOne { int one(); }
extension<U : IArithmetic> U : IOne { int one() { return 1; } }
int oneIfFloat<T>(T t) where optional T : IFloat
{
    if (T is IFloat)
        return t.one();
//CHECK:         ^^^ member not found
    return 0;
}
