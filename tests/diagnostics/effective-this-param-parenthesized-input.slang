//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Semantic checking tracks the same storage through parentheses, fields, built-in aggregate
// indexing, and inherited base-struct projections. Calling a mutating method through `(input)` must
// therefore apply the same by-value input restriction as calling it through `input` directly.
// Checking an explicit dereference or `ref` accessor instead starts an independent storage path and
// deliberately ends the path back to the parameter.

// The test intentionally uses concrete struct inheritance below. Keep this diagnostic focused on
// input storage rather than the separate warning about that experimental feature.
#pragma warning(disable:30816)

[__NonCopyableType]
struct NonCopyableValue
{
    [mutating]
    void mutate()
    {}
}

[__NonCopyableType]
struct NonCopyableContainer
{
    NonCopyableValue member;
    NonCopyableValue elements[1];
}

[__NonCopyableType]
struct NonCopyableBase
{
    [mutating]
    void mutateBase()
    {}
}

[__NonCopyableType]
struct NonCopyableDerived : NonCopyableBase
{}

static NonCopyableValue independentStorage;

[__NonCopyableType]
struct IndependentRefStorage
{
    property NonCopyableValue propertyValue
    {
        [nonmutating]
        ref { return independentStorage; }
    }

    __subscript(int index) -> NonCopyableValue
    {
        [nonmutating]
        ref { return independentStorage; }
    }
}

void rejectDirectInput(NonCopyableValue input)
{
    input.mutate();
//CHECK:        ^ mutating method called on `in` parameter
//CHECK:        ^ mutating method 'mutate' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
}

void rejectParenthesizedInput(NonCopyableValue input)
{
    (input).mutate();
//CHECK:          ^ mutating method called on `in` parameter
//CHECK:          ^ mutating method 'mutate' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
}

void rejectMemberOfInput(NonCopyableContainer input)
{
    input.member.mutate();
//CHECK:               ^ mutating method called on `in` parameter
//CHECK:               ^ mutating method 'mutate' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
}

void rejectIndexedMemberOfInput(NonCopyableContainer input)
{
    input.elements[0].mutate();
//CHECK:                    ^ mutating method called on `in` parameter
//CHECK:                    ^ mutating method 'mutate' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
}

void rejectInheritedBaseOfInput(NonCopyableDerived input)
{
    input.mutateBase();
//CHECK:            ^ mutating method called on `in` parameter
//CHECK:            ^ mutating method 'mutateBase' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
}

void acceptPointeeMutation(Ptr<NonCopyableValue> input)
{
    (*input).mutate();
}

void acceptIndependentPropertyStorage(IndependentRefStorage input)
{
    input.propertyValue.mutate();
}

void acceptIndependentSubscriptStorage(IndependentRefStorage input)
{
    input[0].mutate();
}
