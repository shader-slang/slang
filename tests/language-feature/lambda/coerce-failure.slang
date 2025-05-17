//TEST:SIMPLE(filecheck=CHECK): -target spirv


func test(f: functype(int, int)->float) -> float
{
    return f(2,3) + 10.0f;
}


[numthreads(1,1,1)]
void computeMain()
{
    int c = 2;
    // CHECK: ([[# @LINE+1]]): error 30019
    let result = test((int x, int y)=> x + y + c);

    // CHECK: ([[# @LINE+1]]): error 30019
    let result1 = test((int x, float y) => x + y);
}