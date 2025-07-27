//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: ([[# @LINE+1]]): error 30400: generic type 'Buffer' used without argument
Buffer b;

[numthreads(1,1,1)]
void computeMain()
{
}