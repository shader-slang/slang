//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
// Using non-exhaustive because there are many "candidate: static X x" notes
import lib;

static X x;

void test()
{
    x.m = 2;
//CHECK: ambiguous reference to 'x'
    x[3] = 2;
//CHECK: ambiguous reference to 'x'
    x();
//CHECK: ambiguous reference to 'x'
}