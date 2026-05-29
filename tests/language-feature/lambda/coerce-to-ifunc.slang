// Used as example in docs/user-guide/03-convenience-features.md ("Lambda Expressions - Coercion to IFunc")

//TEST:INTERPRET(filecheck=CHECK):

func apply(f: IFunc<float, float>) -> float
{
    return f(2.0);
}

func main()
{
    float scale = 3;
    let result = apply((float x) => x * scale);
    // CHECK: 6.0
    printf("%f\n", result);
}
