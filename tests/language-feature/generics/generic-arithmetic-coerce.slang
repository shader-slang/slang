//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type -cpu

// Regression test for the `_coerce` fast-rejection of opaque generic type
// parameters into scalar builtins (overload-resolution perf optimization).
//
// Note that nothing here observes the short-circuit itself: this test's
// contract is that the optimization is *behavior-preserving* — it pins down
// every conversion shape that sits just outside the rejection's boundary and
// must keep type-checking (over-rejection breaks them). The perf effect is
// validated by the compile-perf suite, not here; deleting the fast path
// entirely would still pass this test, by design.
//
// - `poly` exercises the short-circuited path: ranking `a*a + b*a + ...` on a
//   constrained generic `T` rejects the many concrete builtin scalar operator
//   overloads (`operator*(float,float)` etc.) by failing to coerce
//   `T` -> scalar at an implicit site.
// - `broadcast` / `matBroadcast` pin the target-type boundary: a scalar
//   generic `T` still broadcasts into `vector<T,3>` / `matrix<T,2,2>` via the
//   element constructor (toType is not a scalar, so the rejection must not
//   fire).
// - `wrapGeneric` pins the aggregate boundary: a struct exposing a generic
//   `__init<U : IArithmetic>(U)` must still accept an opaque `T` (a wider
//   rejection keyed on "any DeclRefType source" or "any target" would break
//   it).
// - `explicitCast` pins the coercion-site boundary: at an *explicit* cast
//   site, nested conversions are allowed, so `float(t)` under `where T == int`
//   legitimately succeeds through `float.__init(int)` plus the cost-0
//   equality-witness conversion `T -> int`; the rejection must not fire for
//   explicit sites.
// - `polyPack` pins the variadic-generics neighborhood: pack elements flow
//   through the same constrained-generic arithmetic (`poly`) whose checking
//   hits the fast rejection, at mixed scalar instantiations. (A *bare* pack
//   parameter as a coercion source — the `GenericTypeParamDeclBase` pack case
//   the C++ comment argues about — was not reachable from surface code in
//   instrumented probes; pack elements are checked as `each T` types that
//   never enter the initializer search with a scalar target. This case keeps
//   the rejection honest around pack expansion regardless.)

T poly<T : IArithmetic>(T a, T b)
{
    return a * a + b * a + a + b;
}

vector<T, 3> broadcast<T : IArithmetic>(T x)
{
    return vector<T, 3>(x); // scalar generic T -> vector<T,3> broadcast
}

matrix<T, 2, 2> matBroadcast<T : IArithmetic>(T x)
{
    return matrix<T, 2, 2>(x); // scalar generic T -> matrix<T,2,2> broadcast
}

struct Wrapped
{
    int tag;
    __init<U : IArithmetic>(U u) { tag = 1; }
}

int wrapGeneric<T : IArithmetic>(T t)
{
    // Aggregate target with a generic initializer must accept an opaque T.
    let w = Wrapped(t);
    return w.tag;
}

float explicitCast<T>(T t) where T == int
{
    // Explicit cast of an opaque T to a scalar builtin, valid only through the
    // nested equality-witness conversion T -> int.
    return float(t);
}

void accept<each T>(expand each T v) {}

int writePoly<T : __BuiltinArithmeticType>(inout int offset, T value)
{
    // Routes one pack element through the same constrained-generic arithmetic
    // as `poly` and records the result.
    outputBuffer[offset++] = __intCast<int>(poly(value, value));
    return 0;
}

void polyPack<each T : __BuiltinArithmeticType>(inout int offset, expand each T args)
{
    accept(expand writePoly(offset, each args));
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = poly<int>(3, 4);
    // CHECK: 28

    outputBuffer[1] = poly<int>(2, 5);
    // CHECK: 21

    // Distinct arguments from the poly<int> calls above, so a bug that routed
    // this call to the wrong instantiation would be observable.
    outputBuffer[2] = int(poly<uint>(5u, 2u));
    // CHECK: 42

    int3 v = broadcast<int>(7);
    outputBuffer[3] = v.x;
    // CHECK: 7
    outputBuffer[4] = v.y + v.z;
    // CHECK: 14

    outputBuffer[5] = poly<int>(0, 6);
    // CHECK: 6

    int2x2 m = matBroadcast<int>(5);
    outputBuffer[6] = m[0][0] + m[1][1];
    // CHECK: 10

    outputBuffer[7] = wrapGeneric<int>(3);
    // CHECK: 1

    outputBuffer[8] = int(explicitCast<int>(9));
    // CHECK: 9

    // poly(v, v) = 2*v*v + 2*v: 2 -> 12, 3u -> 24 (mixed int/uint pack).
    int offset = 9;
    polyPack(offset, 2, 3u);
    // CHECK: 12
    // CHECK: 24
}
