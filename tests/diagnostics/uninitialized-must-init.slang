//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain

// Test must-init (definite assignment) warning 41035.
// These cases involve variables that are conditionally initialized —
// some control flow path has a store, but not all paths do.
//
// Note: simple scalar variables with conditional stores may be optimized
// away (dead-code eliminated or folded) before the uninit check runs.
// Tests here use structs, out params, or other patterns that survive
// optimization.

struct Data
{
    float value;
};

// Struct with conditional field init — survives optimization
float struct_conditional(bool b)
{
    Data d;
    if (b)
        d.value = 1.0;
    return d.value;
//CHK: E41035
//CHK: variable 'd' may be uninitialized on some paths
}

// Out parameter in conditional path
void set_val(out int v) { v = 42; }

int out_param_conditional(bool b)
{
    int x;
    if (b)
        set_val(x);
    return x;
//CHK: E41035
//CHK: variable 'x' may be uninitialized on some paths
}

// Conditional via out param, only one variable
int out_param_conditional_only(bool b)
{
    int y;
    if (b)
        set_val(y);
    return y;
//CHK: E41035
//CHK: variable 'y' may be uninitialized on some paths
}

// Loop-initialized variable: should NOT warn
// (store is unconditional within the loop body)
void loop_init_no_warn()
{
    Data d;
    for (int i = 0; i < 4; i++)
        d.value = float(i);
    float x = d.value;
}

// Struct unconditionally initialized: should NOT warn
float struct_unconditional()
{
    Data d;
    d.value = 1.0;
    return d.value;
}

// Switch fallthrough: variable initialized in case 0 but used in case 1
// which can be entered directly without going through case 0.
// Regression test for https://github.com/shader-slang/slang/issues/9601
float switch_fallthrough(int val)
{
    Data d;
    switch (val)
    {
    case 0:
        d.value = 100.0;
        // falls through to case 1
    case 1:
        return d.value;
//CHK: E41035
//CHK: variable 'd' may be uninitialized on some paths
    default:
        break;
    }
    return -1;
}

bool init_out(out int v) { v = 1; return true; }

// Short-circuit '&&': 'x' is initialized in the first operand and only used in
// the second, which is reached only when the first ran. The '&&' lowering creates
// a merge block whose phi carries a constant 'false' on the short-circuit edge, so
// the flat CFG admits an infeasible store-free path to the use. The must-init
// analysis prunes that edge (getInfeasibleBranchFromPredecessor) and must NOT warn.
int short_circuit_and_no_warn()
{
    int x;
    if (init_out(x) && x > 0)
        return x;
    return 0;
}

// Short-circuit '||': same idea via the negated-first-operand edge. Must NOT warn.
int short_circuit_or_no_warn()
{
    int x;
    if (!init_out(x) || x > 0)
        return x;
    return 0;
}

// Element-wise array fill in a loop, then read: should NOT warn. Exercises a
// different CFG topology (array element stores) than the single struct-field
// loop_init_no_warn case above.
float array_loop_fill_no_warn()
{
    float arr[4];
    [ForceUnroll] for (int i = 0; i < 4; i++)
        arr[i] = float(i);
    float s = 0;
    [ForceUnroll] for (int i = 0; i < 4; i++)
        s += arr[i];
    return s;
}

// while-loop initialization: should NOT warn. Exercises a while-loop CFG shape.
float while_loop_init_no_warn()
{
    Data d;
    int i = 0;
    while (i < 4)
    {
        d.value = float(i);
        i++;
    }
    return d.value;
}

// Regression tests for https://github.com/shader-slang/slang/issues/12545.
//
// `WaveReadLaneFirst()` broadcasts from the first active lane -- the same lane for which
// `WaveIsFirstLane()` is true. So a store inside `if (WaveIsFirstLane()) { ... }`, read back
// only via `WaveReadLaneFirst()`, is not actually reachable while uninitialized: the elected
// lane is guaranteed to have executed the store. These four cases pin down exactly how narrow
// that relaxation is -- it must not spill over into any of the neighboring shapes that are
// still genuine must-init bugs.

RWByteAddressBuffer gWaveCounterBuffer;

// The exact shape from the issue's repro: guarded store, broadcast read. Must NOT warn.
uint wave_elect_broadcast_no_warn(uint2 dispatchThreadID, bool bIsAlive)
{
    uint nBaseIndex;
    if (WaveIsFirstLane())
    {
        gWaveCounterBuffer.InterlockedAdd(0, WaveActiveCountBits(bIsAlive), nBaseIndex);
    }
    return WaveReadLaneFirst(nBaseIndex) + WavePrefixCountBits(bIsAlive);
}

// Same shape, but broadcasting a vector -- exercises the `vector<T,N> WaveReadLaneFirst`
// overload rather than the scalar one, locking in that all overloads carry the
// `[KnownBuiltin(...)]` annotation the relaxation relies on to recognize the call. Must NOT
// warn.
uint2 wave_elect_broadcast_vector_no_warn(uint2 dispatchThreadID)
{
    uint2 nBaseIndex;
    if (WaveIsFirstLane())
    {
        nBaseIndex = dispatchThreadID;
    }
    return WaveReadLaneFirst(nBaseIndex);
}

// Guard is negated: the store executes on the *non*-elected lanes, so the lane that
// `WaveReadLaneFirst` broadcasts from (the elected lane) never executed it. A genuine
// must-init bug -- must still warn.
uint wave_elect_negated_guard_warn(bool bIsAlive)
{
    uint nBaseIndex;
    if (!WaveIsFirstLane())
    {
        gWaveCounterBuffer.InterlockedAdd(0, WaveActiveCountBits(bIsAlive), nBaseIndex);
    }
    return WaveReadLaneFirst(nBaseIndex) + WavePrefixCountBits(bIsAlive);
//CHK: E41035
//CHK: variable 'nBaseIndex' may be uninitialized on some paths
}

// Same guarded store as the no-warn case above, but read back directly instead of through
// `WaveReadLaneFirst()`. Without the broadcast, a non-elected lane really can observe
// `nBaseIndex` uninitialized -- must still warn.
uint wave_elect_plain_read_warn(bool bIsAlive)
{
    uint nBaseIndex;
    if (WaveIsFirstLane())
    {
        gWaveCounterBuffer.InterlockedAdd(0, WaveActiveCountBits(bIsAlive), nBaseIndex);
    }
    return nBaseIndex + WavePrefixCountBits(bIsAlive);
//CHK: E41035
//CHK: variable 'nBaseIndex' may be uninitialized on some paths
}

// The store is nested inside a further conditional *within* the `WaveIsFirstLane()` guard, so
// even the elected lane is not guaranteed to have executed it (e.g. when `bIsAlive` is false
// on the elected lane). This is exactly the shape that requires the relaxation to check
// reachability scoped to the guard's own sub-CFG rather than whole-function dominance -- must
// still warn.
uint wave_elect_nested_conditional_store_warn(bool bIsAlive)
{
    uint nBaseIndex;
    if (WaveIsFirstLane())
    {
        if (bIsAlive)
        {
            gWaveCounterBuffer.InterlockedAdd(0, WaveActiveCountBits(bIsAlive), nBaseIndex);
        }
    }
    return WaveReadLaneFirst(nBaseIndex) + WavePrefixCountBits(bIsAlive);
//CHK: E41035
//CHK: variable 'nBaseIndex' may be uninitialized on some paths
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
}
