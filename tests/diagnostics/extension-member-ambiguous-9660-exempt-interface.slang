//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660, exemption (i): interface satisfaction.
//
// When an extension member satisfies an interface requirement (conformance checking marks it as
// fulfilling that requirement), it is an intended interface implementation, not a conflict, so it
// must NOT warn (41037) even though the struct also declares a `g`.

interface IReq
{
    int g();
}

struct S
{
    int g() { return 1; }
}

extension S : IReq
{
    int g() { return 2; }
}

// Exhaustive diag=CHECK mode fails the test on any unannotated diagnostic, so a clean run here
// proves no 41037 ("...declared in more than one place...") was emitted for this exempt case.
