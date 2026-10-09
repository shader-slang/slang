// param-mutation.slang

// When a parameter is passed to `in`, it is mutable but generates a warning as it probably (?)
// isn't what the programmer intended.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

struct MutatingStruct
{
    [mutating] void setValue(int value) { m_value = value; }
    int m_value;
};

MutatingStruct externalMutationTarget;

struct RefAccessorContainer
{
    property MutatingStruct mutatingRef
    {
        [nonmutating] set {}
        [mutating] ref { return externalMutationTarget; }
    }
};

void mutateThroughRefAccessor(RefAccessorContainer input)
{
    // This method call needs writable storage for the returned value. Because `mutatingRef` has no
    // getter, the read uses its `ref` accessor. The returned reference targets external storage,
    // but the `[mutating]` accessor itself still requires writable access to the local copy of
    // `input`.
    input.mutatingRef.setValue(2);
//diag:                       ^ mutating storage accessor called on `in` parameter
//diag:                       ^ mutating 'ref' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

struct ExternalSubscriptReference
{
    __subscript(int index) -> MutatingStruct
    {
        [nonmutating]
        ref { return externalMutationTarget; }
    }
};

[__NonCopyableType]
struct NonCopyableExternalSubscriptReference
{
    __subscript(int index) -> MutatingStruct
    {
        [nonmutating]
        ref { return externalMutationTarget; }
    }
};

void mutateInOut(inout MutatingStruct value)
{
    value.setValue(3);
}

void mutateRef(__ref MutatingStruct value)
{
    value.setValue(4);
}

void mutateExternalThroughSubscript(
    ExternalSubscriptReference input,
    NonCopyableExternalSubscriptReference nonCopyableInput)
{
    // Neither subscript aliases its receiver, so these calls mutate the global target rather than
    // the `in` parameters.
    input[0].setValue(1);
    nonCopyableInput[0].setValue(2);
    mutateInOut(input[0]);
    mutateInOut(nonCopyableInput[0]);
    mutateRef(input[0]);
    mutateRef(nonCopyableInput[0]);
}

int doThing(MutatingStruct s, int v)
{
    // Should generate a warning.
    s.setValue(v + 1);
//diag:       ^ mutating method called on `in` parameter
//diag:       ^ mutating method 'setValue' called on `in` parameter 's'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    return s.m_value;
}

struct MutatingBase
{
    [mutating] void setValue(int value) {}
    [mutating] void setValue(float value) {}
};

#pragma warning(disable: 30816) // Struct inheritance is required to exercise a super-type cast.

struct MutatingDerived : MutatingBase
{};

int callInheritedOverload(MutatingDerived value)
{
    // Calling `value.setValue(1)` implicitly converts `value` from `MutatingDerived` to
    // `MutatingBase`; receiver validation must still trace the call back to the original `in`
    // parameter.
    value.setValue(1);
//diag:           ^ mutating method called on `in` parameter
//diag:           ^ mutating method 'setValue' called on `in` parameter 'value'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    return 0;
}

int callParenthesizedInput(MutatingStruct input)
{
    (input).setValue(1);
//diag:             ^ mutating method called on `in` parameter
//diag:             ^ mutating method 'setValue' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    return 0;
}

extension float2
{
    [mutating] void clear() {}
}

int callInputSwizzle(float4 input)
{
    input.xy.clear();
//diag:           ^ mutating method called on `in` parameter
//diag:           ^ mutating method 'clear' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    return 0;
}

int callInputMatrixSwizzle(float2x2 input)
{
    input._m00_m01.clear();
//diag:                 ^ mutating method called on `in` parameter
//diag:                 ^ mutating method 'clear' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    return 0;
}

interface IMutatingRequirement
{
    [mutating]
    void mutate();
}

struct MutatingRequirementImpl : IMutatingRequirement
{
    [mutating]
    void mutate() {}
}

void callMutatingRequirement<T : IMutatingRequirement>(T input)
{
    input.mutate();
//diag:         ^ mutating method called on `in` parameter
//diag:         ^ mutating method 'mutate' called on `in` parameter 'input'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
}

void instantiateMutatingRequirement()
{
    MutatingRequirementImpl value;
    callMutatingRequirement(value);
}

// For non-copyable types (such as HitObject or NonCopyableStruct declared below), if passed as as `in`
// should produce an error.

[__NonCopyableType]
struct NonCopyableStruct
{
    [mutating] void setValue(int value) { m_value = value; }
    int m_value;
};

[__NonCopyableType]
struct NonCopyableSetterContainer
{
    MutatingStruct backing;

    property MutatingStruct item
    {
        [nonmutating]
        get { return backing; }

        [mutating]
        set { backing = newValue; }
    }
};

void mutatePropertyOnNonCopyableInput(NonCopyableSetterContainer input)
{
    // The method call updates a temporary returned by `get` and writes the complete value back
    // through `set`. The mutating setter cannot use the non-copyable `in` parameter as its
    // receiver.
    input.item.setValue(1);
//diag:                ^ mutating storage accessor called on `in` parameter
//diag:                ^ mutating 'set' accessor called on `in` parameter 'input'; changes will not be visible to the caller. Copy the parameter into a local variable if this behavior is intended
}

int doThing2(NonCopyableStruct s, int v)
{
    s.setValue(v + 1);
//diag:       ^ mutating method called on `in` parameter
//diag:       ^ mutating method 'setValue' called on `in` parameter 's'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    return s.m_value;
}

