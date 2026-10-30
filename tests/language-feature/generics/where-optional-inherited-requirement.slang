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

// The same holds when the inherited conformance is needed to call a generic function.
U twice<U : IArithmetic>(U x) { return x.add(x); }
T twiceFloat<T>(T a) where optional T : IFloat { return twice(a); }
//CHECK:                                                     ^ type argument doesn't conform to interface

// Only a check of the optional constraint itself counts: `T is IArithmetic` does not show
// that the `T : IFloat` witness the operator would be looked up through exists.
T divGuardedOnSuper<T>(T a, T b) where optional T : IFloat
{
    if (T is IArithmetic)
        return a / b;
//CHECK:         ^ no overload for '/'
    return a;
}
