// A borrow parameter (`__constref`) is an immutable borrow of its argument: the body may not
// write it, because on targets that lower a borrow to a pointer the write would reach the
// caller's storage. Every way of writing it errors: a direct assignment, a compound update, a
// whole-struct assignment, an assignment to a resource-typed parameter, an address taken
// through `&` or `__getAddress` and written through, and a forwarding call that binds it to
// writable or reference storage. Reads still work, and a legacy (C-style) `in` parameter stays
// a mutable local copy. The one escape is an explicit cast that widens a pointer's access, the
// language's general escape hatch as in C.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-experimental-feature

struct S { int a[4]; }

Texture2D<float4> g_input;

// Direct assignments.
int bump(__constref S s) { s.a[0] = 7; return s.a[0]; }
//diag:                           ^ left of '=' is not an l-value
//diag:                           ^ left of '=' is not an l-value.
int bumpInt(__constref int x) { x = 9; return x; }
//diag:                           ^ left of '=' is not an l-value
//diag:                           ^ left of '=' is not an l-value.
int bumpCompound(__constref int x) { x += 1; return x; }
//diag:                              ^ argument must be l-value
//diag:                              ^ argument passed to parameter '0' must be l-value.
int wholeStruct(__constref S s) { s = S(); return s.a[0]; }
//diag:                             ^ left of '=' is not an l-value
//diag:                             ^ left of '=' is not an l-value.
void replaceResource(__constref Texture2D<float4> value) { value = g_input; }
//diag:                                                          ^ left of '=' is not an l-value
//diag:                                                          ^ left of '=' is not an l-value.

// Taking a borrow's address, through either spelling, is an error.
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

// A borrow is a read-only value rather than a memory location, so it cannot be forwarded to
// reference or writable-storage parameters.
void takeInout(inout int x) { x = x + 1; }
int forwardToInout(__constref S s) { takeInout(s.a[0]); return s.a[0]; }
//diag:                                           ^ argument must be l-value
//diag:                                           ^ argument passed to parameter '0' must be l-value.
int leaf(__ref_readonly int x) { return x; }
int forwardBorrow(__constref S s) { return leaf(s.a[0]); }
//diag:                                            ^ argument must be l-value
//diag:                                            ^ argument passed to parameter '0' must be l-value.
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
