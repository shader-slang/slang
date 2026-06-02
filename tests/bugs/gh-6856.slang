//TEST:INTERPRET(filecheck=CHECK):

public uint f<each T: IInteger>(expand each T t)
{
    return (expand (each t).toUInt());
}

void main()
{
    uint result = f(42);
    //CHECK: 42
    printf("%u\n", result);
}
