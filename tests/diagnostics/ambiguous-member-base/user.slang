//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
import lib;

static X x;

void test()
{
    // CHECK: ([[# @LINE+1]]):{{.*}}ambiguous reference to 'x'
    x.m = 2;
    // CHECK: ([[# @LINE+1]]):{{.*}}ambiguous reference to 'x'
    x[3] = 2;
    // CHECK: ([[# @LINE+1]]):{{.*}}ambiguous reference to 'x'
    x();
}