//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): tests/diagnostics/effective-this-param-overloaded-call.slang -no-codegen

// Receiver mode does not participate in overload viability or ranking. The ordinary parameter
// types select one method first; semantic checking then validates only that selected method's
// receiver against its effective mode.

void acceptFunction(functype(int) -> void functionValue) {}

struct RefBox
{
    int storage;

    property int writable
    {
        [nonmutating]
        ref { return storage; }
    }
}

struct RefResult
{
    int field;
}

RefResult takeRef(Ref<int> value) { return {}; }

struct Counter
{
    [mutating]
    int select(int value) { return value; }

    [nonmutating]
    int select(float value) { return int(value); }

    [mutating]
    void selectGeneric<T>(int value) {}

    [nonmutating]
    void selectGeneric<T>(float value) {}

    [mutating]
    void selectUniqueGeneric<T>(int value) {}

    [mutating]
    Counter mutateAndReturn(int value) { return this; }

    [mutating]
    float2 mutateAndReturnVector() { return 0.0; }

    [mutating]
    RefBox mutateAndReturnBox() { return {}; }

    static int staticValue;
    int instanceValue;

    [nonmutating]
    int selectAfterRanking()
    {
        // The exact integer match remains preferable to the nonmutating conversion candidate.
        return select(1);
//CHECK:             ^ mutating method cannot be called on immutable value
//CHECK:             ^ mutating method 'select' cannot be called on an immutable value
//CHECK:       ^^^^^^ attempting to assign to a const variable or immutable member
    }

    [nonmutating]
    int selectNonmutatingOverload()
    {
        // The unselected mutating overload produces no receiver diagnostic.
        return select(1.0);
    }

    [nonmutating]
    void selectAfterGenericApplication()
    {
        // Explicit generic application retains both declarations. The subsequent invocation
        // selects the integer overload before validating its receiver.
        selectGeneric<int>(1);
//CHECK:                  ^ mutating method cannot be called on immutable value
//CHECK:                  ^ mutating method 'selectGeneric' cannot be called on an immutable value
//CHECK:^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [nonmutating]
    void selectAfterUniqueGenericApplication()
    {
        // Generic application chooses the only declaration, but the enclosing invocation remains
        // responsible for validating its selected receiver exactly once.
        selectUniqueGeneric<int>(1);
//CHECK:                        ^ mutating method cannot be called on immutable value
//CHECK:                        ^ mutating method 'selectUniqueGeneric' cannot be called on an immutable value
//CHECK:^^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [nonmutating]
    void retainBareUniqueGenericApplication()
    {
        // Explicit generic application alone selects a specialization, not an evaluated use of
        // its receiver. A later invocation selects the evaluated callable path and validates the
        // receiver.
        selectUniqueGeneric<int>;
    }

    [nonmutating]
    void selectUniqueGenericThroughFunctionConversion()
    {
        // Exact function-type coercion materializes the retained specialization as an evaluated
        // callable path, so it must validate the receiver at this boundary.
        acceptFunction(selectUniqueGeneric<int>);
//CHECK:               ^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:               ^^^^^^^^^^^^^^^^^^^ mutating method 'selectUniqueGeneric' cannot be called on an immutable value
//CHECK:               ^^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member

        // Parentheses preserve the selected callable while remaining transparent to conversion.
        acceptFunction((selectUniqueGeneric<int>));
//CHECK:                ^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:                ^^^^^^^^^^^^^^^^^^^ mutating method 'selectUniqueGeneric' cannot be called on an immutable value
//CHECK:                ^^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [mutating]
    void checkCapturedImplicitThis()
    {
        // The enclosing method has a writable receiver, but a lambda captures `this` by value.
        // Receiver validation must observe the immutable closure field produced by capture
        // rewriting rather than the original writable `this` expression.
        let call = () => select(1);
//CHECK:                       ^ mutating method cannot be called on immutable value
//CHECK:                       ^ mutating method 'select' cannot be called on an immutable value
//CHECK:                 ^^^^^^ attempting to assign to a const variable or immutable member
        call();
    }

    [nonmutating]
    int ignoreSelectedCallBelowStaticMember()
    {
        // The selected inner call is only a qualifier for a static member, so it is not evaluated
        // and must not trigger a receiver diagnostic.
        return mutateAndReturn(0).staticValue;
    }

    [nonmutating]
    int evaluateSelectedCallBelowInstanceMember()
    {
        // An instance member retains and evaluates the selected inner call, so finalizing the
        // outer lookup must consume its pending receiver check even outside a lambda.
        return mutateAndReturn(0).instanceValue;
//CHECK:                      ^ mutating method cannot be called on immutable value
//CHECK:                      ^ mutating method 'mutateAndReturn'
//CHECK:       ^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [nonmutating]
    float evaluateSelectedCallBelowBuiltinOperator()
    {
        // Converting `+` to a builtin operator must retain the evaluated calls in its arguments.
        // Finalizing the outer swizzle then consumes the inner call's pending receiver check.
        return (mutateAndReturnVector() + 1.0).x;
//CHECK:                             ^ mutating method cannot be called on immutable value
//CHECK:                             ^ mutating method 'mutateAndReturnVector'
//CHECK:        ^^^^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [nonmutating]
    int evaluateSelectedCallBelowMakeRef()
    {
        // Coercion to `Ref<int>` wraps the selected property path in a `MakeRefExpr`.
        // Finalizing the outer member access must still reach the inner mutating call.
        return takeRef(mutateAndReturnBox().writable).field;
//CHECK:                                 ^ mutating method cannot be called on immutable value
//CHECK:                                 ^ mutating method 'mutateAndReturnBox'
//CHECK:               ^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [mutating]
    int checkCapturedSelectedCallBelowInstanceMember()
    {
        // The enclosing method has a writable receiver, so checking the inner call immediately
        // would accept it. The instance-member lookup instead retains the call and its pending
        // check observes the immutable receiver produced when the lambda captures `this`.
        let read = () => mutateAndReturn(0).instanceValue;
//CHECK:                                ^ mutating method cannot be called on immutable value
//CHECK:                                ^ mutating method 'mutateAndReturn'
//CHECK:                 ^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
        return read();
    }
};
