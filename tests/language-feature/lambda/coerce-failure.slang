//TEST:SIMPLE(filecheck=CHECK): -target spirv


func test(f: functype(int, int)->float) -> float
{
    return f(2,3) + 10.0f;
}


[numthreads(1,1,1)]
void computeMain()
{
    int c = 2;
    // CHECK: error[E30019]:
    // CHECK: coerce-failure.slang:[[# @LINE+1]]:
    let result = test((int x, int y)=> x + y + c);

    // CHECK: error[E30019]:
    // CHECK: coerce-failure.slang:[[# @LINE+1]]:
    let result1 = test((int x, float y) => x + y);
}