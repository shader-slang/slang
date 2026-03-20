// param-mutation.slang

// When a parameter is passed to `in`, it is mutable but generates a warning as it probably (?)
// isn't what the programmer intended.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

struct MutatingStruct
{
    [mutating] void setValue(int value) { m_value = value; }
    int m_value;
};

int doThing(MutatingStruct s, int v)
{
    // Should generate a warning.
    s.setValue(v + 1);
//diag:       ^ mutating method called on `in` parameter
//diag:       ^ mutating method 'setValue' called on `in` parameter 's'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    return s.m_value;
}

// For non-copyable types (such as HitObject or NonCopyableStruct declared below), if passed as as `in`
// should produce an error.

[__NonCopyableType]
struct NonCopyableStruct
{
    [mutating] void setValue(int value) { m_value = value; }
    int m_value;
};

int doThing2(NonCopyableStruct s, int v)
{
    s.setValue(v + 1);
//diag:       ^ mutating method called on `in` parameter
//diag:       ^ mutating method 'setValue' called on `in` parameter 's'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    return s.m_value;
}

