//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Each construct below wraps a mutating receiver access in a larger checked expression.
// Selected-expression finalization at the enclosing expression must visit that access exactly once,
// so each programmer-written call has exactly one summary/detail diagnostic pair.

struct Predicate
{
    [mutating]
    bool evaluate()
    {
        return true;
    }

    [mutating]
    int readNumber()
    {
        return 0;
    }
}

bool checkShortCircuitOperand(in Predicate predicate)
{
    return true && predicate.evaluate();
//CHECK:                             ^ mutating method called on `in` parameter
//CHECK:                             ^ mutating method 'evaluate' called on `in` parameter 'predicate'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
//CHECK-NOT: mutating method
}

int checkBuiltinOperand(in Predicate predicate)
{
    return predicate.readNumber() + 1;
//CHECK:                       ^ mutating method called on `in` parameter
//CHECK:                       ^ mutating method 'readNumber' called on `in` parameter 'predicate'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
//CHECK-NOT: mutating method
}

bool checkCaseExpression(in Predicate predicate, bool selector)
{
    switch (selector)
    {
    case predicate.evaluate():
//CHECK:                   ^ mutating method called on `in` parameter
//CHECK:                   ^ mutating method 'evaluate' called on `in` parameter 'predicate'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
//CHECK:                   ^ expression does not evaluate to a compile-time constant
//CHECK-NOT: mutating method
        return true;
    }
    return false;
}

__generic<let value : int>
void useConstant()
{}

void checkGenericValueArgument(in Predicate predicate)
{
    // Specialization consumes this generic value argument even while the enclosing generic
    // application is checked as a potentially unevaluated callable. The selected receiver must
    // therefore be finalized exactly once.
    useConstant<predicate.readNumber()>();
//CHECK:                            ^ mutating method called on `in` parameter
//CHECK:                            ^ mutating method 'readNumber' called on `in` parameter 'predicate'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
//CHECK:                            ^ expression does not evaluate to a compile-time constant
//CHECK-NOT: mutating method
}
