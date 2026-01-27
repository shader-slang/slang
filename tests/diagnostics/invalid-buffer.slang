//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: ([[# @LINE+1]]): error 30400: generic type 'Buffer' used without argument
Buffer b0;

// CHECK: ([[# @LINE+1]]): error 30075: cannot specialize generic 'Buffer' with the provided arguments.
Buffer<> b1;

[numthreads(1,1,1)]
void computeMain()
{
}
