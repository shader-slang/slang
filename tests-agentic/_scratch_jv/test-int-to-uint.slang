//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int neg = -1;
    uint u = uint(neg);
    // CHECK: u=4294967295
    printf("u=%u\n", u);
}
