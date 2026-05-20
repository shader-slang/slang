//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int x = 11;
    int* p = &x;
    int* q = p;
    *q = 22;
    // CHECK: x=22
    printf("x=%d\n", x);
    // CHECK: peq=1
    printf("peq=%d\n", (p == q) ? 1 : 0);
}
