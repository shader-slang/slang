//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Receivers that have no instance form for `__fwd_diff(obj.method)` / `__bwd_diff(obj.method)`.

struct Accumulator : IDifferentiable
{
    float total;

    [Differentiable]
    [mutating]
    float add(float x)
    {
        total += x;
        return x * x;
    }
}

struct Handle : IDifferentiablePtrType
{
    typealias Differential = Handle;
    uint offset;

    [Differentiable]
    float square(float x) { return x * x; }
}

struct Counter
{
    float count;

    [Differentiable]
    [mutating]
    [NoDiffThis]
    float step(float x)
    {
        count += detach(x);
        return x * x;
    }
}

Counter makeCounter() { Counter c; c.count = 0.0; return c; }

void test()
{
    Accumulator a;
    a.total = 0.0;
    __fwd_diff(a.add)(diffPair(1.0, 1.0));
/*CHECK:
    ^^^^^^^^^^ cannot differentiate a mutating method with a differentiable 'this' through an instance
    ^^^^^^^^^^ 'add' is [mutating] and its 'this' is differentiable, so its derivative takes 'this' as an 'inout DifferentialPair<Accumulator>', which differentiating it through an instance does not support
*/

    Handle h;
    h.offset = 0;
    __fwd_diff(h.square)(diffPair(1.0, 1.0));
/*CHECK:
    ^^^^^^^^^^ cannot differentiate a method of a differentiable pointer type through an instance
    ^^^^^^^^^^ the derivative of 'square' takes 'this' as a 'DifferentialPtrPair<Handle>', which differentiating it through an instance does not support
*/

    __fwd_diff(makeCounter().step)(diffPair(1.0, 1.0));
//CHECK:                     ^^^^ mutating method cannot be called on immutable value
//CHECK:                     ^^^^ mutating method 'step' cannot be called on an immutable value

}

// As for a plain call, a `[mutating]` method on an `in` parameter only warns.
void stepInParameter(Counter c)
{
    __fwd_diff(c.step)(diffPair(1.0, 1.0));
//CHECK:         ^^^^ mutating method called on `in` parameter
//CHECK:         ^^^^ mutating method 'step' called on `in` parameter 'c'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
}
