//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

struct State : IDifferentiablePtrType
{
    typealias Differential = State;
    no_diff RWStructuredBuffer<float> values;
}

void requireConstantReceiver<F : IDifferentiableFunc<float, float>>(F function) {}
void requireMutatingReceiver<F : IDifferentiableMutatingFunc<float, float>>(inout F function) {}

void checkContracts(State state)
{
    var function = (float value)~ => { let captured = state; return value; };
    requireConstantReceiver(function);
    //CHECK: type argument
    requireMutatingReceiver(function);
    //CHECK: type argument

    var mutating = (float value)!~ => value;
    requireMutatingReceiver(mutating);
}
