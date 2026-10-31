//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-experimental-feature

// Regression guard for https://github.com/shader-slang/slang/issues/10641.
//
// A `groupshared` parameter names one thread-group-shared storage location and is therefore passed
// by reference, so the copy-direction modifiers `in`/`out`/`inout` are not applicable and are a
// compile error on a `groupshared` parameter (error 30707). The read-only spelling is
// `const groupshared`; `__borrow` and its alias `__constref`, which ask for a borrow, are rejected
// (error 30712). `groupshared` names the address space of the reference, so an explicit `__ref`,
// `__ref_readonly`, or `__ref_writeonly` picks its access; without one, `groupshared` implies
// `__ref`, or `__ref_readonly` with `const`. Those spellings must compile with no diagnostic
// (enforced by the exhaustive diagnostic-test mode -- any stray diagnostic on those lines would
// fail the test).
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

// --- Rejected: `__borrow` / `__constref` on a `groupshared` parameter ---

uint badBorrow(uint tid, __borrow groupshared uint s[N])
{
    return s[tid];
}
//diag: '__borrow' cannot be used with 'groupshared' -- use 'const groupshared'

uint badConstRef(uint tid, __constref groupshared uint s[N])
{
    return s[tid];
}
//diag: '__constref' cannot be used with 'groupshared' -- use 'const groupshared'

// A doubled `__constref` reports the duplicate once and E30712 once. The parameter is still lowered
// as a reference, because `groupshared` takes precedence over a borrow, so the call below checks.
uint badDoubleConstRef(uint tid, __constref __constref groupshared uint s[N])
//diag:                                     ^^^^^^^^^^ duplicate modifier
//diag:                                     ^^^^^^^^^^ modifier '__constref' is redundant or conflicting with existing modifier '__constref'
{
    return s[tid];
}
//diag: '__constref' cannot be used with 'groupshared' -- use 'const groupshared'

groupshared uint doubleConstRefStorage[N];

uint callDoubleConstRef(uint tid)
{
    return badDoubleConstRef(tid, doubleConstRefStorage);
}

// --- Rejected: `const` with a reference-access keyword, as on any parameter ---

uint badConstRefReadOnly(uint tid, const __ref_readonly groupshared uint s[N])
//diag:                                  ^^^^^^^^^^^^^^ duplicate modifier
//diag:                                  ^^^^^^^^^^^^^^ modifier '__ref_readonly' is redundant or conflicting with existing modifier 'const'
{
    return s[tid];
}

// --- Accepted: an explicit reference keyword picks the access of a `groupshared` reference ---

void okRefReadWrite(uint tid, __ref groupshared uint s[N])
{
    s[tid] = tid;
}

uint okRefReadOnly(uint tid, __ref_readonly groupshared uint s[N])
{
    return s[tid];
}

void okRefWriteOnly(uint tid, groupshared __ref_writeonly uint s[N])
{
    s[tid] = tid;
}

// --- Accepted: bare (implies `__ref`) and `const` (implies `__ref_readonly`) `groupshared` ---

void okReadWrite(uint tid, groupshared uint s[N])
{
    s[tid] = tid;
}

uint okConstReadOnly(uint tid, const groupshared uint s[N])
{
    return s[tid];
}
