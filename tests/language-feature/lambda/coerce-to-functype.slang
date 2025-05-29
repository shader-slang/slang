//TEST:INTERPRET(filecheck=CHECK):

func test(f: functype(int, int)->float) -> float
{
    return f(2,3) + 10.0f;
}

func main()
{
    let result = test((int x, int y)=>x+y);
    // CHECK: 15.0
    printf("%f\n", result);
}