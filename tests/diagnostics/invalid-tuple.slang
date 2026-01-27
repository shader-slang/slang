//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// CHECK: ([[# @LINE+1]]): error 30400: generic type 'Tuple' used without argument
Tuple t0;

// This should be OK, compiling down to a zero-sized type
Tuple<> t1;

[numthreads(1,1,1)]
void computeMain()
{
}
