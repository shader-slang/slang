//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    var get42 = () => 42;
    // CHECK: r=42
    printf("r=%d\n", get42());
}
