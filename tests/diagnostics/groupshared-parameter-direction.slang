//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Regression guard for https://github.com/shader-slang/slang/issues/10641.
//
// A `groupshared` parameter names one thread-group-shared storage location and is therefore passed
// by reference, so the copy-direction modifiers `in`/`out`/`inout` are not applicable and are a
// compile error on a `groupshared` parameter (error 30707). The read-only spelling is
// `const groupshared`; `__constref`, which asks for a borrow, is rejected (error 30712). The bare
// and `const` spellings must still compile with no diagnostic (enforced by the exhaustive
// diagnostic-test mode -- any stray diagnostic on those lines would fail the test).
//
// The `//diag:` annotations are caret-less substring matches, so each is matched against one of the
// produced diagnostics by message text regardless of line/column.

static const uint N = 8;

// --- Rejected: `in` / `out` / `inout` on a `groupshared` parameter ---

void badIn(uint tid, in groupshared uint s[N])
{
    s[tid] = tid;
}
//diag: cannot be used with 'groupshared' -- use bare

void badOut(uint tid, out groupshared uint s[N])
{
    s[tid] = tid;
}
//diag: cannot be used with 'groupshared' -- use bare

void badInOut(uint tid, inout groupshared uint s[N])
{
    s[tid] = tid;
}
//diag: cannot be used with 'groupshared' -- use bare

// --- Rejected: `__constref` on a `groupshared` parameter ---

uint badConstRef(uint tid, __constref groupshared uint s[N])
{
    return s[tid];
}
//diag: '__constref' cannot be used with 'groupshared' -- use 'const groupshared'

// --- Accepted: bare (read-write) and `const` (read-only) `groupshared` ---

void okReadWrite(uint tid, groupshared uint s[N])
{
    s[tid] = tid;
}

uint okConstReadOnly(uint tid, const groupshared uint s[N])
{
    return s[tid];
}
