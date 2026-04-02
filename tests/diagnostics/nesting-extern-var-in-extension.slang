//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// `extern` vars in extensions reference existing fields and should NOT
// trigger the nesting validation. This tests that the Unknown bypass
// for extern vars works correctly.

struct S { int x; }

extension S
{
    extern int x;
}
