//TEST(compute):SIMPLE:

// Test for correct recursive macro behavior. In particular: 

// The letter of the spec is that we should macro expand
// each argument *before* substitution, and then go and
// macro-expand the substituted body. This means that we
// can invoke a macro as part of an argument to an
// invocation of the same macro:
//
//     FOO( 1, FOO(22, 2, 2), 333 );

// Also in the case on NO_EXPAND, it will not be expanded in the substitution. 

#define ARG_EXPAND(x) ( x )

// This macro should expand to NO_EXPAND(int 'a'), and not multiply invoke NO_EXPAND, because
// one the args have been expanded.
#define NO_EXPAND(a) NO_EXPAND(int a)

// Should expand to int NO_EXPAND(int b)
int NO_EXPAND(b)
{
    return b + ARG_EXPAND(ARG_EXPAND(1));
}