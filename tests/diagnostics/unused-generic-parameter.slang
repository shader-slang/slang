//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// CHECK: ([[# @LINE+1]]): error 30403: generic parameter 'T' is not used
extension<T> int
{
    void foo() {}
}

void bar()
{
    (5).foo();
}

[shader("compute")]
[numthreads(1,1,1)]
float4 computeMain()
{
    bar();
    return float4(1.0);
} 