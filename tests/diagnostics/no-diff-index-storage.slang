//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

void observeNonDifferentiableValue(float value) {}

[ForwardDifferentiable]
float readNoDiffIndex(float[2] values)
{
    // `no_diff` at the start of an expression statement must remain expression syntax rather than
    // being consumed as a declaration modifier on the statement.
    no_diff observeNonDifferentiableValue(values[0]);

    // A `no_diff` subscript remains valid when its value is read.
    return no_diff(values[0]);
}

struct GetSetSubscript
{
    float backing;

    __subscript(int index) -> float
    {
        [nonmutating]
        get { return backing; }

        [nonmutating]
        set {}
    }
}

[ForwardDifferentiable]
float rejectWritingNoDiffIndex(float[2] values)
{
    // Lowering materializes the subscript value before detaching its derivative, so the wrapper
    // cannot provide storage for a write.
    no_diff(values[0]) = 1.0;
//CHECK:               ^ left of '=' is not an l-value
//CHECK:               ^ left of '=' is not an l-value.
    return values[0];
}

[ForwardDifferentiable]
float rejectWritingNoDiffAbstractStorage(GetSetSubscript storage)
{
    // A user-defined getter/setter subscript is an extended l-value before it is wrapped. The
    // wrapper still materializes its value and must be rejected before BoundStorage lowering.
    no_diff(storage[0]) = 2.0;
//CHECK:                ^ left of '=' is not an l-value
//CHECK:                ^ left of '=' is not an l-value.

    // Reading the same wrapped subscript remains valid.
    return no_diff(storage[0]);
}
