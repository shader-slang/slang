//TEST:INTERPRET(filecheck=CHECK):

struct Result
{
    float3 value;
    bool valid;
    int a;
    int b;
    int c;
}

[noinline]
Result makeResult(int seed)
{
    Result result;
    result.value = float3(1.0, 2.0, 3.0);
    result.valid = true;
    result.a = seed + 3;
    result.b = seed + 4;
    result.c = seed + 5;
    return result;
}

[noinline]
void printLastElement(float4 value)
{
    printf("%.1f\n", value[3]);
}

export __extern_cpp int main(int argc, Ptr<NativeString> argv)
{
    Result result = makeResult(argc);
    //CHECK: 6
    printf("%d\n", result.c);
    //CHECK: 4.0
    printLastElement(float4(1.0, 2.0, 3.0, 4.0));
    return 0;
}
