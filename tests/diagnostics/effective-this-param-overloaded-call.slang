//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): tests/diagnostics/effective-this-param-overloaded-call.slang -no-codegen

// Receiver mode does not participate in overload viability or ranking. Overload resolution first
// selects a method from the ordinary parameter types; semantic checking then validates only that
// selected method's receiver against its effective mode.

void acceptFunction(functype(int) -> void functionValue) {}

struct RefBox
{
    static int sharedStorage;

    property int writable
    {
        [nonmutating]
        ref { return sharedStorage; }
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
        // Receiver validation skips the mutating overload that overload resolution did not select.
        return select(1.0);
    }

    [nonmutating]
    void selectAfterGenericApplication()
    {
        // Generic-application checking retains both candidates until the subsequent invocation
        // selects the integer overload and validates its receiver.
        selectGeneric<int>(1);
//CHECK:                  ^ mutating method cannot be called on immutable value
//CHECK:                  ^ mutating method 'selectGeneric' cannot be called on an immutable value
//CHECK:^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [nonmutating]
    void selectAfterUniqueGenericApplication()
    {
        // Generic-application checking selects the only declaration, but the enclosing invocation
        // remains responsible for validating that declaration's receiver exactly once.
        selectUniqueGeneric<int>(1);
//CHECK:                        ^ mutating method cannot be called on immutable value
//CHECK:                        ^ mutating method 'selectUniqueGeneric' cannot be called on an immutable value
//CHECK:^^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [nonmutating]
    void retainBareUniqueGenericApplication()
    {
        // Expression-statement checking finalizes and consumes the function-typed specialization,
        // even though the statement discards the resulting bound method. It must validate the
        // mutating receiver just like an invocation or conversion to a concrete function type.
        selectUniqueGeneric<int>;
//CHECK:^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:^^^^^^^^^^^^^^^^^^^ mutating method 'selectUniqueGeneric' cannot be called on an immutable value
//CHECK:^^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    }

    [nonmutating]
    void selectUniqueGenericThroughFunctionConversion()
    {
        // Contextual function-type coercion consumes the retained specialization as a callable
        // value for `acceptFunction`, so conversion checking must validate the receiver there.
        acceptFunction(selectUniqueGeneric<int>);
//CHECK:               ^^^^^^^^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:               ^^^^^^^^^^^^^^^^^^^ mutating method 'selectUniqueGeneric' cannot be called on an immutable value
//CHECK:               ^^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member

        // Semantic checking propagates the selected callable through parentheses, which remain
        // transparent to conversion checking.
        acceptFunction((selectUniqueGeneric<int>));
//CHECK:               ^ mutating method cannot be called on immutable value
//CHECK:               ^ mutating method 'selectUniqueGeneric' cannot be called on an immutable value
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
        // Checking an instance-member expression retains the selected inner call, and evaluating
        // that expression evaluates the call. Final checking of the outer member expression must
        // therefore validate the inner call's receiver outside a lambda.
        return mutateAndReturn(0).instanceValue;
//CHECK:                      ^ mutating method cannot be called on immutable value
//CHECK:                      ^ mutating method 'mutateAndReturn'
//CHECK:       ^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [nonmutating]
    float evaluateSelectedCallBelowBuiltinOperator()
    {
        // Converting `+` to a builtin operator must retain the evaluated calls in its arguments.
        // Final checking of the outer swizzle must still validate the inner call's receiver.
        return (mutateAndReturnVector() + 1.0).x;
//CHECK:                             ^ mutating method cannot be called on immutable value
//CHECK:                             ^ mutating method 'mutateAndReturnVector'
//CHECK:        ^^^^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [nonmutating]
    int evaluateSelectedCallBelowMakeRef()
    {
        // Coercion to `Ref<int>` wraps the selected property path in a `MakeRefExpr`. Final checking
        // of the outer member access must still reach the inner mutating call.
        return takeRef(mutateAndReturnBox().writable).field;
//CHECK:                                 ^ mutating method cannot be called on immutable value
//CHECK:                                 ^ mutating method 'mutateAndReturnBox'
//CHECK:               ^^^^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
    }

    [mutating]
    int checkCapturedSelectedCallBelowInstanceMember()
    {
        // The enclosing method has a writable receiver, so checking the inner call immediately
        // would accept it. Checking the member expression retains the selected call without
        // finalizing its receiver validation. After capture rewriting makes `this` immutable, final
        // selected-expression validation rejects the call.
        let read = () => mutateAndReturn(0).instanceValue;
//CHECK:                                ^ mutating method cannot be called on immutable value
//CHECK:                                ^ mutating method 'mutateAndReturn'
//CHECK:                 ^^^^^^^^^^^^^^^ attempting to assign to a const variable or immutable member
        return read();
    }
};
