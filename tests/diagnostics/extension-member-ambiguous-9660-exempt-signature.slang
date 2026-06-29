//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660, exemption (iii): different signatures.
//
// A same-named member function whose parameter signature differs is a legitimate overload, not a
// conflict, so it must NOT warn (41037). 41037 is proven to fire by the positive companion test
// extension-member-ambiguous-9660.slang, so a clean run here is meaningful.

// `f(int)` on the struct and `f(float)` on the extension have different parameter types, so they
// form an overload set rather than a conflict.
struct OverloadOk
{
    void f(int x) {}
}

extension OverloadOk
{
    void f(float x) {}
}

// Exhaustive diag=CHECK mode fails on any unannotated diagnostic, so a clean run here proves no
// 41037 was emitted; the CHECK-NOT documents that intent explicitly against the message text.
//CHECK-NOT: declared with the same signature in more than one place
