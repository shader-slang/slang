//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660, exemption (i) under reverse source order.
//
// The interface-satisfaction exemption reads the `IsOverridingModifier` that conformance checking
// attaches to a member. Module checking advances in source order, so when an `extension` is
// declared textually BEFORE the interface-conforming `struct`, the struct (and therefore the
// witness marking) may not have been conformance-checked yet at the point this diagnostic runs.
// `checkExtensionMemberConflicts` forces the comparison sources to conformance-checked state first,
// so neither member must warn (41037) here.

interface IReq
{
    int g();
}

// CASE A: the extension declares `g` before the struct that conforms to `IReq`. The struct member
// satisfies the requirement, so the pair is an intended interface implementation, not a conflict.
extension SA
{
    int g() { return 1; }
}

struct SA : IReq
{
    int g() { return 2; }
}

// CASE B (symmetric): the extension is the side that satisfies the interface requirement, and it is
// declared before the struct. This must likewise NOT warn.
extension SB : IReq
{
    int g() { return 1; }
}

struct SB
{
    int g() { return 2; }
}

// Exhaustive diag=CHECK mode fails on any unannotated diagnostic, so a clean run here proves no
// 41037 was emitted; the CHECK-NOT documents that intent explicitly against the message text.
//CHECK-NOT: declared in more than one place
