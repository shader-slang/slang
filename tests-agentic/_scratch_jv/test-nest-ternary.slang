//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int x = 2;
    int r = (x == 0) ? 100 : (x == 1) ? 200 : (x == 2) ? 300 : 400;
    // CHECK: r=300
    printf("r=%d\n", r);
}
