//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct Data
{
    float value;
};

// Check that unrelated guards cannot prove the conditional store precedes the load.
float differentConditions(bool first, bool second)
{
    Data data;
    if (first)
        data.value = 3.0;
    if (second)
        return data.value;
//CHECK:^^^^^^ E41035
//CHECK:^^^^^^ variable 'data' may be uninitialized on some paths
    return 0;
}

// Check that a repeated outer guard does not hide a path that skips the nested store.
float incompleteAssignment(bool condition, bool assigned)
{
    Data data;
    if (condition)
    {
        if (assigned)
            data.value = 3.0;
    }
    if (condition)
        return data.value;
//CHECK:^^^^^^ E41035
//CHECK:^^^^^^ variable 'data' may be uninitialized on some paths
    return 0;
}

// Check that changing the guard's value leaves the uninitialized load reported.
float changedCondition(bool condition)
{
    Data data;
    if (condition)
        data.value = 3.0;
    condition = !condition;
    if (condition)
        return data.value;
//CHECK:^^^^^^ E41035
//CHECK:^^^^^^ variable 'data' may be uninitialized on some paths
    return 0;
}

// Check that reloading a mutable guard after a write does not reuse its earlier value.
float reloadedCondition(inout bool condition)
{
    Data data;
    if (condition)
        data.value = 3.0;
    condition = !condition;
    if (condition)
        return data.value;
//CHECK:^^^^^^ E41035
//CHECK:^^^^^^ variable 'data' may be uninitialized on some paths
    return 0;
}

// Check that a loop-carried guard becoming true does not hide an uninitialized load.
void loopCondition(bool initialized, bool again, float* output)
{
    Data data;
    if (initialized)
        data.value = 3.0;
    bool condition = false;
    do
    {
        if (condition)
            output[0] = data.value;
//CHECK:              ^ E41035
//CHECK:              ^ variable 'data' may be uninitialized on some paths
        if (condition)
            output[1] = 2.0;
        condition = true;
    }
    while (again);
}

// Check that many unrelated repeated guards do not suppress a required warning.
void refinementBudget(bool initialized, uint flags, float* output)
{
    Data data;
    if (initialized)
        data.value = 3.0;
#define DECLARE_GUARD(N) bool c##N = (flags & (1u << N)) != 0;
#define USE_GUARD(N) if (c##N) output[N] += 1.0;
#define ALL_GUARDS(M) \
    M(0) M(1) M(2) M(3) M(4) M(5) M(6) M(7) \
    M(8) M(9) M(10) M(11) M(12) M(13) M(14) M(15)
    ALL_GUARDS(DECLARE_GUARD)
    ALL_GUARDS(USE_GUARD)
    ALL_GUARDS(USE_GUARD)
#undef ALL_GUARDS
#undef USE_GUARD
#undef DECLARE_GUARD
    output[16] = data.value;
//CHECK:       ^ E41035
//CHECK:       ^ variable 'data' may be uninitialized on some paths
}

[shader("compute")]
void main()
{
}
