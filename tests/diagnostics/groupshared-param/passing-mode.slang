//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-experimental-feature

// A `groupshared` parameter is a reference to thread-group-shared memory (shader-slang/slang#10641).
// With no passing-mode keyword, `groupshared` implies `__ref`, or `__ref_readonly` with `const`,
// and an explicit `__ref`, `__ref_readonly`, or `__ref_writeonly` picks the access of the
// reference. A written `in`, `out`, `inout`, or `__borrow` (or its alias `__constref`) gives the
// parameter a mode that is not a reference, which is error 30707. The test is exhaustive, so the
// accepted spellings must compile with no diagnostic.

static const uint N = 8;

// --- Rejected: `in` / `out` / `inout` on a `groupshared` parameter ---

void badIn(uint tid, in groupshared uint s[N])
//diag:              ^^ cannot use the 'in' passing mode
//diag:              ^^ remove this, or use 'const groupshared' for a read-only reference
{
    s[tid] = tid;
}

void badOut(uint tid, out groupshared uint s[N])
//diag:               ^^^ cannot use the 'out' passing mode
//diag:               ^^^ remove this, or use 'const groupshared' for a read-only reference
{
    s[tid] = tid;
}

void badInOut(uint tid, inout groupshared uint s[N])
//diag:                 ^^^^^ cannot use the 'inout' passing mode
//diag:                 ^^^^^ remove this, or use 'const groupshared' for a read-only reference
{
    s[tid] = tid;
}

void badInOutSpaced(uint tid, in out groupshared uint s[N])
//diag:                       ^^ cannot use the 'inout' passing mode
//diag:                       ^^ remove this, or use 'const groupshared' for a read-only reference
{
    s[tid] = tid;
}

// --- Rejected: `__borrow` / `__constref` on a `groupshared` parameter ---

uint badBorrow(uint tid, __borrow groupshared uint s[N])
//diag:                  ^^^^^^^^ cannot use the 'borrow' passing mode
//diag:                  ^^^^^^^^ remove this, or use 'const groupshared' for a read-only reference
{
    return s[tid];
}

uint badConstRef(uint tid, __constref groupshared uint s[N])
//diag:                    ^^^^^^^^^^ cannot use the 'borrow' passing mode
//diag:                    ^^^^^^^^^^ remove this, or use 'const groupshared' for a read-only reference
{
    return s[tid];
}

// A doubled `__constref` reports the duplicate once and E30707 once.
uint badDoubleConstRef(uint tid, __constref __constref groupshared uint s[N])
//diag:                                     ^^^^^^^^^^ duplicate modifier
//diag:                                     ^^^^^^^^^^ modifier '__constref' is redundant or conflicting with existing modifier '__constref'
//diag:                          ^^^^^^^^^^ cannot use the 'borrow' passing mode
//diag:                          ^^^^^^^^^^ remove this, or use 'const groupshared' for a read-only reference
{
    return s[tid];
}

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
