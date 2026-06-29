//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660, exemption (ii): free-form generic extension.
//
// An `extension<T : IFoo> T { ... }` extends a bare type parameter and legitimately supplies
// default members to every conforming type, so its members must NOT warn (41037) against a member
// of the same name on a concrete conforming type.

interface IFoo {}

struct S2 : IFoo
{
    int h() { return 1; }
}

extension<T : IFoo> T
{
    int h() { return 2; }
}

// Exhaustive diag=CHECK mode fails on any unannotated diagnostic, so a clean run here proves no
// 41037 was emitted; the CHECK-NOT documents that intent explicitly against the message text.
//CHECK-NOT: declared in more than one place
