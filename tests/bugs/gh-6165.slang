// Test for issue #6165: Error reporting when a parse error comes from a macro expansion
// should point to the macro use site, not the macro definition site.
//
// When `Params::TEST` is parsed and TEST expands to `2`, the parser error
// "unexpected integer literal, expected identifier" should point to where TEST
// is used (line with the if statement), not to where it is defined (#define line).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

#define TEST 2

struct Params
{
    static const int kTEST = TEST;
};

int test()
{
    if (Params::TEST == 1)
//CHECK:        ^^^^ unexpected integer literal, expected identifier
        return 1;
    return 2;
}

#define INNER 2
#define OUTER Params::INNER
int test2()
{
    if (OUTER == 1)
//CHECK:^^^^^ unexpected integer literal
        return 1;
    return 2;
}
