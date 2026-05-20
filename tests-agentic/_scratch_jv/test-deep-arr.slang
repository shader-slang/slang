//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int a[16];
    for (int i = 0; i < 16; ++i) a[i] = i * i;
    // CHECK: hi=225
    printf("hi=%d\n", a[15]);
    // CHECK: lo=0
    printf("lo=%d\n", a[0]);
}
