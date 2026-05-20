//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    half a = half(1.5);
    // CHECK: a=1
    printf("a=%d\n", int(a));
}
