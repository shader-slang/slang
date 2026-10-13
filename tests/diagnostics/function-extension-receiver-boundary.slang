//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen -experimental-feature

// An explicit `this` expression stops at the nearest callable boundary. Free functions, static
// methods, and extensions of free functions have no instance receiver to supply, so receiver-mode
// modifiers are also invalid on those function extensions.

void ordinaryFreeFunction()
{
    let invalidReceiver = this;
//CHECK:                  ^^^^ 'this' used in callable without an instance receiver
//CHECK:                  ^^^^ 'this' expression cannot be used in a callable that has no instance receiver
}

struct Value
{
    static void staticMethod()
    {
        let invalidReceiver = this;
//CHECK:                      ^^^^ 'this' used in callable without an instance receiver
//CHECK:                      ^^^^ 'this' expression cannot be used in a callable that has no instance receiver
    }
};

[Differentiable]
float square(float value)
{
    return value * value;
}

struct BackwardContext
{
    void operator()(out float derivative, float outputDerivative)
    {
        derivative = outputDerivative;
    }
};

[mutating]
/*CHECK:
 ^^^^^^^^ receiver-mode specifier requires an instance receiver
 ^^^^^^^^ receiver-mode specifier 'mutating' is only allowed on an instance member
*/
__func_extension __apply(square)(float value) -> Tuple<float, BackwardContext>
{
    let invalidReceiver = this;
//CHECK:                  ^^^^ 'this' used in callable without an instance receiver
//CHECK:                  ^^^^ 'this' expression cannot be used in a callable that has no instance receiver
    return makeTuple(value, BackwardContext());
}
