// A borrow parameter (`__constref`, `__borrow`) is an immutable borrow of its argument: the
// body may not write it, because on targets that lower a borrow to a pointer the write would
// reach the caller's storage. Every way of writing it errors: a direct assignment, a compound
// update, an increment, a swizzle write, a whole-struct assignment, an assignment to a
// resource-typed parameter, an address taken through `&` or `__getAddress` and written through
// (including member access through the pointer), and a forwarding call that binds it to
// writable or reference storage — in both the legacy and the modern parameter style, and
// through every forwarding shape. Reads still work, and a legacy (C-style) `in` parameter
// stays a mutable local copy. The one escape is an explicit cast that widens a pointer's
// access, the language's general escape hatch as in C.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-experimental-feature

struct S { int a[4]; int4 v; }

Texture2D<float4> g_input;

// Direct assignments, in both parameter styles and both borrow spellings.
int bump(__constref S s) { s.a[0] = 7; return s.a[0]; }
//diag:                           ^ left of '=' is not an l-value
//diag:                           ^ left of '=' is not an l-value.
int bumpInt(__constref int x) { x = 9; return x; }
//diag:                           ^ left of '=' is not an l-value
//diag:                           ^ left of '=' is not an l-value.
int bumpBorrowSpelling(__borrow int x) { x = 9; return x; }
//diag:                                    ^ left of '=' is not an l-value
//diag:                                    ^ left of '=' is not an l-value.
func bumpModernA(__borrow x: int) { x = 9; }
//diag:                               ^ left of '=' is not an l-value
//diag:                               ^ left of '=' is not an l-value.
int bumpCompound(__constref int x) { x += 1; return x; }
//diag:                              ^ argument must be l-value
//diag:                              ^ argument passed to parameter '0' must be l-value.
int bumpIncr(__constref int x) { x++; return x; }
//diag:                          ^ argument must be l-value
//diag:                          ^ argument passed to parameter '0' must be l-value.
int writeSwizzle(__constref S s) { s.v.xy = int2(1, 2); return s.v.x; }
//diag:                                   ^ left of '=' is not an l-value
//diag:                                   ^ left of '=' is not an l-value.
int wholeStruct(__constref S s) { s = S(); return s.a[0]; }
//diag:                             ^ left of '=' is not an l-value
//diag:                             ^ left of '=' is not an l-value.
void replaceResource(__constref Texture2D<float4> value) { value = g_input; }
//diag:                                                          ^ left of '=' is not an l-value
//diag:                                                          ^ left of '=' is not an l-value.

// Taking a borrow's address with `&` is an error; `__getAddress` yields a read-only pointer,
// and writing through it — a plain dereference, a member access, or the `->` spelling — is an
// error. The `&` spelling resolves through `operator&`, whose argument check fires alongside
// the immutable-object check, which is why one expression pins two diagnostic families.
int takeAddress(__constref S s) { return (&s).a[0]; }
//diag:                                    ^ cannot take address of immutable object
//diag:                                    ^ Not allowed to take the address of an immutable object
//diag:                                    ^ argument must be l-value
//diag:                                    ^ argument passed to parameter '0' must be l-value.
int writeThroughAddress(__constref int x)
{
    let address = __getAddress(x);
    *address = 11;
//diag:      ^ left of '=' is not an l-value
//diag:      ^ left of '=' is not an l-value.
    return x;
}
int writeMemberThroughAddress(__constref S s)
{
    __getAddress(s).a[0] = 7;
//diag:                  ^ left of '=' is not an l-value
//diag:                  ^ left of '=' is not an l-value.
    return s.a[0];
}
int writeArrowThroughAddress(__constref S s)
{
    __getAddress(s)->a[0] = 7;
//diag:                   ^ left of '=' is not an l-value
//diag:                   ^ left of '=' is not an l-value.
    return s.a[0];
}

// The address of any parameter the body cannot write is a read-only pointer — a modern `in`
// parameter and a `__ref_readonly` parameter included — so writing through it errors.
func modernIn(x: int)
{
    let p = __getAddress(x);
    *p = 5;
//diag:^ left of '=' is not an l-value
//diag:^ left of '=' is not an l-value.
}
int refReadonlyAddress(__ref_readonly int x)
{
    let p = __getAddress(x);
    *p = 5;
//diag:^ left of '=' is not an l-value
//diag:^ left of '=' is not an l-value.
    return x;
}

// A subscript on a pointer whose generic `Access` argument is not yet resolved is read-only by
// default, so writing through it errors at the definition.
void writeGeneric<let acc:Access>(Ptr<int, acc> p)
{
    p[0] = 7;
//diag:  ^ left of '=' is not an l-value
//diag:  ^ left of '=' is not an l-value.
}

// A borrow is a read-only value rather than a memory location, so it cannot be forwarded to
// reference or writable-storage parameters, through any of its parts.
void takeInout(inout int x) { x = x + 1; }
int forwardToInout(__constref S s) { takeInout(s.a[0]); return s.a[0]; }
//diag:                                           ^ argument must be l-value
//diag:                                           ^ argument passed to parameter '0' must be l-value.
void takeOut(out int x) { x = 1; }
int forwardToOut(__constref S s)
{
    takeOut(s.a[0]);
//diag:        ^ argument must be l-value
//diag:        ^ argument passed to parameter '0' must be l-value.
    return s.a[0];
}
int leaf(__ref_readonly int x) { return x; }
int forwardBorrow(__constref S s) { return leaf(s.a[0]); }
//diag:                                            ^ argument must be l-value
//diag:                                            ^ argument passed to parameter '0' must be l-value.
int forwardVectorElement(__constref S s) { return leaf(s.v[2]); }
//diag:                                                   ^ argument must be l-value
//diag:                                                   ^ argument passed to parameter '0' must be l-value.
int forwardWholeStruct(__constref S s) { return leafS(s); }
//diag:                                               ^ argument must be l-value
//diag:                                               ^ argument passed to parameter '0' must be l-value.
int leafS(__ref_readonly S s) { return s.a[0]; }
// The '0' in the messages above is the argument index, not a parameter name: the emitter fills
// the message's parameter slot with the argument index, so a reader should not take '0' as the
// callee's parameter being named "0".

// The remaining escape hatch: an explicit cast widens the read-only pointer's access, as in C,
// so these are expected to compile cleanly. Writing a borrow this way is the programmer
// throwing the rule away on purpose.
int writeThroughCast(__constref int x)
{
    int* address = (int*)__getAddress(x);
    *address = 12;
    return x;
}
void replaceThroughCast(__constref Texture2D<float4> value)
{
    Texture2D<float4>* address = (Texture2D<float4>*)(__getAddress(value));
    *address = g_input;
}

// Negative controls, expected to compile cleanly: reading a borrow, and writing a legacy
// (C-style) `in` parameter.
int readBorrow(__constref S s) { return s.a[0]; }
int bumpLegacyIn(in int x) { x = x + 10; return x; }
